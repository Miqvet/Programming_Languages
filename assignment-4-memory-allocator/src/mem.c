#define _DEFAULT_SOURCE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mem_internals.h"
#include "mem.h"
#include "util.h"

void debug_block(struct block_header* b, const char* fmt, ... );
void debug(const char* fmt, ... );

extern inline block_size size_from_capacity( block_capacity cap );
extern inline block_capacity capacity_from_size( block_size sz );

static bool            block_is_big_enough( size_t query, struct block_header* block ) { return block->capacity.bytes >= query; }
static size_t          pages_count   ( size_t mem )                      { return mem / getpagesize() + ((mem % getpagesize()) > 0); }
static size_t          round_pages   ( size_t mem )                      { return getpagesize() * pages_count( mem ) ; }

static void block_init( void* restrict addr, block_size block_sz, void* restrict next ) {
  *((struct block_header*)addr) = (struct block_header) {
    .next = next,
    .capacity = capacity_from_size(block_sz),
    .is_free = true
  };
}

static size_t region_actual_size( size_t query ) { return size_max( round_pages( query ), REGION_MIN_SIZE ); }

extern inline bool region_is_invalid( const struct region* r );



static void* map_pages(void const* addr, size_t length, int additional_flags) {
  return mmap( (void*) addr, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | additional_flags , -1, 0 );
}

/*  аллоцировать регион памяти и инициализировать его блоком */
static struct region alloc_region  ( void const * addr, size_t query ) {
    size_t current_size = region_actual_size(size_from_capacity((block_capacity) {query}).bytes);
    void *addr_page = map_pages(addr, current_size, MAP_FIXED_NOREPLACE);
    if (addr_page == MAP_FAILED) {
        addr_page = map_pages(addr, current_size, 0);
        if (addr_page == MAP_FAILED)return REGION_INVALID;
    }
    block_init(addr_page, (block_size) {current_size}, NULL);
    return (struct region) { .addr = addr_page,
            .size = current_size,
            .extends = addr_page == addr
    };
}

static void* block_after( struct block_header const* block )         ;

static bool blocks_continuous (
        struct block_header const* fst,
        struct block_header const* snd );

void* heap_init( size_t initial ) {
  const struct region region = alloc_region( HEAP_START, initial );
  if ( region_is_invalid(&region) ) return NULL;
  return region.addr;
}

/*  освободить всю память, выделенную под кучу */
void heap_term( ) {
    struct block_header *current_block = (struct block_header*) HEAP_START;
    size_t region_current_size = 0;
    while (current_block) {
        region_current_size = size_from_capacity(current_block ->capacity).bytes;
        struct block_header *region_start = current_block;
        while(blocks_continuous(current_block, current_block->next)){
            current_block = current_block ->next;
            region_current_size += size_from_capacity(current_block ->capacity).bytes;
        }
        current_block = current_block -> next;
        munmap(region_start, region_current_size);
    }

}

#define BLOCK_MIN_CAPACITY 24

/*  --- Разделение блоков (если найденный свободный блок слишком большой )--- */

static bool block_splittable( struct block_header* restrict block, size_t query) {
  return block-> is_free && query + offsetof( struct block_header, contents ) + BLOCK_MIN_CAPACITY <= block->capacity.bytes;
}

static bool split_if_too_big( struct block_header* block, size_t query ) {
    if (block_splittable(block, query)){
        void *split = query + block->contents;
        block_init(split, (block_size) {block->capacity.bytes - query}, block->next);
        block->capacity.bytes = query;
        block->next = split;
        return true;
    }
    return false;
}


/*  --- Слияние соседних свободных блоков --- */

static void* block_after( struct block_header const* block )              {
  return  (void*) (block->contents + block->capacity.bytes);
}
static bool blocks_continuous (
                               struct block_header const* fst,
                               struct block_header const* snd ) {
  return (void*)snd == block_after(fst);
}

static bool mergeable(struct block_header const* restrict fst, struct block_header const* restrict snd) {
  return fst->is_free && snd->is_free && blocks_continuous( fst, snd ) ;
}

static bool try_merge_with_next( struct block_header* block ) {
    struct block_header *next = block->next;
    if (next && mergeable(block, next)){
        block->capacity.bytes += size_from_capacity(next->capacity).bytes;
        block->next = next->next;
        return true;
    }
    return false;

}


/*  --- ... ecли размера кучи хватает --- */
enum BSR_ERR{
  BSR_FOUND_GOOD_BLOCK = 0,
  BSR_REACHED_END_NOT_FOUND=1,
  BSR_CORRUPTED=2
};
struct block_search_result {
  enum BSR_ERR type;
  struct block_header* block;
};

struct block_search_result create_block_search_result(enum BSR_ERR type, struct block_header* block){
  return (struct block_search_result){type, block};
}

static struct block_search_result find_good_or_last  ( struct block_header* restrict block, size_t sz )    {
    struct block_header* last = NULL;
    if(!block){
      return create_block_search_result(BSR_CORRUPTED, NULL);
    }
    while (block){
        if (block->is_free){
            while (true) {if (!block->next || !try_merge_with_next(block)) {break;}}
            if (block_is_big_enough(sz, block)) return create_block_search_result(BSR_FOUND_GOOD_BLOCK, block);
        }
        last = block;
        block = block->next;
    }
    return create_block_search_result( BSR_REACHED_END_NOT_FOUND, last);
}

/*  Попробовать выделить память в куче начиная с блока `block` не пытаясь расширить кучу
 Можно переиспользовать как только кучу расширили. */
static struct block_search_result try_memalloc_existing ( size_t query, struct block_header* block ) {
    struct block_search_result result_block = find_good_or_last(block, size_max(query, BLOCK_MIN_CAPACITY));
    if (result_block.type == BSR_FOUND_GOOD_BLOCK){
        split_if_too_big(result_block.block, size_max(query, BLOCK_MIN_CAPACITY));
        result_block.block->is_free = false;
    }
    return result_block;
}



static struct block_header* grow_heap( struct block_header* restrict last, size_t query ) {
    if (!last) return NULL;
    struct region buffer_region = alloc_region(block_after(last), query);
    if (region_is_invalid(&buffer_region)) return NULL;
    block_init(buffer_region.addr, (block_size) {.bytes = buffer_region.size}, NULL);
    last->next = buffer_region.addr;
    if (!try_merge_with_next(last))return last->next;
    return last;
}

/*  Реализует основную логику malloc и возвращает заголовок выделенного блока */
static struct block_header* memalloc( size_t query, struct block_header* heap_start) {
    query = size_max(query, BLOCK_MIN_CAPACITY);
    struct block_search_result maybe_good_block = try_memalloc_existing(query, heap_start);

    if (maybe_good_block.type == BSR_FOUND_GOOD_BLOCK)return maybe_good_block.block;
    if (maybe_good_block.type == BSR_CORRUPTED) return NULL;

    maybe_good_block.block = grow_heap(maybe_good_block.block, query);
    if(!maybe_good_block.block) return NULL;
    maybe_good_block = try_memalloc_existing(query, maybe_good_block.block);

    if (maybe_good_block.type != BSR_FOUND_GOOD_BLOCK) return NULL;
    return maybe_good_block.block;
}

void* _malloc( size_t query ) {
  struct block_header* const addr = memalloc( query, (struct block_header*) HEAP_START );
  if (addr) return addr->contents;
  else return NULL;
}

static struct block_header* block_get_header(void* contents) {
  return (struct block_header*) (((uint8_t*)contents)-offsetof(struct block_header, contents));
}

void _free( void* mem ) {
  if (!mem) return ;
  struct block_header* header = block_get_header( mem );
  header->is_free = true;
  try_merge_with_next(header);
}

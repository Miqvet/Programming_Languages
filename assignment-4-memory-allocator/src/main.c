#include "mem.h"
#include "mem_internals.h"
#include <assert.h>
#include <stdio.h>
#define HEAP_SIZE 10000

static void test_1() {
    printf("Test1: allocation\n");
    void* heap = heap_init(HEAP_SIZE);
    assert(heap && "test1 heapInit");
    void* f_memory = _malloc(100);
    assert(f_memory && "test1 malloc");
    heap_term();
    printf("test 1 pass all tasks\n\n\n");
}

static void test2() {
    printf("Test2: Single Block\n");
    void* heap = heap_init(HEAP_SIZE);
    assert(heap && "test2 heapInit");
    void* block1 = _malloc(100);
    void* block2 = _malloc(1000);
    assert(block2 != NULL && block1 != NULL && "Allocation failed for block 2");
    debug_heap(stdout, heap);
    _free(block1);
    debug_heap(stdout, heap);
    heap_term();
    printf("Test 2 pass all tasks\n\n\n");
}
static void test3() {
    printf("Test3: Multi free\n");
    void* heap = heap_init(HEAP_SIZE);
    assert(heap && "test3 heapInit");
    void* block1 = _malloc(100);
    void* block2 = _malloc(1000);
    void* block3 = _malloc(1000);
    assert(block1 != NULL && block2 != NULL && block3 != NULL && "fail allocation");
    debug_heap(stdout, heap);
    _free(block1);
    _free(block2);
    debug_heap(stdout, heap);
    heap_term();
    printf("Test 3 pass all tasks\n\n\n");
}
static void test4() {
    printf("Test4: Update old Region\n");
    void* heap = heap_init(100);
    assert(heap && "test4 heapInit");
    void* block1 = _malloc(1000);
    assert(block1 != NULL && "fail allocation");
    debug_heap(stdout, heap);
    heap_term();
    printf("Test 4 pass all tasks\n\n\n");
}
void start_test(){
    test_1();
    test2();
    test3();
    test4();
}
int main() {
    start_test();
    return 0;
}

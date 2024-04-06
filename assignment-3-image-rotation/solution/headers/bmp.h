
#ifndef BMP_H
#define BMP_H

#include "image.h"
#include <stdio.h>

#pragma pack(push, 1)
struct bmp_header
{
    uint16_t bfType;
    uint32_t  bfileSize;
    uint32_t bfReserved;
    uint32_t bOffBits;
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t  biHeight;
    uint16_t  biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t  biClrImportant;
};
#pragma pack(pop)

/*  deserializer   */
enum read_status  {
    READ_OK = 0,
    READ_INVALID_SIGNATURE=1,
    READ_INVALID_BITS=2,
    READ_INVALID_HEADER=3,
    READ_ERROR=4
};

enum read_status from_bmp( FILE* in, struct image* img );

enum  write_status  {
    WRITE_OK = 0,
    WRITE_HEADER_ERROR = 1,
    WRITE_FILE_ERROR= 2,
    WRITE_ERROR=4
};
enum read_status read_bmp_from_file(const char* filename, struct image* img);
enum write_status write_bmp_to_file(const char* filename, const struct image* img);
enum write_status to_bmp( FILE* out, struct image const* img );
struct bmp_header create_bmp_header(struct image const* img);
#endif //BMP_H

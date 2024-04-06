#include "../headers//bmp.h"
#include "../headers/file_control.h"
#include <stdio.h>
#include <stdlib.h>
#define BMP_SIGNATURE 0x4D42
#define BMP_SIZE 40
#define BMP_BIT_COUNT 24
enum read_status from_bmp(FILE* in, struct image* img) {
    struct bmp_header header;
    if (fread(&header, sizeof(header), 1, in) != 1) {
        return READ_INVALID_HEADER;
    }
    if (header.bfType != BMP_SIGNATURE || header.biBitCount != BMP_BIT_COUNT || header.biSize != BMP_SIZE) {
        return READ_INVALID_SIGNATURE;
    }
    img->width = header.biWidth;
    img->height = header.biHeight;
    img->data = malloc(header.biWidth * header.biHeight * sizeof(struct pixel));
    uint8_t padding = (4 - (sizeof(struct pixel) * header.biWidth) % 4) % 4;
    for (uint32_t i = 0; i < header.biHeight; i++) {
        size_t row = header.biHeight - i - 1;
        if (fread(&(img->data[row * header.biWidth]),
                  sizeof(struct pixel),
                  header.biWidth, in) != header.biWidth ||
                fseek(in, padding, SEEK_CUR)) {
            free(img->data);
            return READ_INVALID_BITS;
        }
    }
    return READ_OK;
}

enum write_status to_bmp(FILE* out, struct image const* img ){
    struct bmp_header header = create_bmp_header(img);
    long padding = (long) (4 - (sizeof(struct pixel) * header.biWidth) % 4) % 4;
    if (fwrite(&header, sizeof(header), 1, out) != 1) {
        return WRITE_ERROR;
    }
    for (uint32_t i = 0; i < img->height; i++) {
        size_t rows = img->height - 1 - i;
        if (fwrite(&img->data[rows *header.biWidth],
                   sizeof(struct pixel),
                header.biWidth,
                out) != header.biWidth) {
            return WRITE_ERROR;
        }
        fseek(out, padding, SEEK_CUR);
    }
    return WRITE_OK;
}

struct bmp_header create_bmp_header(struct image const* img){
    uint8_t padding = (4 - (img->width * sizeof(struct pixel)) % 4) % 4;
    struct bmp_header header;

    header.bfType = BMP_SIGNATURE;
    header.bfileSize = sizeof(struct bmp_header) + (sizeof(struct pixel) * img -> width + padding) * img -> height;
    header.bfReserved = 0;
    header.bOffBits = sizeof(struct bmp_header);
    header.biSize = BMP_SIZE;
    header.biHeight = img -> height;
    header.biWidth = img -> width;
    header.biPlanes = 1;
    header.biBitCount = BMP_BIT_COUNT;
    header.biCompression = 0;
    header.biSizeImage = (img->width * sizeof(struct pixel) + padding) * img->height;
    header.biXPelsPerMeter = 1;
    header.biYPelsPerMeter = 1;
    header.biClrUsed = 0;
    header.biClrImportant = 0;

    return header;
}
enum read_status read_bmp_from_file(const char* filename, struct image* img) {
    FILE* sourse;
    if(file_open(filename, &sourse, FILE_RB) != FILE_OK){
        return READ_ERROR;
    }
    enum read_status status = from_bmp(sourse, img);
    fclose(sourse);
    return status;
}

enum write_status write_bmp_to_file(const char* filename, const struct image* img) {
    FILE* final_file;
    if(file_open(filename, &final_file, FILE_WB) != FILE_OK){
        return WRITE_ERROR;
    }
    enum write_status status = to_bmp(final_file, img);
    fclose(final_file);
    return status;
}

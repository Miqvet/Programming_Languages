
#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>


struct pixel {
    uint8_t b, g, r;
};

struct image {
    uint32_t width, height;
    struct pixel* data;
};
void image_free(struct image* img);
#endif

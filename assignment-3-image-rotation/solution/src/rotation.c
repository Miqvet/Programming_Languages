#include "../headers/rotation.h"
#include <stdlib.h>

struct image rotation_by_angle(struct image source, int angle) {
    struct image rotated;

    angle = (angle + 360) % 360;

    switch (angle) {
        case 0:
            return source;
        case 90:
            rotated = rotation_90deg(source,1);
            break;
        case 180:
            rotated = rotation_90deg(rotation_90deg(source, 1), 1);
            break;
        default:
            rotated = rotation_90deg(source, 0);
            break;

    }
    return rotated;
}


struct image rotation_90deg(struct image source, int side ) {
    struct image rotated;
    rotated.width = source.height;
    rotated.height = source.width;
    rotated.data = malloc(source.height * source.width * sizeof(struct pixel));
    for (int64_t y = 0; y < source.height; y++) {
        for (int64_t x = 0; x < source.width; x++) {
            if(side){
                rotated.data[(source.height - y - 1) + x * source.height] = source.data[y * source.width + x];
            }
            else{rotated.data[y + (source.width - x - 1) * source.height]  = source.data[y * source.width + x];}
        }
    }
    image_free(&source);
    return rotated;
}

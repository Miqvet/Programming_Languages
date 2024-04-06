

#ifndef ROTATION_H
#define ROTATION_H
#include "image.h"
struct image rotation_by_angle(struct image source, int angle) ;
struct image rotation_90deg(struct image source, int side);
#endif //ROTATION_H

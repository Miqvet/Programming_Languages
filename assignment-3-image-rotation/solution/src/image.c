#include "../headers//image.h"
#include <stdlib.h>


void image_free(struct image* img) {
    free(img->data);
    img->data = NULL;
}

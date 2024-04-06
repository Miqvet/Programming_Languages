#include "../headers//bmp.h"
#include "../headers//rotation.h"
#include "../headers//util.h"
#include <stdio.h>
#include <stdlib.h>
static const char* validate_msg[] = {"Usage: ./image-transformer <source> <out-image> <angle>",
                                     "The angle must be integer divisible by 90"
};
static const char* read_msg[] = {"reading ok",
                                     "Error when reading the header part of the file",
                                     "Error in header file, headers have incorrect meaning",
                                     "Error reading the file itself"
};
static const char* write_msg[] = {"writing ok",
                                     "Error writing file header",
                                     "Error while writing the file itself"
};
int main(int argc, char* argv[]) {
    enum status_validate status_of_validate = parse(argc, argv);
    if (status_of_validate){
        fprintf(stderr,  "%s\n" , validate_msg[status_of_validate - 1]);
        return 1;
    }
    const char* input_filename = argv[1];
    const char* output_filename = argv[2];
    int angle = atoi(argv[3]);
    struct image img;
    enum read_status reading_status = read_bmp_from_file(input_filename, &img);
    if (reading_status != READ_OK) {
        fprintf(stderr,  "%s\n" , read_msg[reading_status]);
        return 1;
    }

    struct image rotated = rotation_by_angle(img, angle);
    enum write_status writing_status = write_bmp_to_file(output_filename, &rotated);
    if (writing_status != WRITE_OK) {
        image_free(&rotated);
        fprintf(stderr,  "%s\n" , write_msg[writing_status]);
        return 1;
    }
    image_free(&rotated);
    return 0;
}

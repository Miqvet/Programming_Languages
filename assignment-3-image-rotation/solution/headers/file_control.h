
#ifndef INC_3LAB_FILE_CONTROL_H
#define INC_3LAB_FILE_CONTROL_H

#include <stdio.h>

enum access_type {
    FILE_RB = 0,
    FILE_WB
};

enum file_status {
    FILE_OK = 0,
    FILE_NO_SUCH_TYPE = 1,
    FILE_ERROR
};

enum file_status file_open(const char* filename, FILE** file, enum access_type type);

#endif //INC_3LAB_FILE_CONTROL_H

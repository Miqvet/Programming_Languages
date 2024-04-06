#include "../headers/file_control.h"
enum file_status file_open(const char* filename, FILE** file, enum access_type type) {
    switch (type) {
        case FILE_RB:
            *file = fopen(filename, "rb");
            break;
        case FILE_WB:
            *file = fopen(filename, "wb");
            break;
        default:
            return FILE_NO_SUCH_TYPE;
    }
    return FILE_OK;
}

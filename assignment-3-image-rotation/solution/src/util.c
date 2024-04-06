#include "../headers/util.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
enum status_validate parse(int argc, char* argv[]) {
    if (argc != 4) {
        return INCORRECT_NUMBER_OF_ARGUMENTS;
    }
    if(isNumber(argv[3]) && atoi(argv[3]) % 90 == 0){
        return VALIDATE_OK;
    }
    return WRONG_ANGLE;
}
int isNumber(const char *str) {
    int i;
    unsigned long len;
    len = strlen(str);
    for (i = 0; i < len; i++) {
        if (!isdigit(str[i]) && strcmp( &str[i], "-") == 0) {
            return 0;
        }
    }
    return 1;
}

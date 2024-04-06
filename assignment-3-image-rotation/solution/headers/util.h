

#ifndef INC_3LAB_UTIL_H
#define INC_3LAB_UTIL_H


enum status_validate  {
    VALIDATE_OK = 0,
    INCORRECT_NUMBER_OF_ARGUMENTS=1,
    WRONG_ANGLE=2,
};
enum status_validate parse(int argc, char* argv[]);
int isNumber(const char *str);
#endif //INC_3LAB_UTIL_H

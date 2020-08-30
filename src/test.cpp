#include "test.h"

#include "Global.h"

void setupTest() {
    //пример выделения подстрок
    String buf = "Geeks-for-Geeks";
    int buf_len = buf.length() + 1;
    char char_array[buf_len];
    buf.toCharArray(char_array, buf_len);
    char* token = strtok(char_array, "-");
    while (token != NULL) {
        printf("%s\n", token);
        token = strtok(NULL, "-");
    }

    //char str[] = "Geeks for Geeks";
    //char* token;
    //char* rest = str;
    //while ((token = strtok_r(rest, " ", &rest))) {
    //    printf("%s\n", token);
    //}
}

void loopTest() {
}

//char stringToCharArray(int& i, String in) {
//    int in_len = in.length() + 1;
//    i = in_len;
//    char char_array[in_len];
//    in.toCharArray(char_array, in_len);
//    return char_array[in_len];
//}
//
// Created by shinsya on 2020/12/1.
//

#include "md5.h"
#include <stdio.h>

int main(){
    char buffer_input[512];
    gets(buffer_input);
    unsigned char* buffer_md5 = 0;
    _core_toolbox_md5_get(buffer_input, &buffer_md5);
    printf("%s\n", buffer_md5);
    getchar();
    return 0;
}

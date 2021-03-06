//
// Created by guoku on 2021/3/6.
//

#include "elevator_loader.h"

#include <stdio.h>
#include <stdlib.h>

const char *test_args[3] = {"-arg-test",
                            "-arg-2",
                            "-arg-sample"};
char *test_evr = "./elevator_sample/libelevator_sample.dll";

void printLine(char *text_t){
    puts(text_t);
    //putchar('\n');
}

int main(){
    evr_loader loader;
    evr_arg temp_arg;
    temp_arg.token = (unsigned int)&loader;
    temp_arg.args = test_args;
    temp_arg.argc = 3;
    temp_arg.pin = 0;
    memset(&temp_arg.func_list, 0, sizeof(evr_funcs));
    temp_arg.func_list.sys_printLine = (evrcall)printLine;
    if (loader.init(test_evr)){
        puts("Loader init successful.\n");
        loader.start(&temp_arg);
        loader.uninstall();
    }

    return 0;
}


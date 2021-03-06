#include "lib_evr.h"
#include "args_struct.h"

#include <Windows.h>

elevator_arg *global_arg;

const char *exit_notice = "Sample Module will exit after 10s.";

void print_t(char *text_t){
    if(global_arg->func_list.sys_printLine)
        global_arg->func_list.sys_printLine((void *)text_t);
}

void evr_entry(void *arg_struct){
    evr_arg *arg_s = (elevator_arg *)arg_struct;
    if(arg_s == 0)
        return;
    printArgs(arg_s);
    global_arg = arg_s;
    print_t((char *)"It was inited when you seen this message.");
}

void evr_exit(){
    print_t((char *)exit_notice);
    Sleep(10000);
    print_t((char *)"Sample exited.");
}

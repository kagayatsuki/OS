//
// Created by guoku on 2021/3/1.
//

#ifndef ELEVATOR_LOADER_ARGS_STRUCT_H
#define ELEVATOR_LOADER_ARGS_STRUCT_H


#include <stdio.h>

typedef void (*evrcall)(void *);  //标准elevator函数指针
typedef void *(*evrfunc)(void *);  //次要elevator函数指针
typedef void (*evrthread)();    //多线程入口函数

typedef struct elevator_funcs{  //传给elevator的函数列表，目前就这几个
    //evrcall list
    evrcall sys_printLine;
    evrcall sys_getLine;
    evrcall sys_printLog;
    evrcall sys_regModuleFunc;
    //evrfunc list
    evrfunc sys_getModuleName;      //获取当前elevator实例的名称
}evr_funcs;

typedef struct elevator_arg{
    evr_funcs func_list;
    const char **args;        //传入参数字符串数组
    unsigned int argc;  //传入参数个数
    unsigned int token; //elevator实例号，用于getModuleName等函数
    void *pin;      //未定义具体用途
}evr_arg;

void printArgs(elevator_arg *arg_s){
    if(arg_s){
        puts("=================Base Func List=================");
        printf("\tsys_printline\t%p\n", arg_s->func_list.sys_printLine);
        printf("\tsys_getline\t%p\n", arg_s->func_list.sys_getLine);
        printf("\tsys_printlog\t%p\n", arg_s->func_list.sys_printLog);
        printf("\tsys_regModuleFunc\t%p\n", arg_s->func_list.sys_regModuleFunc);
        printf("\tsys_getModuleName\t%p\n", arg_s->func_list.sys_getModuleName);
        int i = 0;
        puts("===================Init Args====================");
        for(; i < arg_s->argc; i++){
            printf("\t%d:%s\n", i, arg_s->args[i]);
        }
        printf("=================Token: %d===================\n", arg_s->token);
    }
}

#endif //ELEVATOR_LOADER_ARGS_STRUCT_H

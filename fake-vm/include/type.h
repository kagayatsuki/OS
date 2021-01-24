//
// Created by shinsya on 2020/12/27.
//

#ifndef FAKE_VM_TYPE_H
#define FAKE_VM_TYPE_H

typedef int vm_atom;

typedef void(*vm_call)(void *);
typedef void(*stand_interrupt_func_ptr)();

typedef struct{
    unsigned int code_length, code_offset;  //代码区域大小，偏移
    unsigned int data_length, data_offset;  //数据区域大小，偏移
    unsigned int entry_code;    //入口代码的地址
}EXINF_local;

typedef struct {
    unsigned int code_length;
    unsigned int data_length;
    unsigned short data_seg;
}EXINF_mem;

typedef struct{
    unsigned int code_ptr;
    unsigned int X[4];
}Runtime_register;

typedef struct{
    unsigned char operatorSize:4;     //操作数A&B的大小(bytes)需要相等,但不同指令可以有不同规范
    bool operatorA_register:1;  //操作数A是寄存器
    bool operatorB_register:1;  //操作数B是寄存器
    bool operatorA_addressing:1;    //操作数A进行寻址
    bool operatorB_addressing:1;    //操作数B进行寻址
}Code_Conf;

#endif //FAKE_VM_TYPE_H

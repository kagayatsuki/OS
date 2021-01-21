//
// Created by shinsya on 2021/1/17.
//

#include <stdio.h>
#include "memory.h"
#include "calls.h"

#define TEST_SEG 2
#define TEST_OFF 0x0FFF
#define READ_SEG 3
#define READ_OFF 0x0001
#define TEST_SET_DATA 'A'
#define TEST_SET_LEN 8

int main(){
    fakeVM_memory Mem(0x10000);
    char gets_tmp[32] = {0};
    printf("[init] Init memory unit.\n");
    if(Mem.init()){
        printf("[init] Failed.\n");
        return 0;
    }else
        printf("[init] Succeed.\n\n");

    memory_debug_print = false;

    uint16_t temp_p = 0x003E;
    char temp_string[] = "I try to write a string as long as very long, just for test.\n";
    vm_call_memcpy(&Mem, 0x0004, 0x0000, temp_string, 62);      //外部内存向内部复制方法
    Mem.push(sizeof(uint16_t), &temp_p);    //复制长度入栈
    temp_p = 0x0004;
    Mem.push(sizeof(uint16_t), &temp_p);    //源段入栈
    temp_p = 0x0000;
    Mem.push(sizeof(uint16_t), &temp_p);    //源偏移入栈
    temp_p = 0x0004;
    Mem.push(sizeof(uint16_t), &temp_p);    //目标段入栈
    temp_p = 0x003E;
    Mem.push(sizeof(uint16_t), &temp_p);    //目标偏移入栈
    vm_call_memcpy(&Mem);       //内部 内存复制方法
    temp_p = 0x0004;
    Mem.push(sizeof(uint16_t), &temp_p);    //print的参数-源段 入栈
    temp_p = 0x0000;
    Mem.push(sizeof(uint16_t), &temp_p);    //print的参数-源偏移 入栈
    vm_call_print(&Mem);    //执行print
    temp_p = 0x0004;
    Mem.push(sizeof(uint16_t), &temp_p);    //同上方print的参数入栈
    temp_p = 0x003E;    //复制的字符串偏移
    Mem.push(sizeof(uint16_t), &temp_p);
    vm_call_print(&Mem);
    return 0;
}

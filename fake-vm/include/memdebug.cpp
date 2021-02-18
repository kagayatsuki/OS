//
// Created by shinsya on 2021/1/17.
//

#include <stdio.h>
#include "memory.h"
#include "calls.h"
#include "runner.h"

#define TEST_SEG 2
#define TEST_OFF 0x0FFF
#define READ_SEG 3
#define READ_OFF 0x0001
#define TEST_SET_DATA 'A'
#define TEST_SET_LEN 8

int main(){
    /*
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
    */
    memory_debug_print = false;
    runner_debug_print = false;
    unsigned char testCode[128] = {0x10, 0x01, 0x26, 0x00,              //PUSH  0x000D      复制长度
                                  0x10, 0x01, 0x01, 0x00,               //PUSH  0x0001      字符串段入栈
                                  0x10, 0x01, 0x00, 0x00,               //PUSH  0x0000      字符串偏移入栈
                                  0x10, 0x01, 0x01, 0x00,               //PUSH  0x0001      复制目标段入栈
                                  0x10, 0x01, 0x26, 0x00,               //PUSH  0x000D      复制目标偏移入栈
                                  0x11, 0x13,                           //POP   EAX         测试pop 寄存器
                                  0x10, 0x13,                           //PUSH  EAX         测试push 寄存器
                                  0x20, 0x00, 0x02,                     //INT   0x02        中断0x02 内存复制
                                  0x10, 0x01, 0x01, 0x00,               //PUSH  0x0001      字符串段入栈
                                  0x10, 0x01, 0x00, 0x00,               //PUSH  0x0000      字符串偏移入栈
                                  0x20, 0x00, 0x01,                     //INT   0x01        中断0x01 输出栈中参数所指字符串
                                  0x10, 0x01, 0x01, 0x00,               //PUSH  0x0001      复制的字符串段入栈
                                  0x10, 0x01, 0x26, 0x00,               //PUSH  0x000D      复制的字符串偏移入栈
                                  0x20, 0x00, 0x01,                     //INT   0x01        中断0x01 输出栈中参数所指字符串
                                  0x10, 0x03, 0x00, 0x00, 0x00, 0x00,   //PUSH  0x00000000  return 0
                                  0x20, 0x00, 0x00,                     //INT   0x00        中断0x00 程序终止
                                  'H', 'e', 'l', 'l', 'o', ' ', 'W',
                                  'o', 'r', 'l', 'd', '.', 'T', 'h',
                                  'i', 's', ' ', 'i', 's', ' ', 'a',
                                  ' ', 's', 'i', 'm', 'p', 'l', 'e',
                                  ' ', 'p', 'r', 'o', 'g', 'r', 'a',
                                  'm', '\n', '\0'};   //数据段 0x0014起
    fakeVM_runner Runner(testCode, 0x0000, 0x003A, 0x0026);
    if(Runner.LaunchEntry()){
        //printf("done.\n");
    } else{
        printf("failed.\n");
    }

    return 0;
}

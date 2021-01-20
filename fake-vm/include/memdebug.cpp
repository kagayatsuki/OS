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
        printf("[init] Succeed.\n");

    char temp_string[18] = "This is a string\n";
    vm_call_memcpy(&Mem, 0x0004, 0x0000, temp_string, 18);
    uint16_t temp_p = 0x0004;
    Mem.push(sizeof(uint16_t), &temp_p);
    temp_p = 0x0000;
    Mem.push(sizeof(uint16_t), &temp_p);
    vm_call_print(&Mem);
    return 0;
}

//
// Created by shinsya on 2021/1/17.
//

#include <stdio.h>
#include "memory.h"

#define TEST_SEG 2
#define TEST_OFF 0x0FFF
#define READ_SEG 3
#define READ_OFF 0x0001
#define TEST_SET_DATA 'A'
#define TEST_SET_LEN 8

int main(){
    fakeVM_memory Mem(0x5000);
    char gets_tmp[32] = {0};
    printf("[init] Init memory unit.\n");
    if(Mem.init()){
        printf("[init] Failed.\n");
        return 0;
    }else
        printf("[init] Succeed.\n");
    Mem.set(TEST_SEG, TEST_OFF, TEST_SET_DATA, TEST_SET_LEN);
    printf("Seg %d, Off %d Were set\n");
    char tmp = Mem.get(READ_SEG, READ_OFF);
    Mem.gets(TEST_SEG, TEST_OFF, TEST_SET_LEN, gets_tmp);
    printf("Seg %d, Off %d:[%d]  %s\n", TEST_SEG, TEST_OFF, TEST_SET_LEN, gets_tmp);
    //printf("Seg %d, Off %d: %d | %c\n", READ_SEG, READ_OFF, tmp, tmp);
    return 0;
}

//
// Created by shinsya on 2021/1/17.
//

#include <stdio.h>
#include "memory.h"

#define TEST_SEG 3
#define TEST_OFF 32
#define TEST_SET_DATA 'C'
#define TEST_SET_LEN 8

int main(){
    fakeVM_memory Mem(0x5000);
    printf("Init memory unit.\n");
    if(Mem.init()){
        printf("Failed.\n");
        return 0;
    }else
        printf("Succeed.\n");
    Mem.set(TEST_SEG, TEST_OFF, TEST_SET_DATA, TEST_SET_LEN);
    printf("Seg %d, Off %d Were set\n");
    char tmp = Mem.get(TEST_SEG, TEST_OFF);
    printf("Seg %d, Off %d: %d | %c\n", TEST_SEG, TEST_OFF, tmp, tmp);
    return 0;
}

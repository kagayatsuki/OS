//
// Created by shinsya on 2020/12/27.
//

#ifndef FAKE_VM_STACK_H
#define FAKE_VM_STACK_H

#include <stdlib.h>
#include "type.h"
#include <string>

#define FAKEVM_STACK_EXCEPTION_OUT 1
#define FAKEVM_STACK_EXCEPTION_ILLEGAL 2

class FakeVM_Stack{
    FakeVM_Stack();
    ~FakeVM_Stack();
    bool init(uint32_t stack_size);
    bool push(void* buff, uint32_t size);
    bool pop(void* buff, uint32_t size);
private:
    void* stack_mem;
    uint32_t offset, stack_size;
    uint8_t error_num;
};

FakeVM_Stack::FakeVM_Stack() {
    error_num = stack_mem = offset = 0;     //操作参数初始化
}

FakeVM_Stack::~FakeVM_Stack() {
    if(stack_mem)
        delete [] (char *)stack_mem;    //释放伪栈内存
}

bool FakeVM_Stack::init(uint32_t size) {
    if((size==0) || stack_mem)
        return false;
    stack_size = size;
    stack_mem = new char[stack_size];
    memset(stack_mem, 0, stack_size);
    return true;
}

bool FakeVM_Stack::push(void *buff, uint32_t size) {
    if(!buff || !size){
        error_num = FAKEVM_STACK_EXCEPTION_ILLEGAL;
        return false;
    }if(size + offset > stack_size){
        error_num = FAKEVM_STACK_EXCEPTION_OUT;
        return false;
    }
    for(int i = 0; i < size; i++)
        *((char *)stack_mem + offset + i) = *((char *)buff + size - (i + 1));     //从高字节到低字节放入
    offset+=size;
    error_num = 0;
    return true;
}

bool FakeVM_Stack::pop(void *buff, uint32_t size) {
    if(!size){
        error_num = FAKEVM_STACK_EXCEPTION_ILLEGAL;
        return false;
    }if(offset - size < 0){
        error_num = FAKEVM_STACK_EXCEPTION_OUT;
        return false;
    }if(buff){
        for(int i = 0; i < size; i++)
            *((char *)buff + i) = *((char *)stack_mem - (i + 1));   //从低字节到高字节放出
    }
    offset-=size;
    error_num = 0;
    return true;
}

#endif //FAKE_VM_STACK_H

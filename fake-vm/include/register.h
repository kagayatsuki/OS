//
// Created by shinsya on 2020/12/27.
//

#ifndef FAKE_VM_REGISTER_H
#define FAKE_VM_REGISTER_H

#include "type.h"
#include "stack.h"

class FakeVM_Register{
    char X[8], EX[16];
    uint32_t ESI, EDI;  //索引暂存
    uint32_t *EBP, *ESP;  //栈指针
    uint32_t FLAG, EIP; //状态寄存器

    FakeVM_Register(FakeVM_Stack* stack_set);
protected:
    FakeVM_Stack* stack;
};

FakeVM_Register::FakeVM_Register(FakeVM_Stack *stack_set) {
    stack = stack_set;
    if(stack_set){
        EBP = stack->stack_mem;
        ESP = &stack->offset;
    }
    memset(X, 0, 8);
    memset(EX, 0, 16);
    ESI = EDI = FLAG = EIP = 0;
}

#endif //FAKE_VM_REGISTER_H

//
// Created by shinsya on 2020/12/27.
//

#ifndef FAKE_VM_INTERRUPT_H
#define FAKE_VM_INTERRUPT_H

#include <stdlib.h>
#include "memory.h"
#include "type.h"

class fakeVM_Interrupter{
    fakeVM_Interrupter();
    void SetInt(vm_call func, unsigned char id);
    void Int(unsigned char id, fakeVM_memory *mem);
private:
    vm_call interruptFuncs[255];    //0号中断固定为终止程序
};

fakeVM_Interrupter::FakeVM_Interrupter() {
    memset(interruptFuncs, 0, sizeof(vm_call)*255);
}

void fakeVM_Interrupter::SetInt(vm_call func, unsigned char id) {
    if((id == 0) || (id > 255))
        return;
    interruptFuncs[id-1] = func;
}

void fakeVM_Interrupter::Int(unsigned char id, fakeVM_memory *mem) {
    if((id == 0) || (id > 255))
        return;
    if(interruptFuncs[id-1])
        interruptFuncs[id-1](mem);
}

#endif //FAKE_VM_INTERRUPT_H

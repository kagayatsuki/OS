//
// Created by shinsya on 2020/12/27.
//

#ifndef FAKE_VM_INTERRUPT_H
#define FAKE_VM_INTERRUPT_H

#include <stdlib.h>
#include "type.h"

class FakeVM_Interrupter{
    FakeVM_Interrupter();
    void SetInt(stand_interrupt_func_ptr func, char id);
    void Int(char id);
private:
    stand_interrupt_func_ptr interruptFuncs[256];
};

FakeVM_Interrupter::FakeVM_Interrupter() {
    memset(interruptFuncs, 0, sizeof(stand_interrupt_func_ptr)*256);
}

void FakeVM_Interrupter::SetInt(stand_interrupt_func_ptr func, unsigned char id) {
    interruptFuncs[id] = func;
}

void FakeVM_Interrupter::Int(unsigned char id) {
    if(interruptFuncs[id]){
        interruptFuncs[id]();
    }
}

#endif //FAKE_VM_INTERRUPT_H

//
// Created by shinsya on 2020/12/27.
//

#ifndef FAKE_VM_INTERRUPT_H
#define FAKE_VM_INTERRUPT_H

#include <stdlib.h>
#include "memory.h"
#include "type.h"

class FakeVM_Interrupter{
    FakeVM_Interrupter();
    void SetInt(vm_ptr func, unsigned char id);
    vm_ptr Int(unsigned char id);
private:
    vm_ptr interruptFuncs[256];
};

FakeVM_Interrupter::FakeVM_Interrupter() {
    memset(interruptFuncs, 0, sizeof(vm_ptr)*256);
}

void FakeVM_Interrupter::SetInt(vm_ptr func, unsigned char id) {
    interruptFuncs[id] = func;
}

vm_ptr FakeVM_Interrupter::Int(unsigned char id) {
    return interruptFuncs[id];
}

#endif //FAKE_VM_INTERRUPT_H

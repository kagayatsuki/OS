//
// 中间层中断函数
// Created by shinsya on 2020/12/27.
//

#ifndef FAKE_VM_CALLS_H
#define FAKE_VM_CALLS_H

#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

void vm_call_print(fakeVM_memory *mem){
    uint16_t seg, ptr;
    uint32_t addr;
    if(mem){
        mem->pop(sizeof(uint16_t), &ptr);
        mem->pop(sizeof(uint16_t), &seg);
        CC_DEBUG printf("[printf] seg: %d offset: %d\n", seg, ptr);
        addr = seg * fakeVM_memory_segment + ptr;
        char tmp = mem->get(seg, ptr);
        while(tmp != 0){
            putchar(tmp);
            addr++;
            mem->addr(seg, ptr, addr);
            tmp = mem->get(seg, ptr);
        }
    }
}

void vm_call_memcpy(fakeVM_memory *mem){
    uint16_t seg_des, ptr_des, seg_src, ptr_src;
    uint32_t addr_des, addr_src;
    uint16_t cp_size;
    if(mem){
        mem->pop(sizeof(uint16_t), &ptr_des);
        mem->pop(sizeof(uint16_t), &seg_des);
        addr_des = seg_des * fakeVM_memory_segment + ptr_des;
        mem->pop(sizeof(uint16_t), &ptr_src);
        mem->pop(sizeof(uint16_t), &seg_src);
        addr_src = seg_src * fakeVM_memory_segment + ptr_src;
        mem->pop(sizeof(uint16_t), &cp_size);

        //TODO: 优化内存复制算法
        //debug
        CC_DEBUG printf("[memcpy %d] ", cp_size);
        char tmp;
        for(; cp_size > 0; cp_size--){  //先用着这个低效率的方法，之后想办法优化
            tmp = mem->get(seg_src, ptr_src);
            mem->set(seg_des, ptr_des, tmp, 1);
            CC_DEBUG printf("%02x ", mem->get(seg_des, ptr_des));
            addr_des++;
            addr_src++;
            mem->addr(seg_des, ptr_des, addr_des);
            mem->addr(seg_src, ptr_src, addr_src);
        }
        CC_DEBUG putchar('\n');
    }
}

void vm_call_memcpy(fakeVM_memory *mem, uint16_t seg, uint16_t offset, void* src, uint16_t size){
    uint16_t seg_des = seg, ptr_des = offset;
    uint32_t addr_des;

    //debug
    CC_DEBUG printf("[memcpy %d] ", size);
    //TODO: 优化内存复制算法
    if(mem && src){
        addr_des = seg_des * fakeVM_memory_segment + ptr_des;
        for(int i = 0; i < size; i++){
            mem->set(seg_des, ptr_des, *((char*)src + i), 1);
            CC_DEBUG printf("%02x ", mem->get(seg_des, ptr_des));
            addr_des++;
            mem->addr(seg_des, ptr_des, addr_des);
        }
        CC_DEBUG putchar('\n');
    }
}

void vm_call_func(fakeVM_memory *mem){


}

#endif //FAKE_VM_CALLS_H

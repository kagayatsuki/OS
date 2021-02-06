//
// Created by shinsya on 2021/2/6.
//

#ifndef FAKE_VM_RUNNER_COM_H
#define FAKE_VM_RUNNER_COM_H

#include "type.h"
#include "memory.h"
#include "permissions.h"
#include "calls.h"

#define VM_MEM_P fakeVM_memory *

typedef struct vm_paramSegment{
    unsigned char Param8A, Param8B;     //8位的缓冲区
    uint16_t Param16A, Param16B;        //16位缓冲区
    uint32_t Param32A, Param32B;        //32位缓冲区
    uint16_t ParamSeg, ParamOff;    //ParamSeg一般用于临时存放当前指令操作数的段, ParamOff存放它的偏移
    Runtime_register *reg;      //指令执行中的寄存器
    VM_MEM_P mem;               //指令执行中的内存区域
    EXINF_local *localEXI;      //可执行程序本地镜像信息
    EXINF_mem *memEXI;          //可执行程序内存镜像信息
}VMCache;


uint16_t _com_push(VMCache *thisCache, Code_Conf conf){
    uint16_t code_offset = 0x0002;
    /** 对于operatorSize记录的数据长度，从0开始记，所以实际值为+1 **/
    char *ParamTemp = new char[conf.operatorSize + 0x01];    //临时内存
    thisCache->mem->addr(thisCache->ParamSeg, thisCache->ParamOff, thisCache->reg->code_ptr + 0x0002);   //首个操作数必然从指令地址+2处开始
    if (conf.operatorA_register) {  //操作数是寄存器
        /** 当操作数为寄存器时, 寄存器的id由opB_r作低1位 和 opB_a作高1位 组成 **/
        uint16_t opId = conf.operatorB_register + conf.operatorB_addressing * 2;
        if (conf.operatorA_addressing) {  //间接寻址
            thisCache->ParamSeg = ((uint16_t)thisCache->reg->X[opId] >> 0x10) & 0xFFFF; //前两个字节是段参数
            thisCache->ParamOff = ((uint16_t)thisCache->reg->X[opId] & 0xFFFF);    //后两个字节是偏移参数
            thisCache->mem->gets(thisCache->ParamSeg + thisCache->memEXI->data_seg, thisCache->ParamOff, conf.operatorSize + 0x01, ParamTemp);
            thisCache->mem->push(conf.operatorSize + 0x01, ParamTemp);  //间接寻址数据压栈
        } else {
            thisCache->mem->push(sizeof(uint32_t), &thisCache->reg->X[opId]); //寄存器数据压栈
        }
    } else if (conf.operatorA_addressing) {  //操作数是地址,直接寻址取数据
        thisCache->mem->gets(thisCache->ParamSeg, thisCache->ParamOff, sizeof(uint16_t), &thisCache->Param16A);  //push 寻址段参数
        thisCache->mem->addr(thisCache->ParamSeg, thisCache->ParamOff, thisCache->reg->code_ptr + 0x0004);
        thisCache->mem->gets(thisCache->ParamSeg, thisCache->ParamOff, sizeof(uint16_t), &thisCache->Param16B);  //push 寻址偏移参数
        thisCache->mem->gets(thisCache->Param16A + thisCache->memEXI->data_seg, thisCache->Param16B, conf.operatorSize + 0x01, ParamTemp); //寻址(基址为data_seg) 得数据至临时内存
        thisCache->mem->push(conf.operatorSize + 0x01, ParamTemp); //寻址数据压栈
        code_offset += sizeof(uint32_t);    //总共32位大小的操作数
    } else {  //操作数是立即数
        thisCache->mem->gets(thisCache->ParamSeg, thisCache->ParamOff, conf.operatorSize + 0x0001, ParamTemp);     //获取立即数
        thisCache->mem->push(conf.operatorSize + 0x0001, ParamTemp); //立即数压栈
        code_offset += conf.operatorSize + 0x0001;
    }
    delete [] ParamTemp;
    return code_offset;
}

uint16_t _com_pop(VMCache *thisCache, Code_Conf conf){
    uint16_t code_offset = 0x0002;
    char *ParamTemp = new char[conf.operatorSize + 0x01];
    thisCache->mem->addr(thisCache->ParamSeg, thisCache->ParamOff, thisCache->reg->code_ptr + 0x0002);
    if(conf.operatorA_register){        //向寄存器pop
        /** 当操作数为寄存器时, 寄存器的id由opB_r作低1位 和 opB_a作高1位 组成 **/
        uint16_t opId = conf.operatorB_register + conf.operatorB_addressing * 2;
        if(conf.operatorA_addressing){      //向地址
            thisCache->ParamSeg = (uint16_t)(thisCache->reg->X[opId] >> 0x10);      //寄存器数据的高两字节为段参数
            thisCache->ParamOff = (uint16_t)(thisCache->reg->X[opId] & 0xFFFF);     //低两字节为偏移参数
            thisCache->mem->pop(conf.operatorSize + 0x01, ParamTemp);
            vm_call_memcpy_ex(thisCache->mem, thisCache->ParamSeg, thisCache->ParamOff, ParamTemp, conf.operatorSize + 0x01);
        }else { //向寄存器
            thisCache->mem->pop(sizeof(uint32_t), &thisCache->reg->X[opId]);
        }
    }else if(conf.operatorA_addressing){    //直接寻址
        thisCache->mem->pop(conf.operatorSize + 0x01, ParamTemp);
        thisCache->mem->gets(thisCache->ParamSeg, thisCache->ParamOff, sizeof(uint16_t), &thisCache->Param16A);  //段参数
        thisCache->mem->addr(thisCache->ParamSeg, thisCache->ParamOff, thisCache->reg->code_ptr + 0x0004);
        thisCache->mem->gets(thisCache->ParamSeg, thisCache->ParamOff, sizeof(uint16_t), &thisCache->Param16B);  //偏移参数
        vm_call_memcpy_ex(thisCache->mem, thisCache->Param16A, thisCache->Param16B, ParamTemp, conf.operatorSize + 0x01);
        code_offset += 0x0004;
    }
    delete [] ParamTemp;
    return code_offset;
}

#endif //FAKE_VM_RUNNER_COM_H

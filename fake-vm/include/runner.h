//
// Created by shinsya on 2021/1/24.
//

#ifndef FAKE_VM_RUNNER_H
#define FAKE_VM_RUNNER_H

#include <string.h>
#include "memory.h"
#include "calls.h"
#include "interrupt.h"
#include "permissions.h"
#include "type.h"
#include "runner_com.h"

#define VM_UNIBUFFER_SIZE 0x1000

#define CD_RUNNER if(runner_debug_print)

static bool runner_debug_print = true;

/** 输出指令配置详情 **/
void debug_print_opConf(Code_Conf conf){
    printf("AReg: %1d BReg: %1d AAdr: %1d BAdr: %1d OpSize: %d", conf.operatorA_register, conf.operatorB_register, conf.operatorA_addressing, conf.operatorB_addressing, conf.operatorSize);
}

class fakeVM_runner{
    fakeVM_memory *thisMemory;
    fakeVM_Interrupter *thisIntList;
    RuntimePermission *thisPermission;

    void _CodeExplain(unsigned char code_t, Code_Conf conf, VMCache *ComCache);
    void _CodePosition(uint32_t offset);

    void _ThreadDestroy();
    void _MainThread();

    void _InitDefaultInterrupt();
    bool _ImageCheck();
protected:
    char *uniBuffer;    //通用缓冲区
    char *localFile;
    uint32_t uniBuffer_size, uniBuffer_used;    //实例中通用缓冲区长度和使用量

    EXINF_local *exi_l;  //本地映像信息
    EXINF_mem *exi_m;    //内存映像信息

    Runtime_register thisRegister;  //模拟寄存器

    VMCache mainCache;   //主线程下的参数段&偏移缓存变量

    int Running, ExitCode; //运行状态指示器, 退出码
    int InitError;
public:
    fakeVM_runner();
    fakeVM_runner(void *filename);
    fakeVM_runner(void *image_mem, uint16_t entry, uint16_t code_size, uint16_t data_size); //调试初始化方法，代码区和数据区要连续


    bool LaunchEntry();
};

//暂时还想不到什么办法把解释器的代码更好的精简或模块化
void fakeVM_runner::_CodeExplain(unsigned char code_t, Code_Conf conf, VMCache *ComCache) {
    /** Debug 信息 **/
    CC_DEBUG printf("[runner 0x%02x] op_size: %d op_conf: ", code_t, conf.operatorSize);
    CC_DEBUG debug_print_bin(conf);
    CC_DEBUG putchar(' ');
    CD_RUNNER debug_print_opConf(conf);
    CC_DEBUG putchar('\n');

    /** 指令解释 **/
    if(code_t == 32){   //Int
        thisMemory->addr(ComCache->ParamSeg, ComCache->ParamOff, thisRegister.code_ptr + 0x0002);   //获取中断编号
        thisMemory->gets(ComCache->ParamSeg, ComCache->ParamOff, sizeof(unsigned char), &ComCache->Param8A);
        if(ComCache->Param8A == 0){  //Int 0 退出程序
            thisMemory->pop(sizeof(uint32_t), &ComCache->Param32B);   //弹出退出代码
            Running = 0;
            ExitCode = ComCache->Param32B;
            return;
        }
        thisIntList->Int(ComCache->Param8A, thisMemory);
        _CodePosition(thisRegister.code_ptr + 0x0003);
        return;
    }else{
        uint32_t code_off = 0;
        switch (code_t) {
            case 0x10:     //push
                code_off = _com_push(ComCache, conf);
                break;
            case 0x11:     //pop
                code_off = _com_pop(ComCache, conf);
                break;
            case 0x12:
                code_off = _com_mv(ComCache, conf);
                break;
            default:
                Running = 0;
                ExitCode = -329602;
                return;
        }
        _CodePosition(thisRegister.code_ptr + code_off);
        return;
    }
}

void fakeVM_runner::_CodePosition(uint32_t offset) {
    thisRegister.code_ptr = offset;
}

void fakeVM_runner::_MainThread() {
    thisRegister.code_ptr = exi_l->entry_code;
    Running = 1;
    uint16_t run_seg = 0, run_ptr = thisRegister.code_ptr;              //代码段&偏移
    uint16_t conf_seg = 0, conf_ptr = thisRegister.code_ptr + 1;        //代码配置段&偏移
    unsigned char code_t = 0;
    /** mainCache 初始化 **/
    memset(&mainCache, 0, sizeof(VMCache));
    mainCache.reg = &thisRegister;
    mainCache.memEXI = exi_m;
    mainCache.localEXI = exi_l;
    mainCache.mem = thisMemory;

    Code_Conf conf_t;
    while (Running){
        code_t = thisMemory->get(conf_seg, conf_ptr);   //获取代码配置
        memcpy(&conf_t, &code_t, sizeof(unsigned char));
        code_t = thisMemory->get(run_seg, run_ptr);     //获取代码
        _CodeExplain(code_t, conf_t, &mainCache);   //处理代码
        thisMemory->addr(run_seg, run_ptr, thisRegister.code_ptr);
        thisMemory->addr(conf_seg, conf_ptr, thisRegister.code_ptr + 1);
    }
    printf("Program has exited. Code: %d\n", ExitCode); //暂时不算做调试输出
}

bool fakeVM_runner::_ImageCheck() {
    if(exi_m)   //debug方法后，跳过检查
        return true;
    if(!localFile){
        InitError = -3; //没有提供文件名
        return false;
    }
    FILE *tmp = fopen(localFile, "r+");
    if(!tmp){
        InitError = -2; //文件无法打开
        return false;
    }
    char tmp_magic[4] = {0};
    fread(tmp_magic, 4, 1, tmp);
    if(strcmp(tmp_magic, "OSEC")){
        fclose(tmp);
        InitError = -4; //错误的文件
        return false;
    }
    fseek(tmp, 0, SEEK_END);
    EXINF_local *tmp_exinf_l = new EXINF_local;
    uint32_t tmp_file_len = ftell(tmp); //文件长度
    fseek(tmp, 0, SEEK_SET);
    /** 读取文件头信息 **/
    fread(&tmp_exinf_l->code_length, sizeof(unsigned int), 1, tmp);
    fread(&tmp_exinf_l->code_offset, sizeof(unsigned int), 1, tmp);
    fread(&tmp_exinf_l->data_length, sizeof(unsigned int), 1, tmp);
    fread(&tmp_exinf_l->data_offset, sizeof(unsigned int), 1, tmp);
    fread(&tmp_exinf_l->entry_code, sizeof(unsigned int), 1, tmp);
    if((tmp_exinf_l->data_length + tmp_exinf_l->code_length) > tmp_file_len){   //代码加上数据的长度大于文件长度,显然错误
        fclose(tmp);
        delete tmp_exinf_l;
        InitError = -5;
        return false;
    }
    /** 数据信息错误 **/
    if(((tmp_exinf_l->code_offset + tmp_exinf_l->code_length) > tmp_file_len) || ((tmp_exinf_l->data_offset + tmp_exinf_l->data_length) > tmp_file_len)){
        fclose(tmp);
        delete tmp_exinf_l;
        InitError = -6;
        return false;
    }
    /** 数据正式化 **/
    exi_l = tmp_exinf_l;
    exi_m = new EXINF_mem;
    exi_m->data_length = exi_l->data_length;
    exi_m->code_length = exi_m->code_length;
    exi_m->data_seg = exi_m->code_length / fakeVM_memory_segment + 1;
    /** 载入数据 **/
    fseek(tmp, exi_l->code_offset, SEEK_SET);
    char *buff = new char[fakeVM_memory_segment];
    int ci = 0, cMod = exi_m->code_length % fakeVM_memory_segment;
    for(; ci < exi_m->code_length / fakeVM_memory_segment; ci++){     //载入代码
        fread(buff, fakeVM_memory_segment, 1, tmp);
        vm_call_memcpy_ex(thisMemory, ci, 0, buff, fakeVM_memory_segment);
    }
    if(cMod){
        fread(buff, cMod, 1, tmp);
        vm_call_memcpy_ex(thisMemory, ci, 0, buff, cMod);
        ci++;
    }
    fseek(tmp, exi_l->data_offset, SEEK_SET);
    int di = 0, dMod = exi_m->data_length % fakeVM_memory_segment;
    for(; di < exi_m->data_length / fakeVM_memory_segment; di++){       //载入数据
        fread(buff, fakeVM_memory_segment, 1, tmp);
        vm_call_memcpy_ex(thisMemory, di + exi_m->data_seg, 0, buff, fakeVM_memory_segment);
    }
    if(dMod){
        fread(buff, dMod, 1, tmp);
        vm_call_memcpy_ex(thisMemory, di + exi_m->data_seg, 0, buff, dMod);
    }
    delete [] buff;
    fclose(tmp);
    return true;
}

void fakeVM_runner::_InitDefaultInterrupt() {
    thisIntList = new fakeVM_Interrupter();
    thisIntList->SetInt(vm_call_print, 1);
    thisIntList->SetInt(vm_call_memcpy, 2);
}

/** 打算先完成调试用构造函数 **/
fakeVM_runner::fakeVM_runner(void *image_mem, uint16_t entry, uint16_t code_size, uint16_t data_size)
    :localFile(0), uniBuffer_size(VM_UNIBUFFER_SIZE), uniBuffer_used(0), exi_l(0), exi_m(0), InitError(0)
    ,Running(0), ExitCode(0)
{
    /** 初始化通用缓冲区 **/
    uniBuffer = new char[uniBuffer_size];
    /** 以默认方式初始化memory类 **/
    thisMemory = new fakeVM_memory();
    if (thisMemory->init()){
        InitError = -1; //memory类未能初始化
        return;
    }
    /** 开始初始化程序结构 **/
    if(image_mem){  //非空地址
        if(entry < code_size){  //入口地址合法
            /** 基本信息初始化,用于直接通过 _ImageCheck的检查 **/
            exi_m = new EXINF_mem;
            exi_l = new EXINF_local;
            exi_l->entry_code = entry;
            exi_m->code_length = code_size;
            exi_m->data_length = data_size;
            exi_m->data_seg = code_size / fakeVM_memory_segment + 1;
            /** 复制代码到内存(memory类的0x00000000开始) **/
            vm_call_memcpy_ex(thisMemory, 0x0000, 0x0000, image_mem, code_size);
            vm_call_memcpy_ex(thisMemory, exi_m->data_seg, 0x0000, ((char *)image_mem + code_size), data_size);
            /** 初始化默认模拟中断表 **/
            _InitDefaultInterrupt();
        }
    }
}

bool fakeVM_runner::LaunchEntry() {
    if(Running)
        return false;
    if(!_ImageCheck())
        return false;
    _MainThread();
    return true;
}

#endif //FAKE_VM_RUNNER_H

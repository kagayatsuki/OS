//
// Created by shinsya on 2021/1/17.
//

#ifndef FAKE_VM_MEMORY_H
#define FAKE_VM_MEMORY_H

#include <stdlib.h>
#include <string.h>

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;

typedef unsigned long vm_ptr;       //内存指针

#define fakeVM_memory_default 0x7A12000     /** 默认申请的内存是128MB **/
#define fakeVM_memory_lowest 0x1000     /** 最低内存为4KB **/
#define fakeVM_memory_segment 0x1000    /** 以4KB为一段 **/

#define CC_OFFSET if(offset > fakeVM_memory_segment){ err_num = -1; return 0;}
#define CC_SEGMENT if(segment + 1 > memory_size / fakeVM_memory_segment + ((memory_size % fakeVM_memory_segment) ? 1 : 0)){ err_num = -3; return -1;}
#define CC_SEGMENT_U if(segment + 1 > memory_size / fakeVM_memory_segment + ((memory_size % fakeVM_memory_segment) ? 1 : 0)){ err_num = -3; return 0;}

#define ERR_RESET err_num = 0;

typedef struct fakeVM_memtab{
    char *segment_ptr;
    fakeVM_memtab *next_node;
}mem_map;

class fakeVM_memory{
    uint32_t memory_size;
    mem_map *mmap;

    void destroy(mem_map *p);
    char* getSegment(uint16_t segment);
    void operator_split(uint32_t &first, uint32_t &mid_c, uint32_t &final, uint16_t offset, uint32_t size);
protected:
    int err_num;    //操作返回码
public:
    fakeVM_memory(){memory_size = fakeVM_memory_default; mmap = 0;}
    fakeVM_memory(uint32_t mem_size){memory_size = mem_size; mmap = 0;}
    ~fakeVM_memory(){if(mmap)destroy(mmap);}

    int init();     //初始化内存空间
    void addr(uint16_t &segment, uint16_t &offset, uint32_t addr){segment = addr / fakeVM_memory_segment; offset = addr % fakeVM_memory_segment;}    //由地址得段和偏移
    uint32_t set(uint16_t segment, uint16_t offset, char data, uint32_t size);      //memset实现
    char get(uint16_t segment, uint16_t offset);    //读一个字节
    uint32_t gets(uint16_t segment, uint16_t offset, uint32_t size, void* ptr);    //复制一段内存到buffer
    void* getAddr_unsafe(vm_ptr ptr);   //获取映射的真实地址(对于操作系统仍是虚拟地址) - 不安全方法
};

void * fakeVM_memory::getAddr_unsafe(vm_ptr ptr) {
    uint16_t seg, off;
    addr(seg, off, ptr);
    char *tmp = getSegment(seg);
    if(tmp)
        return tmp+off;
    return 0;
}

uint32_t fakeVM_memory::gets(uint16_t segment, uint16_t offset, uint32_t size, void *ptr) {
    CC_SEGMENT_U
    CC_OFFSET
    uint32_t first = 0, mid_c = 0, final = 0, gets_c = 0;
    if(size > (memory_size - (segment * fakeVM_memory_segment + offset))){
        err_num = -1;
        return 0;
    }
    char *local = getSegment(segment), *op = (char*)ptr;
    if(!local || !ptr){
        err_num = -2;
        return 0;
    }
    operator_split(first, mid_c, final, offset, size);

    if(first){
        memcpy(op, local + offset, first);
        gets_c+=first;
        op+=first;
        for(; mid_c > 0; mid_c--){
            segment++;
            local = getSegment(segment);
            if(local){
                memcpy(op, local, fakeVM_memory_segment);
                op+=fakeVM_memory_segment;
                gets_c+=fakeVM_memory_segment;
            }else{
                err_num = -3;
                return gets_c;
            }
        }
        if(final){
            segment++;
            local = getSegment(segment);
            if(local){
                memcpy(op, local, final);
                gets_c += final;
            }
        }
    }

    ERR_RESET
    return gets_c;
}

void fakeVM_memory::operator_split(uint32_t &first, uint32_t &mid_c, uint32_t &final, uint16_t offset, uint32_t size) {
    first = (((offset+size) > fakeVM_memory_segment) ? fakeVM_memory_segment - offset : size);
    mid_c = (((offset+size) > fakeVM_memory_segment) ? (size - (fakeVM_memory_segment - offset)) / fakeVM_memory_segment : 0);
    final = (((offset+size) > fakeVM_memory_segment) ? (size - (fakeVM_memory_segment - offset)) % fakeVM_memory_segment : 0);
}

char fakeVM_memory::get(uint16_t segment, uint16_t offset) {
    CC_OFFSET
    CC_SEGMENT
    char* local_src = getSegment(segment);
    return *(local_src+offset);
}

uint32_t fakeVM_memory::set(uint16_t segment, uint16_t offset, char data, uint32_t size) {
    CC_OFFSET   //非法偏移
    if(size > (memory_size - (segment * fakeVM_memory_segment + offset)))       //地址越界
        return 0;
    uint32_t set_c = 0;     //实际操作的字节数
    char *local = getSegment(segment);
    if(!local){
        err_num = -2;
        return 0;
    }
    uint32_t first = 0,  //初始段内操作长度
        mid_c = 0,  //中间整段数（如果有）
        final = 0;  //尾段长度（如果有）
    operator_split(first, mid_c, final, offset, size);

    //debug out
    //printf("Set -first:%d -mid:%d -final:%d\n", first, mid_c, final);

    if(first){
        memset(local+offset, data, first);  //首段
        set_c+=first;
        for(; mid_c > 0; mid_c--){  //中间段
            segment++;
            local = getSegment(segment);
            if(local){
                memset(local, data, fakeVM_memory_segment);
                set_c+=fakeVM_memory_segment;
            }else{
                err_num = -3;
                return set_c;
            }
        }
        if(final){  //尾端
            segment++;
            local = getSegment(segment);
            if(local){
                memset(local, data, final);
                set_c+=final;
            }
        }
    }
    ERR_RESET
    return set_c;
}

char * fakeVM_memory::getSegment(uint16_t segment) {
    if(segment + 1 > memory_size / fakeVM_memory_segment + ((memory_size % fakeVM_memory_segment) ? 1 : 0))
        return 0;
    if(!mmap)
        return 0;
    mem_map *tmp = mmap;
    //debug
    //printf("Get Seg %d = ", segment);
    for(uint16_t i = 0; i < segment; i++)
        tmp = tmp->next_node;
    //debug
    //printf("%p\n", tmp->segment_ptr);
    return tmp->segment_ptr;
}

void fakeVM_memory::destroy(mem_map *p) {
    if(p->next_node)
        destroy(p->next_node);
    if(p->segment_ptr)
        delete [] p->segment_ptr;
    delete p;
}

int fakeVM_memory::init() {
    if(memory_size<fakeVM_memory_lowest)
        return -1;
    if(mmap)
        return -2;
    mem_map *tmp = mmap = new mem_map;
    uint32_t segment_c = (memory_size / fakeVM_memory_segment) + ((memory_size % fakeVM_memory_segment) ? 1 : 0);
    //printf("[init] Init : mem = %d\n       segment count = %d\n", memory_size, segment_c);
    for(int i = 0; i < segment_c; i++){
        tmp->segment_ptr = new char[fakeVM_memory_segment];
        if(i+1 < segment_c){
            tmp->next_node = new mem_map;
            tmp = tmp->next_node;
        }else{
            tmp->next_node = 0;
        }
    }
    //printf("[init] Seg0 : %p\n", mmap->segment_ptr);
    return 0;
}

#endif //FAKE_VM_MEMORY_H

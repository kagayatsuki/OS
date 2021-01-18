//
// Created by shinsya on 2021/1/17.
//

#ifndef FAKE_VM_MEMORY_H
#define FAKE_VM_MEMORY_H

#include <stdlib.h>
#include <string.h>

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;

#define fakeVM_memory_default 0x7A12000     /** 默认申请的内存是128MB **/
#define fakeVM_memory_lowest 0x1000     /** 最低内存为4KB **/
#define fakeVM_memory_segment 0x1000    /** 以4KB为一段 **/

#define CC_OFFSET if(offset > fakeVM_memory_segment){ err_num = -1; return 0;}
#define CC_SEGMENT if(segment + 1 > memory_size / fakeVM_memory_segment + ((memory_size % fakeVM_memory_segment) ? 1 : 0)){ err_num = -3; return -1;}

typedef struct fakeVM_memtab{
    char *segment_ptr;
    fakeVM_memtab *next_node;
}mem_map;

class fakeVM_memory{
    uint32_t memory_size;
    mem_map *mmap;

    void destroy(mem_map *p);
    char* getSegment(uint16_t segment);
    void operator_split(int &first, int &mid_c, int &final, uint16_t segment, uint16_t offset, uint32_t size);
protected:
    int err_num;    //操作返回码
public:
    fakeVM_memory(){memory_size = fakeVM_memory_default; mmap = 0;}
    fakeVM_memory(uint32_t mem_size){memory_size = mem_size; mmap = 0;}
    ~fakeVM_memory(){if(mmap)destroy(mmap);}

    int init();     //初始化内存空间
    void addr(uint16_t &segment, uint16_t &offset, uint32_t addr){segment = addr / fakeVM_memory_segment; offset = addr % fakeVM_memory_segment;}    //由地址得段和偏移
    uint32_t set(uint16_t segment, uint16_t offset, char data, uint32_t size);      //memset实现
    char get(uint16_t segment, uint16_t offset);
    uint32_t gets(uint16_t segment, uint16_t offset, uint32_t size, uint16_t seg_des, uint16_t off_des);
};

void fakeVM_memory::operator_split(int &first, int &mid_c, int &final, uint16_t segment, uint16_t offset,
                                   uint32_t size) {
    first = size + offset > (fakeVM_memory_segment ? fakeVM_memory_segment - offset : size);
    mid_c = size + offset > (fakeVM_memory_segment ? (size - (fakeVM_memory_segment - offset)) / fakeVM_memory_segment : 0);
    final = size + offset > (fakeVM_memory_segment ? (size - (fakeVM_memory_segment - offset)) % fakeVM_memory_segment : 0);
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
    char* local = getSegment(segment);
    if(!local){
        err_num = -2;
        return 0;
    }
    int first = 0,  //初始段内操作长度
        mid_c = 0,  //中间整段数（如果有）
        final = 0;  //尾段长度（如果有）
    operator_split(first, mid_c, final, segment, offset, size);

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
                break;
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
    err_num = 0;
    return set_c;
}

char * fakeVM_memory::getSegment(uint16_t segment) {
    if(segment + 1 > memory_size / fakeVM_memory_segment + ((memory_size % fakeVM_memory_segment) ? 1 : 0))
        return 0;
    if(!mmap)
        return 0;
    mem_map *tmp = mmap;
    printf("Get Seg %d = ", segment);
    for(uint16_t i = 0; i < segment; i++)
        tmp = tmp->next_node;
    printf("%p\n", tmp->segment_ptr);
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
    printf("Init : mem = %d\n       segment count = %d\n", memory_size, segment_c);
    for(int i = 0; i < segment_c; i++){
        tmp->segment_ptr = new char[fakeVM_memory_segment];
        if(i+1 < segment_c){
            tmp->next_node = new mem_map;
            tmp = tmp->next_node;
        }else{
            tmp->next_node = 0;
        }
    }
    printf("Seg0 : %p\n", mmap->segment_ptr);
    return 0;
}

#endif //FAKE_VM_MEMORY_H

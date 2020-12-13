//
// Created by shinsya on 2020/12/13.
//

#ifndef OS_VFS_DEBUG_MEMORIES_H
#define OS_VFS_DEBUG_MEMORIES_H

#include <stdlib.h>
#include <types.h>
#include <malloc.h>

size_t mem_get_alloc_size(void* first_prt){
    return _msize(first_prt);
}

#endif //OS_VFS_DEBUG_MEMORIES_H

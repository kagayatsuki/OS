#ifndef OS_VFS_DEBUG_FS_FILE_H
#define OS_VFS_DEBUG_FS_FILE_H

#include <types.h>
#include <calls.h>

struct _core_fs_file_struct{
    char* filename;
    uint32_t size;

    uint8_t type;
};

class _core_fs_file{
public:

};

#endif //OS_VFS_DEBUG_FS_FILE_H

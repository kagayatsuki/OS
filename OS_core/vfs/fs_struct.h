/***********************************************
 * Virtual filesystem universal struct
 * :shinsya
 ***********************************************/

#ifndef FS_STRUCT
#define FS_STRUCT

#include "../include/types.h"
#include "fs_file.h"
#include "../log/logtime.h"

struct _core_fs_info_struct{
    uint32_t file_count;
    char* root_path;
    uint16_t umask;      //default permission of new file or directory

    uint32_t block_size;        //size of per block in this filesystem
    uint32_t block_count;
};


class _core_fs{
public:
    int fs_create(char* local_path, char* path, uint32_t size);
    int fs_mount(char* local_path, uint8_t mode);    /** return inode id of root in this fs **/
    virtual int create(char* local_path, uint32_t size);
    virtual int mount(char* local_path, uint8_t mode);
    virtual int unmount();
    virtual int format();
    virtual uint32_t getFilesReset();   /** reset traversal counter **/
    virtual uint32_t getFilesOfParent(uint32_t p_inode);    /** get files or directories of parent directory by traversal way **/
    virtual _core_fs_file_struct getFileInfo(uint32_t p_inode);
    virtual uint32_t openFile(uint32_t p_inode);
    virtual char* getFileName(uint32_t p_inode);
    virtual uint32_t fileWrite(uint32_t p_inode, void* buffer, uint32_t size, uint32_t count);
    virtual uint32_t fileRead(uint32_t p_inode, void* buffer, uint32_t buffer_size, uint32_t read_size);
    virtual uint32_t fileSeek(uint32_t p_inode, long offset, int start_point);
    virtual void closeFile(uint32_t p_handle);
    virtual uint32_t getFileCount();
    virtual _core_log_date* getCreateTime();
    virtual _core_log_date* getChangeTime();
    _core_fs(const char* fs_name){memset(&fs_info, 0, sizeof(fs_info));printf("fs %s\n", fs_name);}

protected:
    struct _core_fs_info_struct fs_info;
};


#endif //FS_STRUCT

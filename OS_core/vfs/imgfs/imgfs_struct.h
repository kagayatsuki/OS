/** Read only filesystem **/

#ifndef OS_VFS_DEBUG_IMGFS_STRUCT_H
#define OS_VFS_DEBUG_IMGFS_STRUCT_H

#include <stdlib.h>
#include <types.h>
#include <calls.h>

#include <logservice.h>
#include <logtime.h>
#include <fs_struct.h>

#define _IMGFS_BLOCK_SIZE 512       //bytes

namespace imgfs_struct{
    struct imgfs_info{  /** read only **/
        _core_log_date ctime;   //create time
        _core_log_date atime;   //access time
        uint32_t block_count;
        uint32_t inode_count;
    };

    struct imgfs_uni_table{
        void* data;
        struct imgfs_uni_table* next;
    };

    struct imgfs_already_file_list{
        char type;
        FILE *local;
        uint32_t pre_id;
        uint32_t parent_id;
        char* filename;
        struct imgfs_uni_table* child_list;
        struct imgfs_already_file_list* next;
    };

    struct imgfs_inode{     /** in this read only fs. Don't think about data align **/
        uint32_t id;
        _core_log_date ctime;
        _core_log_date atime;
        uint8_t type;
        char* filename;
        uint32_t data_long;
        uint32_t data_offset;

        struct imgfs_uni_table* block_table;
    };

    struct imgfs_file{
        struct imgfs_inode* node;
        uint32_t handle;
        uint32_t cur;
    };

}

class imgfs{
public:
    void set_create_file_table(struct imgfs_struct::imgfs_uni_table* first_addr);
    int create(char* local_path, uint32_t size);
    int mount(char* local_path, uint8_t mode);    /** return inode id of root in this fs **/
    int unmount();
    int format();
    uint32_t getFilesReset();   /** reset traversal counter **/
    uint32_t getFilesOfParent(uint32_t p_inode);    /** get files or directories of parent directory by traversal way **/
    _core_fs_file_struct getFileInfo(uint32_t p_inode);
    uint32_t openFile(uint32_t p_inode);
    char* getFileName(uint32_t p_inode);
    uint32_t fileWrite(uint32_t p_inode, void* buffer, uint32_t size, uint32_t count);
    uint32_t fileRead(uint32_t p_inode, void* buffer, uint32_t buffer_size, uint32_t read_size);
    uint32_t fileSeek(uint32_t p_inode, long offset, int start_point);
    void closeFile(uint32_t p_handle);
    uint32_t getFileCount();
    _core_log_date* getCreateTime();
    _core_log_date* getChangeTime();


protected:
    struct imgfs_struct::imgfs_info info;
private:
    struct imgfs_struct::imgfs_uni_table* file_list_t;
    FILE *local;
    //void info_sync();
};

void imgfs::set_create_file_table(struct imgfs_struct::imgfs_uni_table *first_addr) {
    if(!first_addr)
        return;
    file_list_t = first_addr;
}

int imgfs_pre_dir_level(char *path){
    int len = strlen(path);
    printf("%d  ",len);
    if(len < 2)return -1;
    int ret = 0;
    bool first = false, back = false;
    for(int i = 0; i < len; i++){
        if(path[i] == '/'){
            if(first) {
                if(!back){
                    if (i > 0) {
                        if (path[i - 1] != '/')
                            ret++;
                    }else if (!i)
                        ret++;
                }else{
                    back = false;
                }
            }else{
                first = true;
            }
        }else if(path[i] == '.'){
            if(i > 0){
                if(path[i-1] == '.'){   /** back to parent dir like '/root/../var' it's level is 1 **/
                    if(i < len - 1){
                        if(path[i+1] == '/'){
                            ret--;
                            back = true;
                        }
                    }
                }
            }
        }
    }
    if(path[len-1] == '/')
        ret--;
    if(path[len-1] == '.')  /** it's illegal like '/var/..' or '/var/.' **/
        return -1;
    return ret;
}

int imgfs_pre_get_filename(char *path, uint32_t level, char *buffer){
    int _lvl = imgfs_pre_dir_level(path);
    if(level > _lvl)
        return -1;
    int len = strlen(path);
    char name_t[FILENAME_MAX] = {0};
    uint32_t now_lvl = 0;


    return strlen(name_t);
}

int imgfs::create(char *local_path, unsigned int size) {

    return 0;
}

#endif //OS_VFS_DEBUG_IMGFS_STRUCT_H

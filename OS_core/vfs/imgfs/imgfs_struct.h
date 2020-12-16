/** Read only filesystem **/

#ifndef OS_VFS_DEBUG_IMGFS_STRUCT_H
#define OS_VFS_DEBUG_IMGFS_STRUCT_H

#include <stdlib.h>
#include <new>
#include <types.h>
#include <calls.h>

#include <logservice.h>
#include <logtime.h>
#include <fs_struct.h>
#include <fs_path.h>

#define _IMGFS_BLOCK_SIZE 512       //bytes
#define _IMGFS_A_FILE_P imgfs_struct::imgfs_already_file_list*
#define _IMGFS_A_FILE imgfs_struct::imgfs_already_file_list

#define _IMGFS_NODE_BASE_SIZE (sizeof(char)+sizeof(uint32_t)*3+FS_FILENAME_MAX)
#define _IMGFS_CHILD_ADDR_SIZE sizeof(uint32_t)

#define IMGFS_SEEK_CUR 1
#define IMGFS_SEEK_SET 0
#define IMGFS_SEEK_END -1

namespace imgfs_struct{
    uint32_t imgfs_magic_code = 0xABAB1024;
    char header_string[] = " imgfs";

    struct imgfs_info{  /** read only **/
        _core_log_date ctime;   //create time
        uint32_t file_count;
        uint32_t data_length;
    };

    /** UniTable是一个通用链表，若要以cleanUniTable函数清理，则data需要是new得到的char型指针 **/
    struct imgfs_uni_table{
        void* data;
        struct imgfs_uni_table* next;
    };

    struct imgfs_already_file_list{
        char type;  // 0 - dir, 1 - file
        uint32_t data_length;
        FILE *local;
        uint32_t local_addr;
        uint32_t table_addr;
        char* filename;
        uint32_t child_count;
        struct imgfs_already_file_list* child_list;
        struct imgfs_already_file_list* next;
    };

    struct imgfs_inode{     /** in this read only fs. Don't think about data align **/
        uint8_t type;
        char* filename;
        uint32_t data_length;
        uint32_t data_address;
        uint32_t child_count;

        struct imgfs_inode* child_list;
        struct imgfs_inode* next;
    };

    struct imgfs_file{
        struct imgfs_inode* node;
        uint32_t handle;
        uint32_t cur;

        struct imgfs_file* next;
    };
}

class imgfs{
public:
    int create(char* local_path);
    int mount(char* local_path);    /** return inode id of root in this fs **/
    int unmount();
    //int format();

    void getFilesReset();   /** reset traversal counter **/
    //imgfs_struct::imgfs_uni_table* getFilesOfParent(uint32_t p_inode);    /** get files or directories of parent directory by traversal way **/
    //_core_fs_file_struct* getFileInfo(uint32_t p_inode);
    //uint32_t openFile(uint32_t p_inode);
    //char* getFileName(uint32_t p_inode);
    //uint32_t fileWrite(uint32_t p_inode, void* buffer, uint32_t size, uint32_t count);
    //uint32_t fileRead(uint32_t p_inode, void* buffer, uint32_t buffer_size, uint32_t read_size);
    //uint32_t fileSeek(uint32_t p_inode, long offset, int start_point);
    //void closeFile(uint32_t p_handle);
    uint32_t getFileCount(){return info.file_count;};
    _core_log_date getCreateTime(){return info.ctime;};
    //_core_log_date* getChangeTime();
    double getCreateProcess(){return createProcess;};

    //API:
    int create_directory_table_insert(char* path);
    int create_file_table_insert(char* local_path, char* fs_path);
    imgfs_struct::imgfs_uni_table* getAlreadyFilesOfParent(char* p_dir);
    char* SearchFile(char* filename);
    uint32_t OpenFile(char* filename);
    void CloseFile(uint32_t handle);
    uint32_t FileSeek(uint32_t handle, long offset, int start_p);
    uint32_t FileRead(uint32_t handle, void* buffer, uint32_t read_len);
    uint32_t FileTell(uint32_t handle);
    imgfs_struct::imgfs_uni_table* getFilesOfParent(char* p_dir);
    uint32_t GetTsize();
    uint32_t GetDsize();

    imgfs();
    ~imgfs();
protected:
    struct imgfs_struct::imgfs_info info;
    uint32_t opened_count;
    double createProcess;
    struct imgfs_struct::imgfs_file* FILE_LIST; /** opened file link list **/
private:
    struct imgfs_struct::imgfs_inode* ROOT;     /** ROOT **/
    struct _IMGFS_A_FILE_P already_file_list;   /** already root **/
    FILE *local;    /** local img file **/
    uint32_t already_ino;   /** next id **/

    _core_mutex read_lock;

    uint32_t getFileOfParent(uint32_t p_inode, char* filename);
    void* already_dir_get(char* path);
    void* already_file_get(_IMGFS_A_FILE_P p_struct, char* name);
    void* already_dir_append(_IMGFS_A_FILE_P p_struct, char* name);
    void* already_file_append(_IMGFS_A_FILE_P p_struct, char* name, char* local_path);
    void already_list_clear(_IMGFS_A_FILE_P p);

    uint32_t GetNodeTableLength(_IMGFS_A_FILE_P root, int single);
    uint32_t GetNodeTableLength(_IMGFS_A_FILE_P root);
    uint32_t GetDataLengthSum(_IMGFS_A_FILE_P root);
    uint32_t MakeLocalAddr(_IMGFS_A_FILE_P root_child, uint32_t* start_addr_t, uint32_t* start_addr_d);

    void* SearchForFile(char* path);
    void* SearchForOpenedFile(uint32_t handle);

};

void cleanUniTable(imgfs_struct::imgfs_uni_table* first_p);


#endif //OS_VFS_DEBUG_IMGFS_STRUCT_H

/** Read only filesystem **/

#ifndef OS_VFS_DEBUG_IMGFS_STRUCT_H
#define OS_VFS_DEBUG_IMGFS_STRUCT_H

#include <stdlib.h>
#include <new>
#include "../../include/types.h"
#include "../../include/calls.h"

#include "../../log/logservice.h"
#include "../../log/logtime.h"
#include "../fs_struct.h"
#include "../../include/fs_path.h"

/** IMGFS 内部宏定义 **/
#define _IMGFS_A_FILE_P imgfs_struct::imgfs_already_file_list*  //预备文件信息结构的指针型
#define _IMGFS_A_FILE imgfs_struct::imgfs_already_file_list

#define _IMGFS_NODE_BASE_SIZE (sizeof(char)+sizeof(uint32_t)+sizeof(uint64_t)*2+FS_FILENAME_MAX)     //每个文件元信息的基本大小相同，目录节点会额外记录子文件的元信息地址
#define _IMGFS_CHILD_ADDR_SIZE sizeof(uint64_t)            //子文件元信息地址所占空间

#define IMGFS_SEEK_CUR 1    //seek 从当前读取位置为起始点 (本文件系统不支持写操作)
#define IMGFS_SEEK_SET 0    //seek 从文件首为起始点
#define IMGFS_SEEK_END -1   //seek 从文件尾为起始点

/** IMGFS 内部结构体定义 **/
namespace imgfs_struct{
    const uint32_t imgfs_magic_code = 0xABAB1024;     //用于认证的第一块数据 4字节
    const char header_string[] = " imgfs";            //用于认证的第二块数据 6字节

    struct imgfs_info{      //文件系统信息结构体
        _core_log_date ctime;   //创建时间
        uint32_t file_count;    //文件数量
        uint32_t data_length;   //总数据长度(字节)
    };

    struct imgfs_uni_table{     //一些情况下通用的链表
        void* data;
        struct imgfs_uni_table* next;
    };

    struct imgfs_already_file_list{     //预备文件信息节点
        char type;  // 0 - dir, 1 - file 我认为这个文件系统不需要链接这种东西，因此只有目录和普通文件两种细分类型
        uint64_t data_length;
        FILE *local;    //local存放该文件对应的本地文件描述符,该描述符由 预备文件添加函数 获取并赋值于local,目录没有
        uint64_t local_addr;    //若是文件,存放create预处理时计算得到的本地数据地址偏移量
        uint64_t table_addr;    //create预处理时计算得到该文件的元信息数据偏移量
        char* filename;
        uint32_t child_count;   //若是目录,则为该目录下的子文件数量
        struct imgfs_already_file_list* child_list;     //预备子文件信息表
        struct imgfs_already_file_list* next;
    };

    struct imgfs_inode{     //文件元信息节点
        uint8_t type;   //type,data_length,child_count 与预备信息节点相同
        char* filename;
        uint64_t data_length;
        uint64_t data_address;  //数据偏移量 同预备文件信息节点中的local_addr相同
        uint32_t child_count;

        struct imgfs_inode* child_list;     //子文件信息表
        struct imgfs_inode* next;
    };

    struct imgfs_file{      //“打开文件”的抽象结构体
        struct imgfs_inode* node;   //该文件对应的元信息
        uint32_t handle;    //打开文件返回的描述符
        uint64_t cur;     //本文件抽象的数据读取指针位置

        struct imgfs_file* next;
    };
}

class imgfs{
public:
    int create(char* local_path);   //在给定的地址创建文件,并在其中由预备文件树创建文件系统,成功返回非负数
    int mount(char* local_path);    //从本地文件挂载文件系统,正确挂载返回非负数
    int unmount();  //卸载已挂载的文件系统


    /** API: **/
    uint32_t getFileCount(){return info.file_count;};   //获取文件系统中文件总数
    _core_log_date getCreateTime(){return info.ctime;}; //获取文件系统创建时间,返回值为log_date结构体

    int create_directory_table_insert(char* path);  //向预备文件树添加目录,参数为本文件系统内的路径
    int create_file_table_insert(char* local_path, char* fs_path);  //向预备文件树添加文件,参数为本地文件路径和本文件系统内路径
    imgfs_struct::imgfs_uni_table* getAlreadyFilesOfParent(char* p_dir);    //调试期函数,目前遗留不做改动或删除,从预备文件树获取某目录下子文件表,该函数返回的通用链表中,data为char数组指针

    char* SearchFile(char* filename, int type);   //实现在之后的版本补充,其用法将与windows系统api的寻找文件类似
    uint32_t OpenFile(char* filename);  //创建指定文件的抽象,返回其描述符,用户层读取数据需通过该描述符
    void CloseFile(uint32_t handle);    //销毁某文件的抽象
    uint32_t FileSeek(uint32_t handle, uint64_t offset, int start_p);   //改变描述符对应的文件的数据读取位置,
    uint32_t FileRead(uint32_t handle, void* buffer, uint32_t read_len);    //从描述符对应的文件中读取给定长度的数据到提供的缓冲区
    uint64_t FileTell(uint32_t handle);     //获取描述符对应文件的当前数据读取位置
    imgfs_struct::imgfs_uni_table* getFilesOfParent(char* p_dir);   //获取给定路径的目录下的所有文件,返回的通用链表的data存放文件名
    uint32_t GetTsize();    //调试期函数,计算当前预备文件表的结构体大小
    uint64_t GetDsize();    //调试期函数,计算当前预备文件表中的所有文件数据大小

    uint64_t CopyImage(char* local_path);   //将本文件系统的内容复制到本地的目录中. 注:该参数提供的路径需要确保其目录不存在，目录由程序创建

    imgfs();
    ~imgfs();
protected:
    struct imgfs_struct::imgfs_info info;   //文件系统信息
    uint32_t opened_count;      //已打开的文件数量
    struct imgfs_struct::imgfs_file* FILE_LIST;     //“OpenFile”文件抽象信息表
private:
    struct imgfs_struct::imgfs_inode* ROOT;     //该文件系统根节点
    struct _IMGFS_A_FILE_P already_file_list;   //预备文件根节点
    FILE *local;    //本地镜像文件描述符
    uint32_t already_ino;   //下一次创建文件抽象时分配的描述符

    _core_mutex read_lock;      //数据读取互斥锁

    double proc_rate;     //数据百分比例
    double write_proc;    //写入进度

    /** 内部处理函数 **/

    void* already_dir_get(char* path);      //从给定路径获取到最末级的文件节点
    void* already_file_get(_IMGFS_A_FILE_P p_struct, char* name);   //从指定的父节点通过文件名获得文件节点
    void* already_dir_append(_IMGFS_A_FILE_P p_struct, char* name); //向预备文件树追加目录
    void* already_file_append(_IMGFS_A_FILE_P p_struct, char* name, char* local_path);  //向预备文件树追加文件
    void already_list_clear(_IMGFS_A_FILE_P p); //销毁整个预备文件树

    uint64_t imgfs_write_data(FILE* file_t, _IMGFS_A_FILE_P root);    //创建过程中按预计算好的地址写入预备区数据

    uint32_t GetNodeTableLength(_IMGFS_A_FILE_P root, int single);  //获取树的某层结构体大小,single指定单层还是向下多层计算,root提供的并非父节点,而是父节点拥有的第一个子节点
    uint32_t GetNodeTableLength(_IMGFS_A_FILE_P root);  //前者的 默认多层 重调
    uint64_t GetDataLengthSum(_IMGFS_A_FILE_P root);    //获取某层树下子节点的数据长度
    uint32_t MakeLocalAddr(_IMGFS_A_FILE_P root_child, uint64_t* start_addr_t, uint64_t* start_addr_d); //create时调用的预处理函数,预先计算数据偏移

    uint32_t CreateLocalFolders(char* local_root, imgfs_struct::imgfs_inode* root); //CopyImage的辅助函数,用于预先创建本地与本文件系统相同的目录结构
    uint64_t WriteLocalFiles(char* local_root, imgfs_struct::imgfs_inode* root);    //CopyImage的主要实现,复制本文件系统内的文件数据到创建好的目录结构中

    void* SearchForFile(char* path);    //内部处理函数,通过路径字符串解析得节点指针
    void* SearchForOpenedFile(uint32_t handle);     //内部处理函数,用以配对文件描述符与其对应的结构体

};

void cleanUniTable(imgfs_struct::imgfs_uni_table* first_p);     //销毁一个通用链表,该函数针对的是存储字符串的通用链表
void cleanUniTableC(imgfs_struct::imgfs_uni_table* first_p);    //针对counter

#include "imgfs.cpp"

#endif //OS_VFS_IMGFS_STRUCT_H

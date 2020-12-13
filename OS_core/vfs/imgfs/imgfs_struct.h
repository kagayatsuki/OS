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
        char type;  // 0 - dir, 1 - file
        FILE *local;
        uint32_t pre_id;
        uint32_t parent_id;
        char* filename;
        struct _core_log_date* ctime;
        struct imgfs_already_file_list* child_list;
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

    struct imgfs_local_struct{
        uint32_t id;
        uint32_t p_id;
        char type;
        struct _core_log_date* ctime;
        struct imgfs_uni_table* child_list;
    };
}

class imgfs{
public:
    int create(char* local_path);
    int mount(char* local_path, uint8_t mode);    /** return inode id of root in this fs **/
    int unmount();
    //int format();

    void getFilesReset();   /** reset traversal counter **/
    //imgfs_struct::imgfs_uni_table* getFilesOfParent(uint32_t p_inode);    /** get files or directories of parent directory by traversal way **/
    _core_fs_file_struct* getFileInfo(uint32_t p_inode);
    //uint32_t openFile(uint32_t p_inode);
    //char* getFileName(uint32_t p_inode);
    //uint32_t fileWrite(uint32_t p_inode, void* buffer, uint32_t size, uint32_t count);
    //uint32_t fileRead(uint32_t p_inode, void* buffer, uint32_t buffer_size, uint32_t read_size);
    //uint32_t fileSeek(uint32_t p_inode, long offset, int start_point);
    //void closeFile(uint32_t p_handle);
    uint32_t getFileCount();
    _core_log_date* getCreateTime();
    _core_log_date* getChangeTime();
    double getCreateProcess();

    //API:
    int create_directory_table_insert(char* path);
    int create_file_table_insert(char* local_path, char* fs_path);
    char* SearchFile(char* filename);
    uint32_t OpenFile(char* filename);
    void CloseFile(uint32_t handle);
    uint32_t FileSeek(uint32_t handle, uint32_t offset, uint32_t start_p);
    uint32_t FileRead(uint32_t handle, void* buffer, uint32_t read_len);
    imgfs_struct::imgfs_uni_table* getFilesOfParent(char* p_dir);

    imgfs();
    ~imgfs();
protected:
    struct imgfs_struct::imgfs_info info;
    double createProcess;
private:
    struct _IMGFS_A_FILE_P already_file_list;   /** root **/
    FILE *local;    /** local img file **/
    uint32_t already_ino;   /** next id **/

    uint32_t getFileOfParent(uint32_t p_inode, char* filename);

    void* already_file_get(_IMGFS_A_FILE_P p_struct, char* name);
    void* already_dir_append(_IMGFS_A_FILE_P p_struct, char* name);
    void* already_file_append(_IMGFS_A_FILE_P p_struct, char* name, char* local_path);
    void already_list_clear(_IMGFS_A_FILE_P p);

};

imgfs::imgfs() {
    already_file_list = new _IMGFS_A_FILE();
    printf("root: %p\n", already_file_list);
    createProcess = 0.0;
    already_ino = 1;
    info.inode_count = 0;
}

void imgfs::already_list_clear(_IMGFS_A_FILE_P p){
    if(p->next){
        already_list_clear(p->next);
    }
    if(p->child_list){
        already_list_clear(p->child_list);
    }

    if(p->ctime)free(p->ctime);
    if(p->filename)delete [] (char*)p->filename;
    if(p->local)fclose(p->local);
    delete p;
}

imgfs::~imgfs() {
    _core_service_log_print("[imgfs] cleaning.");
    already_list_clear(already_file_list);
    if(local)
        fclose(local);
}

void* imgfs::already_file_get(_IMGFS_A_FILE_P p_struct, char* name){
    if(!p_struct || !name)
        return 0;
    _IMGFS_A_FILE_P op_c = p_struct->child_list;
    while(op_c){
        //printf("[debug string cmp] |=%s=|=%s=|\n", op_c->filename, name);
        if(fs_string_cmp(op_c->filename, name) == 0){
            return op_c;
        }
        op_c = op_c->next;
    }
    return nullptr;
}

void* imgfs::already_dir_append(_IMGFS_A_FILE_P p_struct, char* name){
    _IMGFS_A_FILE_P tmp = (_IMGFS_A_FILE_P)already_file_get(p_struct, name);
    _IMGFS_A_FILE_P table_t;
    if(tmp)
        return 0;  /** already exists **/
    if(!p_struct || !name)
        return 0;
    if(p_struct->type)
        return 0;  /** parent is not a dir **/

    printf("add to parent - %p\n", p_struct);

    table_t = p_struct->child_list;
    if(table_t){
        while(table_t->next)
            table_t = table_t->next;
        table_t->next = new _IMGFS_A_FILE();
        table_t=table_t->next;
    }else{
        p_struct->child_list = new _IMGFS_A_FILE();
        table_t = p_struct->child_list;
    }
    table_t->parent_id = p_struct->pre_id;
    table_t->filename = new char[FS_FILENAME_MAX+1]();
    memcpy(table_t->filename, name, strlen(name));
    table_t->filename[strlen(name)] = '\0';
    table_t->pre_id = already_ino;
    already_ino++;
    table_t->ctime = _core_log_date_get();
    _core_service_log_print("[imgfs] new directory named:");
    _core_service_log_print(name);
    return table_t;
}

void* imgfs::already_file_append(_IMGFS_A_FILE_P p_struct, char *name, char *local_path) {
    _IMGFS_A_FILE_P tmp = (_IMGFS_A_FILE_P)already_file_get(p_struct, name);
    _IMGFS_A_FILE_P table_t;
    if(tmp || !p_struct || !name || !local_path || p_struct->type)
        return 0;

    _core_service_log_print("[imgfs] try to create file:");
    _core_service_log_print(name);
    _core_service_log_print("[imgfs] new file from");
    _core_service_log_print(local_path);

    FILE* local_t = fopen(local_path, "r");
    if(!local_t) {
        _core_service_log_print("[imgfs] append file failed. can't open file.");
        return 0;
    }

    _core_service_log_print("processing.");

    table_t=p_struct->child_list;
    /**debug**/
    //printf("p_ptr:%p parent_id: %d\ntable_t: %p\n", p_struct, p_struct->parent_id, table_t);

    if(table_t){
        while(table_t->next){
            table_t=table_t->next;
            putchar('-');
        }
        try{
            table_t->next = new _IMGFS_A_FILE();
        } catch (std::bad_alloc) {
            _core_service_log_print("[imgfs] bad alloc.");
            _core_service_log_print("[imgfs] processing failed.");
            return 0;
        }
        printf("child ptr: %p\n", table_t->next);
        table_t = table_t->next;
    }else{
        try{
            p_struct->child_list = new _IMGFS_A_FILE();
        }catch (std::bad_alloc) {
            _core_service_log_print("[imgfs] bad alloc.");
            _core_service_log_print("[imgfs] processing failed.");
            return 0;
        }
        printf("child ptr: %p\n", p_struct->child_list);
        table_t=p_struct->child_list;
    }

    _core_service_log_print("processing done. making.");

    table_t->parent_id = p_struct->pre_id;
    table_t->filename = new char[FS_FILENAME_MAX+1]();
    memcpy(table_t->filename, name, strlen(name));
    table_t->filename[strlen(name)] = '\0';
    /** debug **/
    //printf("%d |%s|%s|\n", strlen(name), table_t->filename, name);
    table_t->local = local_t;
    table_t->type = 1;
    table_t->pre_id = already_ino;
    already_ino++;
    table_t->ctime = _core_log_date_get();

    _core_service_log_print("making done.");
    return table_t;
}

int imgfs::create_file_table_insert(char* local_path, char* fs_path) {
    if(!local_path || !fs_path)
        return -1;
    if(fs_path_check_illegal(fs_path))
        return -5;
    _IMGFS_A_FILE_P p_struct = already_file_list;
    char* path_t = 0;
    int path_len = fs_path_format(fs_path, &path_t);
    if(!path_t || path_len < 2)
        return -1;
    path_level lvl_t = fs_get_path_level(path_t);
    char* name_t;
    for(int i = 0; i < lvl_t; i++){
        if(p_struct){
            name_t = fs_get_path_name(path_t, i);
            /** debug **/
            //printf("lvl %d of %s is %s\n", i, path_t, name_t);
            p_struct = (_IMGFS_A_FILE_P)already_file_get(p_struct, name_t);
            if(name_t)free(name_t);

        }
    }
    if(!p_struct){
        _core_service_log_print("[imgfs] can't append file, the parent directory is not exist.");
        free(path_t);
        return -2;
    }
    name_t = fs_get_path_name(path_t, lvl_t);
    _IMGFS_A_FILE_P this_file = (_IMGFS_A_FILE_P)already_file_get(p_struct, name_t);
    if(this_file){
        _core_service_log_print("[imgfs] can't append file, it was already exist.");
        if(name_t)
            free(name_t);
        free(path_t);
        return -3;
    }
    this_file = (_IMGFS_A_FILE_P)already_file_append(p_struct, name_t, local_path);
    if(!this_file){
        _core_service_log_print("[imgfs] can't append file, not reason.");
        if(name_t)
            free(name_t);
        free(path_t);
        return -4;
    }
    _core_service_log_print("[imgfs] touch file:");
    _core_service_log_print(path_t);
    if(name_t)
        free(name_t);
    free(path_t);
    return this_file->pre_id;
}

int imgfs::create_directory_table_insert(char* path){
    _IMGFS_A_FILE_P parent_t = already_file_list;
    if(fs_path_check_illegal(path))
        return -5;
    if(!path)
        return -1;
    char* path_t = 0;
    int path_len = fs_path_format(path, &path_t);
    if(path_len < 2 || !path_t)
        return -1;
    path_level lvl_t = fs_get_path_level(path_t);
    char* name_t;
    for(int i = 0; i < lvl_t; i++){
        name_t = fs_get_path_name(path_t, i);
        parent_t = (_IMGFS_A_FILE_P)already_file_get(parent_t, name_t);
        if(name_t)
            free(name_t);
    }
    if(!parent_t){
        _core_service_log_print("[imgfs] can't append directory, the parent was already exist.");
        free(path_t);
        return -2;
    }
    if(parent_t->type){
        _core_service_log_print("[imgfs] can't append directory, the parent dir not exist.");
        free(path_t);
        return -3;
    }
    name_t = fs_get_path_name(path_t, lvl_t);
    _IMGFS_A_FILE_P this_file = (_IMGFS_A_FILE_P)already_file_get(parent_t, name_t);
    if(this_file){
        _core_service_log_print("[imgfs] can't append directory, it was already exist.");
        if(name_t)
            free(name_t);
        free(path_t);
        return -3;
    }
    this_file = (_IMGFS_A_FILE_P)already_dir_append(parent_t, name_t);
    if(!this_file){
        _core_service_log_print("[imgfs] can't append directory.");
        if(name_t)
            free(name_t);
        free(path_t);
        return -3;
    }
    _core_service_log_print("[imgfs] mkdir:");
    _core_service_log_print(path_t);
    if(name_t)
        free(name_t);
    free(path_t);
    return this_file->pre_id;
}

void cleanUniTable(imgfs_struct::imgfs_uni_table* first_p){
    if(first_p->next)
        cleanUniTable(first_p->next);
    if(first_p->data)
        delete [] (char*)(first_p->data);
    delete first_p;
}

imgfs_struct::imgfs_uni_table* makeUniTable(_IMGFS_A_FILE_P dir_p){
    imgfs_struct::imgfs_uni_table* first = new imgfs_struct::imgfs_uni_table();
    imgfs_struct::imgfs_uni_table* op = first;

    //printf("making Uni table of %p\n", dir_p);
    dir_p = dir_p->child_list;
    bool undo = true;
    while(dir_p){
        if(dir_p->filename){
            undo = false;
            op->data = new char[FS_FILENAME_MAX]();
            memcpy(op->data, dir_p->filename, FS_FILENAME_MAX);
            op->next = new imgfs_struct::imgfs_uni_table();
            op = op->next;
        }
        dir_p = dir_p->next;
    }
    if(undo){
        if(first->data)delete[] (char*)first->data;
        delete first;
        return 0;
    }
    return first;
}

imgfs_struct::imgfs_uni_table* imgfs::getFilesOfParent(char* p_dir){
    if(!p_dir)
        return 0;
    char* format_path = 0;
    int len = fs_path_format(p_dir, &format_path);
    _IMGFS_A_FILE_P root = already_file_list;
    if(!format_path)
        return 0;

    _core_service_log_print("cat list about:");
    _core_service_log_print(format_path);

    if(len < 1){
        free(format_path);
        return 0;
    }
    if(len == 1){
        if(format_path[0] == '/'){
            free(format_path);
            return makeUniTable(root);
        }
        else{
            free(format_path);
            return 0;
        }
    }
    int lvl = fs_get_path_level(format_path);
    char* name_t;
    for(int i = 0; i <= lvl; i++){
        name_t = fs_get_path_name(format_path, i);
        root = (_IMGFS_A_FILE_P)already_file_get(root, name_t);
        if(name_t)
            free(name_t);
    }
    free(format_path);
    return makeUniTable(root);
}

int imgfs::create(char *local_path) {

    return 0;
}

#endif //OS_VFS_DEBUG_IMGFS_STRUCT_H

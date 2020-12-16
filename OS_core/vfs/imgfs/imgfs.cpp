//
// Created by shinsya on 2020/12/15.
// 对某些不准确的英文注释换成了中文，哎是我懒得查单词了

#include "imgfs_struct.h"

imgfs::imgfs() {
    already_file_list = new _IMGFS_A_FILE();
    already_file_list->filename = new char[FS_FILENAME_MAX+1]();
    already_file_list->filename[0] = '/';
    createProcess = 0.0;
    already_ino = 1;
    memset(&info, 0, sizeof(imgfs_struct::imgfs_info));
    local = 0;
    ROOT = 0;
    FILE_LIST = 0;
    opened_count = 0;
    _core_mutex_create(read_lock);
}

/** 清除预备区节点数据 **/
void imgfs::already_list_clear(_IMGFS_A_FILE_P p){
    if(p->next){
        _core_service_log_print("[imgfs] clean next node");
        already_list_clear(p->next);
    }
    if(p->child_list){
        _core_service_log_print("[imgfs] clean child node");
        already_list_clear(p->child_list);
    }

    _core_service_log_print("[imgfs] clean this node");
    if(p->filename)delete [] (char*)p->filename;
    if(p->local)fclose(p->local);
    delete p;
}

imgfs::~imgfs() {
    _core_service_log_print("[imgfs] cleaning.");
    already_list_clear(already_file_list);
    if(local)
        fclose(local);
    _core_mutex_destroy(read_lock);
}

/** 从预备区域寻找文件， 参数提供父节点和文件名 **/
void* imgfs::already_file_get(_IMGFS_A_FILE_P p_struct, char* name){
    if(!p_struct || !name)
        return 0;
    _IMGFS_A_FILE_P op_c = p_struct->child_list;
    while(op_c){
        if(fs_string_cmp(op_c->filename, name) == 0){
            return op_c;
        }
        op_c = op_c->next;
    }
    return nullptr;
}

/** 从预备区获取某一级目录 **/
void* imgfs::already_dir_get(char* path){
    char* path_t = 0;
    int ret = fs_path_format(path, &path_t);
    if(ret < 2)
        return 0;
    _IMGFS_A_FILE_P tmp_p = already_file_list;
    path_level lvl = fs_get_path_level(path_t);
    if(lvl < 0)
        return 0;
    char* name_t;
    for(int i = 0; i < lvl; i++){
        if(tmp_p){
            name_t = fs_get_path_name(path_t, i);
            tmp_p = (_IMGFS_A_FILE_P)already_file_get(tmp_p, name_t);
            if(name_t)
                free(name_t);
        }
    }
    return tmp_p;
}

/** 向预备区追加目录, 参数提供父节点和目录文件名 **/
void* imgfs::already_dir_append(_IMGFS_A_FILE_P p_struct, char* name){
    _IMGFS_A_FILE_P tmp = (_IMGFS_A_FILE_P)already_file_get(p_struct, name);
    _IMGFS_A_FILE_P table_t;
    if(tmp)
        return 0;  /** already exists **/
    if(!p_struct || !name)
        return 0;
    if(p_struct->type)
        return 0;  /** parent is not a dir **/

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
    table_t->filename = new char[FS_FILENAME_MAX+1]();
    memset(table_t->filename, 0, FS_FILENAME_MAX+1);
    memcpy(table_t->filename, name, strlen(name));
    table_t->filename[strlen(name)] = '\0';
    already_ino++;
    p_struct->child_count++;
    info.file_count++;

    _core_service_log_print("[imgfs] new directory named:");
    _core_service_log_print(name);
    return table_t;
}

/** 同上，追加文件，需要提供对应的本地文件 **/
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

    if(table_t){
        while(table_t->next){
            table_t=table_t->next;
            putchar('-');
        }

        table_t->next = new _IMGFS_A_FILE();
        printf("child ptr: %p\n", table_t->next);
        table_t = table_t->next;
    }else{
        p_struct->child_list = new _IMGFS_A_FILE();
        printf("child ptr: %p\n", p_struct->child_list);
        table_t=p_struct->child_list;
    }

    _core_service_log_print("processing done. making.");

    table_t->filename = new char[FS_FILENAME_MAX+1]();
    memset(table_t->filename, 0, FS_FILENAME_MAX+1);
    memcpy(table_t->filename, name, strlen(name));
    table_t->filename[strlen(name)] = '\0';
    table_t->local = local_t;
    table_t->type = 1;
    already_ino++;
    p_struct->child_count++;
    fseek(local_t, 0, SEEK_END);
    table_t->data_length = ftell(local_t);
    info.file_count++;
    info.data_length+=table_t->data_length;

    _core_service_log_print("making done.");
    return table_t;
}

/** API: 向预备区追加文件的封装 **/
int imgfs::create_file_table_insert(char* local_path, char* fs_path) {
    if(!local_path || !fs_path)
        return -1;
    if(fs_path_check_illegal(fs_path))
        return -5;
    _IMGFS_A_FILE_P p_struct;
    char* path_t = 0;
    int path_len = fs_path_format(fs_path, &path_t);
    if(!path_t || path_len < 2)
        return -1;

    char* name_t;
    path_level lvl_t = fs_get_path_level(path_t);

    p_struct = (_IMGFS_A_FILE_P)already_dir_get(path_t);

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
    return already_ino-1;
}

/** API: 向预备区追加目录的封装 **/
int imgfs::create_directory_table_insert(char* path){
    _IMGFS_A_FILE_P parent_t;
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

    parent_t = (_IMGFS_A_FILE_P)already_dir_get(path_t);

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
    return already_ino-1;
}

/** 非类函数，用于清理一个UniTable **/
void cleanUniTable(imgfs_struct::imgfs_uni_table* first_p){
    if(!first_p)
        return;
    if(first_p->next)
        cleanUniTable(first_p->next);
    if(first_p->data)
        delete [] (char*)(first_p->data);
    delete first_p;
}

/** 生成一个保存了某一级目录下子成员的UniTable **/
imgfs_struct::imgfs_uni_table* makeUniTable(_IMGFS_A_FILE_P dir_p){
    imgfs_struct::imgfs_uni_table* first = new imgfs_struct::imgfs_uni_table();
    imgfs_struct::imgfs_uni_table* op = first;

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

/** API: 从某级目录获得子成员表的封装 **/
imgfs_struct::imgfs_uni_table* imgfs::getAlreadyFilesOfParent(char* p_dir){
    if(!p_dir)
        return 0;
    char* format_path = 0;
    int len = fs_path_format(p_dir, &format_path);
    _IMGFS_A_FILE_P root = already_file_list;
    if(!format_path)
        return 0;

    _core_service_log_print("get list about:");
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

/** 获取整个预备区的结构体理论大小 **/
uint32_t imgfs::GetNodeTableLength(_IMGFS_A_FILE_P root, int single){
    uint32_t this_fl = 0;
    _IMGFS_A_FILE_P root_t = root;
    while(root_t){
        if(root_t->type){
            this_fl+= _IMGFS_NODE_BASE_SIZE;
        }else{
            this_fl+= _IMGFS_NODE_BASE_SIZE + _IMGFS_CHILD_ADDR_SIZE * root_t->child_count;
            if(!single)
                this_fl+= GetNodeTableLength(root_t->child_list);
        }
        root_t = root_t->next;
    }
    return this_fl;
}

uint32_t imgfs::GetNodeTableLength(_IMGFS_A_FILE_P root){
    return GetNodeTableLength(root, 0);
}

/** 获取预备区所有数据总和长度 **/
uint32_t imgfs::GetDataLengthSum(_IMGFS_A_FILE_P root){
    uint32_t this_fl = 0;
    _IMGFS_A_FILE_P root_t = root;
    while(root_t){
        if(root_t->type){
            this_fl+=root_t->data_length;
        }else{
            this_fl+=GetDataLengthSum(root_t->child_list);
        }
        root_t = root_t->next;
    }
    return this_fl;
}

/** 提前计算各个节点的地址,递归式 **/
uint32_t imgfs::MakeLocalAddr(_IMGFS_A_FILE_P root_child, uint32_t* start_addr_t, uint32_t* start_addr_d){
    uint32_t* proc_addr_t = start_addr_t, *proc_addr_d = start_addr_d;
    _IMGFS_A_FILE_P prt_p = root_child;
    _IMGFS_A_FILE_P prt_p_t = prt_p;
    while(prt_p_t){
        prt_p_t->table_addr = *proc_addr_t;
        *proc_addr_t+= _IMGFS_NODE_BASE_SIZE;
        if(prt_p_t->type){
            prt_p_t->local_addr = *proc_addr_d;
            *proc_addr_d+=prt_p_t->data_length;
        }else{
            *proc_addr_t+= _IMGFS_CHILD_ADDR_SIZE * prt_p_t->child_count;
        }
        prt_p_t = prt_p_t->next;
    }
    prt_p_t = prt_p;
    while(prt_p_t){
        if(!prt_p_t->type && prt_p_t->child_list && prt_p_t->child_count){
            MakeLocalAddr(prt_p_t->child_list, proc_addr_t, proc_addr_d);
        }
        prt_p_t = prt_p_t->next;
    }
    return *proc_addr_t;
}

/** API: 获取预备区结构体大小 **/
uint32_t imgfs::GetTsize(){
    return GetNodeTableLength(already_file_list);
}

/** API: 获取预备区数据长度总和 **/
uint32_t imgfs::GetDsize(){
    return GetDataLengthSum(already_file_list);
}

/** 对文件填充指定数量的'0' **/
uint32_t imgfs_write_blank_table(FILE* file, uint32_t size){
    if(!file)
        return 0;
    char tmp[512];
    memset(tmp,0,512);
    uint32_t i = 0, block_c = size / 512;
    uint32_t mod_c = size % 512;

    for(;i<block_c;i++){
        fwrite(tmp,512,1,file);
    }
    if(mod_c)
        fwrite(tmp,mod_c,1,file);

    return i * 512 + mod_c;
}

/** 该函数不得被直接调用。用于创建过程中的元数据写入 **/
void imgfs_meta_write(FILE* file, _IMGFS_A_FILE_P meta){
    fwrite(&meta->type, sizeof(char), 1, file);
    fwrite(meta->filename, FS_FILENAME_MAX, 1, file);
    fwrite(&meta->data_length, sizeof(uint32_t), 1, file);
    fwrite(&meta->local_addr, sizeof(uint32_t), 1, file);
    fwrite(&meta->child_count, sizeof(uint32_t), 1, file);
}

/** 创建过程中按预计算好的地址写入预备区数据 **/
uint32_t imgfs_write_data(FILE* file_t, _IMGFS_A_FILE_P root){
    uint32_t writen_d = 0;
    _IMGFS_A_FILE_P root_t = root;
    char buff[512];
    while(root_t){
        if(root_t->type){
            fseek(root_t->local, 0, SEEK_SET);
            fseek(file_t, root_t->local_addr-ftell(file_t), SEEK_CUR);
            for(int t = 0; t < root_t->data_length / 512; t++){
                memset(buff, 0, 512);
                fread(buff, 512, 1, root_t->local);
                fwrite(buff, 512, 1, file_t);
            }
            memset(buff, 0, 512);
            if(root_t->data_length % 512){
                fread(buff, root_t->data_length % 512, 1, root_t->local);
                fwrite(buff, root_t->data_length % 512, 1, file_t);
            }
            writen_d+= root_t->data_length;
        }else{
            writen_d+= imgfs_write_data(file_t, root_t->child_list);
        }
        fseek(file_t, root_t->table_addr-ftell(file_t), SEEK_CUR);
        imgfs_meta_write(file_t, root_t);
        _IMGFS_A_FILE_P tmp = root_t->child_list;
        for(int i = 0; i < root_t->child_count; i++){
            if(tmp) {
                fwrite(&tmp->table_addr, sizeof(uint32_t), 1, file_t);
                tmp = tmp->next;
            }
        }
        root_t = root_t->next;
    }
    return writen_d;
}

/** 从预备区创建镜像 **/
int imgfs::create(char *local_path) {
    _core_service_log_print("imgfs local image creating.");
    FILE* local_t = fopen(local_path, "w+");

    if(!local_t) {
        _core_service_log_print("[imgfs] can't create file when creating local image.");
        return -1;
    }

    uint32_t table_addr;
    uint32_t tSize = GetTsize(), dSize = GetDsize();
    if(dSize != info.data_length){
        _core_service_log_print("[imgfs] data information not matched. process abort.");
        fclose(local_t);
        return -2;
    }

    info.data_length = dSize;
    _core_log_date* tmp = _core_log_date_get();
    memcpy(&info.ctime, tmp, sizeof(_core_log_date));
    free(tmp);

    fseek(local_t, 0, SEEK_SET);
    /** write fs flag **/
    fwrite(&imgfs_struct::imgfs_magic_code, sizeof(uint32_t), 1, local_t);
    fwrite(imgfs_struct::header_string, strlen(imgfs_struct::header_string), 1, local_t);
    /** write basic information **/
    fwrite(&info, sizeof(info), 1, local_t);
    /** record this address, it's node table start address **/
    table_addr = ftell(local_t);

    _core_service_log_print("[imgfs] writing meta-data table.");
    imgfs_write_blank_table(local_t, tSize);
    fseek(local_t, table_addr, SEEK_SET);

    uint32_t d_start = tSize + table_addr, t_start = table_addr;
    uint32_t cRet = MakeLocalAddr(already_file_list, &t_start, &d_start);
    if(cRet != t_start || cRet != table_addr + tSize) {
        _core_service_log_print("[imgfs] there are some problems. Maybe it's memory exception.");
        fclose(local_t);
        return -3;
    }

    cRet = imgfs_write_data(local_t, already_file_list);

    return cRet;
}

/** 生成或追加无符号整数UniTable **/
imgfs_struct::imgfs_uni_table* imgfs_counter_add(imgfs_struct::imgfs_uni_table* ptr, uint32_t data){
    imgfs_struct::imgfs_uni_table *tmp = ptr, *ret = ptr;
    if(!tmp){
        tmp = new imgfs_struct::imgfs_uni_table();
        ret = tmp;
    }else{
        while(tmp->next){
            tmp = tmp->next;
        }
        tmp->next = new imgfs_struct::imgfs_uni_table();
        tmp = tmp->next;
    }
    tmp->data = new uint32_t();
    *(uint32_t*)tmp->data = data;
    return ret;
}

/** 装载过程中读取元数据的函数。不得直接调用 **/
void imgfs_mount_read_meta(imgfs_struct::imgfs_inode* inode_s, FILE* file_t){
    inode_s->filename = new char[FS_FILENAME_MAX+1]();
    fread(&inode_s->type, sizeof(char), 1, file_t);
    fread(inode_s->filename, FS_FILENAME_MAX, 1, file_t);
    fread(&inode_s->data_length, sizeof(uint32_t), 1, file_t);
    fread(&inode_s->data_address, sizeof(uint32_t), 1, file_t);
    fread(&inode_s->child_count, sizeof(uint32_t), 1, file_t);
}

/** 递归式从元数据挂载文件系统文件记录表 **/
int imgfs_mount_root(imgfs_struct::imgfs_inode* root, FILE* file_t){
    uint32_t read_t = 0;
    imgfs_struct::imgfs_uni_table *counter = 0, *op_counter;

    /** make counter **/
    for (int i = 0; i < root->child_count; i++){
        fread(&read_t, sizeof(uint32_t), 1, file_t);
        counter = imgfs_counter_add(counter, read_t);
    }
    op_counter = counter;
    imgfs_struct::imgfs_inode* op_child;
    for (int t = 0; t < root->child_count; t++){
        /** read child node **/
        fseek(file_t, *(uint32_t*)op_counter->data - ftell(file_t), SEEK_CUR);
        if(!root->child_list){
            op_child = new imgfs_struct::imgfs_inode();
            root->child_list = op_child;
        }else {
            op_child->next = new imgfs_struct::imgfs_inode();
            op_child = op_child->next;
        }
        imgfs_mount_read_meta(op_child, file_t);
        op_counter = op_counter->next;
    }

    /** read child directory **/
    op_child = root->child_list;
    op_counter = counter;
    while (op_child){
        if(!op_child->type){
            fseek(file_t, (*(uint32_t*)op_counter->data + _IMGFS_NODE_BASE_SIZE)-ftell(file_t), SEEK_CUR);
            imgfs_mount_root(op_child, file_t);
        }
        op_child = op_child->next;
        op_counter = op_counter->next;
    }
    cleanUniTable(counter);
    return 0;
}

/** 挂载镜像 **/
int imgfs::mount(char* local_path){
    _core_service_log_print("[imgfs] mounting image from");
    _core_service_log_print(local_path);
    /** basic safe check **/
    if(local || !local_path)
        return -1;
    FILE* local_t = fopen(local_path, "r+");
    if(!local_t)
        return -2;
    /** flags check **/
    char sFlag[10] = {0};
    fseek(local_t, 0, SEEK_SET);
    fread(sFlag, 10, 1, local_t);
    if(fs_string_cmp(&sFlag[4], imgfs_struct::header_string) || (*(uint32_t*)sFlag != 0xABAB1024)){
        _core_service_log_print("[imgfs] mounted failed.");
        _core_service_log_print("[imgfs] The file may be not a imgfs image file.");
        fclose(local_t);
        return -3;
    }
    /** read info **/
    fread(&info, sizeof(info), 1, local_t);
    /** init root **/
    ROOT = new imgfs_struct::imgfs_inode();
    imgfs_mount_read_meta(ROOT, local_t);

    local = local_t;
    return imgfs_mount_root(ROOT, local_t);
}

/** 释放文件系统文件节点树 **/
void imgfs_inode_tree_free(imgfs_struct::imgfs_inode* node){
    if(!node)
        return;
    if(node->next){
        imgfs_inode_tree_free(node->next);
    }
    if(node->child_list){
        imgfs_inode_tree_free(node->child_list);
    }
    if(node->filename)
        delete [] node->filename;
    delete node;
    return;
}

/** 卸载镜像 **/
int imgfs::unmount(){
    if(!local)
        return -1;
    fclose(local);
    imgfs_inode_tree_free(ROOT);
    ROOT = 0;
    local = 0;
    return 0;
}

/** 从文件节点书寻找指定路径的文件或目录 **/
void* imgfs::SearchForFile(char* path){
    _core_service_log_print("[imgfs] Search for");
    _core_service_log_print(path);
    if(!ROOT){
        _core_service_log_print("[imgfs] image was not mounted.");
        return 0;
    }
    char* path_t = 0;
    int len_t = fs_path_format(path, &path_t);
    if(!path_t){
        _core_service_log_print("[imgfs] can not format path string.");
        return 0;
    }
    if(len_t < 1){
        _core_service_log_print("[imgfs] illegal path.");
        _core_service_log_print(path_t);
        free(path_t);
        return 0;
    }
    if(len_t == 1){
        if(path_t[0] == '/'){
            free(path_t);
            return ROOT;
        }
        _core_service_log_print("[imgfs] illegal path.");
        _core_service_log_print(path_t);
    }

    path_level path_lvl = fs_get_path_level(path_t);
    imgfs_struct::imgfs_inode *root_tmp = 0, *op_tmp = ROOT->child_list;
    char *name_t;
    for(int i = 0; i < path_lvl; i++){      /** find in node tree **/
        name_t = fs_get_path_name(path_t, i);
        root_tmp = 0;       /** if all true, root_tmp will be the last parent node **/
        while(op_tmp){
            if (fs_string_cmp(op_tmp->filename, name_t) == 0){      /** match to filename **/
                root_tmp = op_tmp;
                op_tmp = op_tmp->child_list;    /** enter to child tree **/
                break;
            }
            op_tmp = op_tmp->next;
        }
        if(name_t)
            free(name_t);
    }

    if(!root_tmp) {
        if(path_lvl > 0)
            goto RET;
        root_tmp = ROOT;
    }

    root_tmp = root_tmp->child_list;
    name_t = fs_get_path_name(path_t, path_lvl);
    /** Search child file **/
    while(root_tmp){
        if(fs_string_cmp(name_t, root_tmp->filename) == 0){
            break;
        }
        root_tmp = root_tmp->next;
    }

    RET:    /** universally return **/
    free(path_t);
    if(name_t)
        free(name_t);
    return root_tmp;
}

/** 找到与句柄对应的已打开文件的信息节点 **/
void* imgfs::SearchForOpenedFile(uint32_t handle){
    imgfs_struct::imgfs_file* tmp = 0;
    if(handle && handle <= opened_count && FILE_LIST){
        imgfs_struct::imgfs_file* op = FILE_LIST;
        while(op){
            if(op->handle == handle){
                tmp = op;
                break;
            }
            op = op->next;
        }
    }
    return tmp;
}

/** 移动文件读取指针 **/
uint32_t imgfs::FileSeek(uint32_t handle, long offset, int start_p){
    imgfs_struct::imgfs_file* op_t = (imgfs_struct::imgfs_file*)SearchForOpenedFile(handle);
    if(!op_t)
        return 0;
    switch(start_p){
        case IMGFS_SEEK_SET:
            op_t->cur = 0 + offset;
            if(offset < 0)
                op_t->cur = 0;
            if(offset > op_t->node->data_length)
                op_t->cur = op_t->node->data_length;
            break;
        case IMGFS_SEEK_CUR:
            if((offset + op_t->cur > op_t->node->data_length) || (offset + op_t->cur < 0))
                break;
            op_t+=offset;
            break;
        case IMGFS_SEEK_END:
            op_t->cur = op_t->node->data_length;
            if(offset > 0)
                break;
            if(offset + op_t->cur < 0)
                break;
            op_t->cur+=offset;
        default:
            return op_t->cur;
    }
    return op_t->cur;
}

/** 获取当前文件读取指针 **/
uint32_t imgfs::FileTell(uint32_t handle){
    imgfs_struct::imgfs_file* op_t = (imgfs_struct::imgfs_file*)SearchForOpenedFile(handle);
    if(!op_t)
        return 0;
    return op_t->cur;
}

/** 从文件读取指定长度数据到缓冲区 **/
uint32_t imgfs::FileRead(uint32_t handle, void* buffer, uint32_t read_len){
    imgfs_struct::imgfs_file* op_t = (imgfs_struct::imgfs_file*)SearchForOpenedFile(handle);
    if(!op_t)
        return 0;
    _core_mutex_lock(read_lock);
    fseek(local, (op_t->node->data_address+op_t->cur) - ftell(local), SEEK_CUR);
    if(read_len > op_t->node->data_length - op_t->cur)
        read_len = op_t->node->data_length - op_t->cur;
    uint32_t ret = fread(buffer, read_len, 1, local);
    _core_mutex_unlock(read_lock);
    op_t->cur+=read_len;
    return ret;
}

/** 关闭打开的文件信息 **/
void imgfs::CloseFile(uint32_t handle){
    if(handle){
        imgfs_struct::imgfs_file* op_t = FILE_LIST;
        _core_mutex_lock(read_lock);
        while(op_t){
            if(op_t->next){
                if(op_t->next->handle == handle){
                    imgfs_struct::imgfs_file* tmp = op_t->next->next;
                    delete op_t->next;
                    op_t->next = tmp;
                    break;
                }
            }
            op_t = op_t->next;
        }
        _core_mutex_unlock(read_lock);
    }
    return;
}

/** 打开某文件，返回与之对应的描述句柄 **/
uint32_t imgfs::OpenFile(char* filename){
    imgfs_struct::imgfs_inode* op = (imgfs_struct::imgfs_inode*)SearchForFile(filename);
    if(!op){
        _core_service_log_print("[imgfs] File were not existed.");
        return 0;
    }
    if(!op->type) {
        _core_service_log_print("[imgfs] It's a directory.");
        return 0;
    }
    _core_service_log_print("[imgfs] opening file:");
    _core_service_log_print(filename);
    imgfs_struct::imgfs_file* extra = new imgfs_struct::imgfs_file();
    _core_mutex_lock(read_lock);
    /** link **/
    extra->node = op;
    extra->cur = 0;
    ++opened_count; /** counter **/
    extra->handle = opened_count;
    imgfs_struct::imgfs_file* root_t = FILE_LIST;
    if(root_t) {
        while (root_t->next) {
            root_t = root_t->next;
        }
        root_t->next = extra;
    }else{
        FILE_LIST = extra;
    }
    _core_mutex_unlock(read_lock);
    return opened_count;
}

/** 获取某目录下所有子成员的UniTable，该Table只记录文件名 **/
imgfs_struct::imgfs_uni_table* imgfs::getFilesOfParent(char* p_dir){
    imgfs_struct::imgfs_inode* obj = (imgfs_struct::imgfs_inode*)SearchForFile(p_dir);
    if(!obj)
        return 0;
    if(obj->type) {
        _core_service_log_print("[imgfs] gets dir. it's not a directory.");
        _core_service_log_print(p_dir);
        return 0;
    }
    uint32_t count_t = obj->child_count;
    obj = obj->child_list;
    imgfs_struct::imgfs_uni_table* first = new imgfs_struct::imgfs_uni_table();
    imgfs_struct::imgfs_uni_table* op_t = first;
    for(int i = 0; i < count_t; i++){
        uint32_t string_l = strlen(obj->filename);
        op_t->data = new char[string_l+1]();
        memset(op_t->data, 0, string_l+1);
        memcpy(op_t->data, obj->filename, string_l);
        if(i != count_t-1) {
            op_t->next = new imgfs_struct::imgfs_uni_table();
            op_t = op_t->next;
        }
        obj = obj->next;
    }
    return first;
}

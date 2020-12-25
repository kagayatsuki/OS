//
// Created by shinsya on 2020/12/15.
// 对某些不准确的英文注释换成了中文，哎是我懒得查单词了

#include "imgfs_struct.h"

imgfs::imgfs() {    //初始化
    already_file_list = new _IMGFS_A_FILE();
    already_file_list->filename = new char[FS_FILENAME_MAX + 1]();
    memset(already_file_list->filename, 0, FS_FILENAME_MAX + 1);
    already_file_list->filename[0] = '/';
    already_ino = 1;
    memset(&info, 0, sizeof(imgfs_struct::imgfs_info));
    local = 0;
    ROOT = 0;
    FILE_LIST = 0;
    opened_count = 0;
    proc_rate = write_proc = 0;
    _core_mutex_create(read_lock);
}

void imgfs::already_list_clear(_IMGFS_A_FILE_P p){
    if(p->next){
        already_list_clear(p->next);
    }
    if(p->child_list){
        already_list_clear(p->child_list);
    }

    if(p->filename)delete [] (char*)p->filename;
    if(p->local)fclose(p->local);
    delete p;
}

imgfs::~imgfs() {
    already_list_clear(already_file_list);
    if(local)
        fclose(local);
    _core_mutex_destroy(read_lock);
}

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
    for(int i = 0; i < lvl; i++){   //从0级开始按等级逐层获取到目标
        if(tmp_p){
            name_t = fs_get_path_name(path_t, i);   //单某一级的文件名
            tmp_p = (_IMGFS_A_FILE_P)already_file_get(tmp_p, name_t);
            if(name_t)
                free(name_t);
        }
    }
    return tmp_p;
}

void* imgfs::already_dir_append(_IMGFS_A_FILE_P p_struct, char* name){
    _IMGFS_A_FILE_P tmp = (_IMGFS_A_FILE_P)already_file_get(p_struct, name);
    _IMGFS_A_FILE_P table_t;
    if(tmp)
        return 0;  //目标已存在
    if(!p_struct || !name)  //空指针
        return 0;
    if(p_struct->type)
        return 0;  //父级非目录

    //节点追加
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

    /** 追加完成 **/
    return table_t;
}

void* imgfs::already_file_append(_IMGFS_A_FILE_P p_struct, char *name, char *local_path) {
    _IMGFS_A_FILE_P tmp = (_IMGFS_A_FILE_P)already_file_get(p_struct, name);
    _IMGFS_A_FILE_P table_t;
    if(tmp || !p_struct || !name || !local_path || p_struct->type)
        return 0;

    FILE* local_t = fopen(local_path, "rb");
    if(!local_t) {  //关联的本地文件无法打开,追加失败
        return 0;
    }

    table_t=p_struct->child_list;

    if(table_t){    //节点追加
        while(table_t->next){
            table_t=table_t->next;
            putchar('-');
        }
        table_t->next = new _IMGFS_A_FILE();
        table_t = table_t->next;
    }else{
        p_struct->child_list = new _IMGFS_A_FILE();
        table_t=p_struct->child_list;
    }

    /** 创建中进程 **/

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
    /** 创建完成 **/
    return table_t;
}

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

    if(!p_struct){  //父级目录不存在
        free(path_t);
        return -2;
    }
    name_t = fs_get_path_name(path_t, lvl_t);
    _IMGFS_A_FILE_P this_file = (_IMGFS_A_FILE_P)already_file_get(p_struct, name_t);
    if(this_file){  //该文件已存在
        if(name_t)
            free(name_t);
        free(path_t);
        return -3;
    }
    this_file = (_IMGFS_A_FILE_P)already_file_append(p_struct, name_t, local_path);
    if(!this_file){ //出现其他错误,未能完成追加
        if(name_t)
            free(name_t);
        free(path_t);
        return -4;
    }

    if(name_t)
        free(name_t);
    free(path_t);
    return already_ino-1;
}

int imgfs::create_directory_table_insert(char* path){
    _IMGFS_A_FILE_P parent_t;
    if(fs_path_check_illegal(path))     //仅注释本次: 路径合法检测
        return -5;
    if(!path)
        return -1;
    char* path_t = 0;
    int path_len = fs_path_format(path, &path_t);   //仅注释本次: 对路径标准化
    if(path_len < 2 || !path_t)
        return -1;
    path_level lvl_t = fs_get_path_level(path_t);   //仅注释本次: 路径级数
    char* name_t;

    parent_t = (_IMGFS_A_FILE_P)already_dir_get(path_t);    //仅注释本次: 解析路径,获得父节点指针

    if(!parent_t){      //其路径指定的父节点不存在,追加失败
        free(path_t);
        return -2;
    }
    if(parent_t->type){     //其路径中指定的父节点不是目录,追加失败
        free(path_t);
        return -3;
    }
    name_t = fs_get_path_name(path_t, lvl_t);   //获取路径中目的文件的文件名
    _IMGFS_A_FILE_P this_file = (_IMGFS_A_FILE_P)already_file_get(parent_t, name_t);    //试图寻找该文件存在的指针
    if(this_file){      //该将要追加的目录已经存在,追加失败
        if(name_t)
            free(name_t);
        free(path_t);
        return -3;
    }
    this_file = (_IMGFS_A_FILE_P)already_dir_append(parent_t, name_t);
    if(!this_file){     //追加函数返回了空指针,追加失败
        if(name_t)
            free(name_t);
        free(path_t);
        return -3;
    }

    if(name_t)
        free(name_t);
    free(path_t);
    return already_ino-1;
}

void cleanUniTable(imgfs_struct::imgfs_uni_table* first_p){
    if(!first_p)
        return;
    if(first_p->next)
        cleanUniTable(first_p->next);
    if(first_p->data)
        delete [] (char*)(first_p->data);
    delete first_p;
}

void cleanUniTableC(imgfs_struct::imgfs_uni_table* first_p){
    if(!first_p)
        return;
    if(first_p->next)
        cleanUniTable(first_p->next);
    if(first_p->data)
        delete (uint64_t*)(first_p->data);
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

imgfs_struct::imgfs_uni_table* imgfs::getAlreadyFilesOfParent(char* p_dir){
    if(!p_dir)
        return 0;
    char* format_path = 0;
    int len = fs_path_format(p_dir, &format_path);
    _IMGFS_A_FILE_P root = already_file_list;
    if(!format_path)
        return 0;

    if(len < 1){
        free(format_path);
        return 0;
    }
    if(len == 1){
        if(format_path[0] == '/'){  //断定其路径为根目录
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

uint64_t imgfs::GetDataLengthSum(_IMGFS_A_FILE_P root){
    uint64_t this_fl = 0;
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

uint32_t imgfs::MakeLocalAddr(_IMGFS_A_FILE_P root_child, uint64_t* start_addr_t, uint64_t* start_addr_d){
    uint64_t* proc_addr_t = start_addr_t, *proc_addr_d = start_addr_d;
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

uint32_t imgfs::GetTsize(){
    return GetNodeTableLength(already_file_list);
}

uint64_t imgfs::GetDsize(){
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
    fwrite(&meta->data_length, sizeof(uint64_t), 1, file);
    fwrite(&meta->local_addr, sizeof(uint64_t), 1, file);
    fwrite(&meta->child_count, sizeof(uint32_t), 1, file);
}

uint64_t imgfs::imgfs_write_data(FILE* file_t, _IMGFS_A_FILE_P root){
    uint64_t writen_d = 0;
    _IMGFS_A_FILE_P root_t = root;
    char buff[4096];
    while(root_t){
        if(root_t->type && root_t->local){
            fseek(root_t->local, 0, SEEK_SET);
            fseek(file_t, root_t->local_addr - ftell(file_t), SEEK_CUR);
            for(int t = 0; t < root_t->data_length / 4096; t++){    //按4K数据为一个块来写入
                memset(buff, 0, 4096);
                fread(buff, 4096, 1, root_t->local);
                fwrite(buff, 4096, 1, file_t);
                write_proc+=proc_rate;  //写入进度
                //printf("%5.2f%%\b\b\b\b\b\b", write_proc);  //进度输出,稍微影响一点性能,可以注释掉
                //fflush(stdout);
            }
            fflush(file_t);
            memset(buff, 0, 4096);
            if(root_t->data_length % 4096){     //对不足4K的情况的补充
                fread(buff, root_t->data_length % 4096, 1, root_t->local);
                fwrite(buff, root_t->data_length % 4096, 1, file_t);
                fflush(file_t);
            }
            writen_d+= root_t->data_length;
        }else{  //对目录的写入,即递归写入子节点
            writen_d+= imgfs_write_data(file_t, root_t->child_list);
        }
        fseek(file_t, root_t->table_addr - ftell(file_t), SEEK_CUR);
        /** 写完数据后再向元数据区写入元数据 **/
        imgfs_meta_write(file_t, root_t);

        _IMGFS_A_FILE_P tmp = root_t->child_list;
        for(int i = 0; i < root_t->child_count; i++){   //若元类型为目录,则还需在元数据后紧跟每个子节点元的数据偏移量
            if(tmp) {
                fwrite(&tmp->table_addr, sizeof(uint64_t), 1, file_t);
                tmp = tmp->next;
            }
        }
        root_t = root_t->next;
    }
    return writen_d;
}

/** 从预备区创建镜像 **/
int imgfs::create(char *local_path) {
    FILE* local_t = fopen(local_path, "wb+");

    if(!local_t) {  //无法创建文件
        return -1;
    }

    uint64_t table_addr;
    uint64_t tSize = GetTsize(), dSize = GetDsize();    //计算元数据区大小和数据长度,用于和记录值做核对
    if(dSize != info.data_length){  //信息不匹配,可能某处处理出错
        fclose(local_t);
        return -2;
    }

    info.data_length = dSize;
    _core_log_date* tmp = _core_log_date_get(); //获取当前的时间
    memcpy(&info.ctime, tmp, sizeof(_core_log_date));   //文件系统创建时间
    free(tmp);

    fseek(local_t, 0, SEEK_SET);
    /** 写入文件系统标志 **/
    fwrite(&imgfs_struct::imgfs_magic_code, sizeof(uint32_t), 1, local_t);
    fwrite(imgfs_struct::header_string, strlen(imgfs_struct::header_string), 1, local_t);
    /** 写入基础信息 **/
    fwrite(&info, sizeof(info), 1, local_t);
    /** 记录元数据区开始的数据偏移量(紧跟基础信息) **/
    table_addr = ftell(local_t);

    proc_rate = dSize;
    /** 为元信息先写一块空数据 **/
    imgfs_write_blank_table(local_t, tSize);
    fseek(local_t, table_addr, SEEK_SET);

    uint64_t d_start = tSize + table_addr, t_start = table_addr;
    uint32_t cRet = MakeLocalAddr(already_file_list, &t_start, &d_start);
    if(cRet != t_start || cRet != table_addr + tSize) {     //计算的数据与记录理论值不相符,可能出了某些问题
        fclose(local_t);
        return -3;
    }

    cRet = imgfs_write_data(local_t, already_file_list);    //实际写入的数据长度

    return cRet;
}

/** 生成或追加无符号整数UniTable **/
imgfs_struct::imgfs_uni_table* imgfs_counter_add(imgfs_struct::imgfs_uni_table* ptr, uint64_t data){
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
    tmp->data = new uint64_t();
    *(uint64_t*)tmp->data = data;
    return ret;
}

/** 装载过程中读取元数据的函数。不得直接调用 **/
void imgfs_mount_read_meta(imgfs_struct::imgfs_inode* inode_s, FILE* file_t){
    inode_s->filename = new char[FS_FILENAME_MAX+1]();
    fread(&inode_s->type, sizeof(char), 1, file_t);
    fread(inode_s->filename, FS_FILENAME_MAX, 1, file_t);
    fread(&inode_s->data_length, sizeof(uint64_t), 1, file_t);
    fread(&inode_s->data_address, sizeof(uint64_t), 1, file_t);
    fread(&inode_s->child_count, sizeof(uint32_t), 1, file_t);
}

/** 递归式从元数据挂载文件系统文件记录表 **/
int imgfs_mount_root(imgfs_struct::imgfs_inode* root, FILE* file_t){
    uint64_t read_t = 0;
    imgfs_struct::imgfs_uni_table *counter = 0, *op_counter;

    /** 建立counter,记录子节点元数据偏移量 **/
    for (int i = 0; i < root->child_count; i++){
        fread(&read_t, sizeof(uint64_t), 1, file_t);
        counter = imgfs_counter_add(counter, read_t);
    }
    op_counter = counter;
    imgfs_struct::imgfs_inode* op_child;
    for (int t = 0; t < root->child_count; t++){
        /** 读取子节点 **/
        fseek(file_t, *(uint64_t*)op_counter->data - ftell(file_t), SEEK_CUR);
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

    /** 读取子节点中的目录节点 **/
    op_child = root->child_list;
    op_counter = counter;
    while (op_child){
        if(!op_child->type){
            fseek(file_t, (*(uint64_t*)op_counter->data + _IMGFS_NODE_BASE_SIZE)-ftell(file_t), SEEK_CUR);
            imgfs_mount_root(op_child, file_t);
        }
        op_child = op_child->next;
        op_counter = op_counter->next;
    }
    cleanUniTableC(counter);
    return 0;
}

/** 挂载镜像 **/
int imgfs::mount(char* local_path){
    if(local || !local_path)
        return -1;
    FILE* local_t = fopen(local_path, "rb+");
    if(!local_t)
        return -2;
    /** 文件系统标识检查 **/
    char sFlag[10] = {0};
    fseek(local_t, 0, SEEK_SET);
    fread(sFlag, 10, 1, local_t);
    if(fs_string_cmp(&sFlag[4], (char*)imgfs_struct::header_string) || (*(uint32_t*)sFlag != imgfs_struct::imgfs_magic_code)){     //不匹配
        fclose(local_t);
        return -3;
    }
    /** 读取基本信息 **/
    fread(&info, sizeof(info), 1, local_t);
    /** 初始化root节点信息 **/
    ROOT = new imgfs_struct::imgfs_inode();
    imgfs_mount_read_meta(ROOT, local_t);

    local = local_t;
    /** 正式加载过程 **/
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

void* imgfs::SearchForFile(char* path){
    if(!ROOT){  //未挂载
        return 0;
    }
    char* path_t = 0;
    int len_t = fs_path_format(path, &path_t);
    if(!path_t){
        return 0;
    }
    if(len_t < 1){  //不合法路径
        free(path_t);
        return 0;
    }
    if(len_t == 1){
        if(path_t[0] == '/'){   //判断是否为根目录
            free(path_t);
            return ROOT;
        }
        return 0;
    }

    path_level path_lvl = fs_get_path_level(path_t);
    imgfs_struct::imgfs_inode *root_tmp = 0, *op_tmp = ROOT->child_list;
    char *name_t;
    for(int i = 0; i < path_lvl; i++){      /** 从文件树逐级寻找 **/
        name_t = fs_get_path_name(path_t, i);
        root_tmp = 0;       /** 如果一切无误，root_tmp记录最后一级父级节点的地址 **/
        while(op_tmp){
            if (fs_string_cmp(op_tmp->filename, name_t) == 0){      /** 匹配对应级的文件名 **/
                root_tmp = op_tmp;
                op_tmp = op_tmp->child_list;    /** 进入子树 **/
                break;
            }
            op_tmp = op_tmp->next;
        }
        if(name_t)
            free(name_t);
    }

    if(!root_tmp) {     //没有成功获得父级
        if(path_lvl > 0)
            goto RET;
        root_tmp = ROOT;
    }

    root_tmp = root_tmp->child_list;    //父级子树
    name_t = fs_get_path_name(path_t, path_lvl);
    /** 从父级的子树中匹配文件名 **/
    while(root_tmp){
        if(fs_string_cmp(name_t, root_tmp->filename) == 0){
            break;
        }
        root_tmp = root_tmp->next;
    }

    RET:
    free(path_t);
    if(name_t)
        free(name_t);
    return root_tmp;
}

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

uint32_t imgfs::FileSeek(uint32_t handle, uint64_t offset, int start_p){
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

uint64_t imgfs::FileTell(uint32_t handle){
    imgfs_struct::imgfs_file* op_t = (imgfs_struct::imgfs_file*)SearchForOpenedFile(handle);
    if(!op_t)
        return 0;
    return op_t->cur;
}

uint32_t imgfs::FileRead(uint32_t handle, void* buffer, uint32_t read_len){
    imgfs_struct::imgfs_file* op_t = (imgfs_struct::imgfs_file*)SearchForOpenedFile(handle);
    if(!op_t)
        return 0;
    _core_mutex_lock(read_lock);    //数据读取不可并发,对互斥锁上锁,应用层IO阻塞
    char buff[512] = {0};       //该缓冲区并非多此一举,之后数据加密会用到
    uint32_t off_mod = 0;
    fseek(local, (op_t->node->data_address+op_t->cur) - ftell(local), SEEK_CUR);    //本地偏移映射
    if(read_len > op_t->node->data_length - op_t->cur)
        read_len = op_t->node->data_length - op_t->cur;
    off_mod = read_len % 512;
    int i = 0;
    uint32_t ret = 0;
    for (; i < read_len / 512; i++){
        ret+= fread(buff, 512, 1, local);
        memcpy(((char*)buffer + 512*i), buff, 512);
    }
    if(off_mod){
        ret+= fread(buff, off_mod, 1, local);
        memcpy(((char*)buffer + 512*i), buff, off_mod);
    }
    _core_mutex_unlock(read_lock);  //解除阻塞
    op_t->cur+=read_len;
    return ret;
}

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

uint32_t imgfs::OpenFile(char* filename){
    imgfs_struct::imgfs_inode* op = (imgfs_struct::imgfs_inode*)SearchForFile(filename);
    if(!op){    //不存在
        return 0;
    }
    if(!op->type) {     //非数据文件
        return 0;
    }
    imgfs_struct::imgfs_file* extra = new imgfs_struct::imgfs_file();
    _core_mutex_lock(read_lock);
    /** 抽象结构建立 **/
    extra->node = op;
    extra->cur = 0;
    ++opened_count;
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

imgfs_struct::imgfs_uni_table* imgfs::getFilesOfParent(char* p_dir){
    imgfs_struct::imgfs_inode* obj = (imgfs_struct::imgfs_inode*)SearchForFile(p_dir);
    if(!obj)
        return 0;
    if(obj->type) { //非目录
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

uint32_t imgfs::CreateLocalFolders(char* local_root, imgfs_struct::imgfs_inode* root){
    uint32_t ret = 0;
    imgfs_struct::imgfs_inode* op_t = root;
    if(op_t == ROOT)
        op_t = op_t->child_list;
    char path_t[1025];
    memset(path_t, 0, 1025);
    int root_path_len = strlen(local_root);
    memcpy(path_t, local_root, root_path_len);
    if(path_t[root_path_len-1] != '\\' && path_t[root_path_len-1] != '/'){
        path_t[root_path_len] = '/';
        root_path_len++;
    }

    while(op_t){
        if(!op_t->type){
            memset(path_t+root_path_len, 0, 1025 - root_path_len);
            memcpy(path_t+root_path_len, op_t->filename, strlen(op_t->filename));
            printf("[imgfs] Coping: create folder - %-64s", path_t);
            bool t = CreateDirectory(path_t, NULL);
            if(t) {
                printf("  success.\n");
                ret++;
            }
            else
                printf("  failed.\n");
            if(op_t->child_list)
                ret+= CreateLocalFolders(path_t, op_t->child_list);
        }
        op_t = op_t->next;
    }
    return ret;
}

uint64_t imgfs::WriteLocalFiles(char* local_root, imgfs_struct::imgfs_inode* root){
    uint64_t ret = 0;
    imgfs_struct::imgfs_inode* op_t = root;
    if(op_t == ROOT)
        op_t = op_t->child_list;
    char path_t[1025];  //base filename
    memset(path_t, 0, 1025);
    char wr_tmp[4096];
    int root_path_len = strlen(local_root);
    memcpy(path_t, local_root, root_path_len);
    if(path_t[root_path_len-1] != '\\' && path_t[root_path_len-1] != '/'){
        path_t[root_path_len] = '/';
        root_path_len++;
    }

    while(op_t){
        memset(path_t+root_path_len, 0, 1025 - root_path_len);
        memcpy(path_t+root_path_len, op_t->filename, strlen(op_t->filename));
        if(op_t->type){
            printf("[imgfs] Coping: writing file - %-64s", path_t);
            FILE* file_t = fopen(path_t, "wb+");
            if(!file_t){
                printf("  failed.\n");
            }else{
                fseek(file_t, 0, SEEK_SET);
                fseek(local, op_t->data_address, SEEK_SET);
                int count = 0;
                for(int i = 0; i < op_t->data_length / 4096; i++){
                    memset(wr_tmp, 0, 4096);
                    fread(wr_tmp, 4096, 1, local);
                    fwrite(wr_tmp, 4096, 1, file_t);
                    count++;
                    ret++;
                    printf("%5.2f%%\b\b\b\b\b\b", (100.0 / (op_t->data_length / 4096.0)) * (i+1));
                    fflush(stdout);
                }
                fflush(file_t);
                memset(wr_tmp, 0, 4096);
                if(op_t->data_length % 4096){
                    fread(wr_tmp, op_t->data_length % 4096, 1, local);
                    count += fwrite(wr_tmp, op_t->data_length % 4096, 1, file_t)*op_t->data_length % 4096;
                    fflush(file_t);
                    ret+=op_t->data_length % 4096;
                }
                fclose(file_t);
                printf("  success.[%d bytes]\n", count);
            }
        }else{
            ret+=WriteLocalFiles(path_t, op_t->child_list);
        }
        op_t = op_t->next;
    }

    return ret;
}

uint64_t imgfs::CopyImage(char* local_path){
    if(!local_path)
        return 0;

    bool local_root_t = CreateDirectory(local_path, NULL);
    if(!local_root_t){
        printf("[imgfs] can not create local root directory.\n");
        return 0;
    }
    printf("[imgfs] creating local folders.\n");
    printf("[imgfs] Coping: created %d directories\n", CreateLocalFolders(local_path, ROOT));
    printf("[imgfs] writing local files.\n");
    return WriteLocalFiles(local_path, ROOT);
}

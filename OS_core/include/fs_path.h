//
// Created by shinsya on 2020/12/13.
//

#ifndef OS_VFS_DEBUG_FS_PATH_H
#define OS_VFS_DEBUG_FS_PATH_H

#define FS_FILENAME_MAX 255
#define FS_PATH_MAX 2047

#include <calls.h>
#include <types.h>

/** format path string. like "/var\..///a/./c" to "/var/../a/c" **/
int fs_path_format(char* path, char** path_buffer){
    if(!path_buffer || !path)
        return -1;
    char buff[FS_PATH_MAX] = {0};
    int path_len = strlen(path);
    int buff_len = path_len;
    memcpy(buff, path, path_len);
    bool undone;
    do{
        undone = false;
        for(int i = 0; i < buff_len; i++){
            if(buff[i] == '\\'){
                buff[i] = '/';
                undone = true;
            }else if(buff[i] == '/'){
                if(i>0){
                    if(buff[i-1] == '/'){
                        memcpy(&buff[i-1], &buff[i], buff_len - i);
                        buff_len--;
                        undone = true;
                        break;
                    }else if(i < buff_len-1 && i > 1){
                        if(buff[i-1] == '.' && buff[i-2] == '/'){
                            undone = true;
                            memcpy(&buff[i-2], &buff[i], buff_len - i);
                            buff_len-=2;
                        }
                    }
                }
            }
        }
    }while (undone);
    char* ret = (char*)malloc(buff_len+1);
    ret[buff_len] = '\0';
    memcpy(ret, buff, buff_len);
    *path_buffer = ret;
    return buff_len;
}

/** get max level of path **/
path_level fs_get_path_level(char* path_buffer){
    char* format_t = 0;         /** format string **/
    fs_path_format(path_buffer, &format_t);
    if(!format_t)
        return -1;
    path_buffer = format_t;

    int len = strlen(path_buffer);
    if(len < 2 || !path_buffer)
        return -1;
    int subs = 0, level = -1;
    if((path_buffer[0] != '.') && (path_buffer[0] != '/'))
        return -1;

    for(;subs<len;subs++){
        if(path_buffer[subs] == '/') {
            if (subs > 0){
                if (path_buffer[subs - 1] != '/')
                    level++;
            }else
                level++;
        }else if(path_buffer[subs] == '.'){
            if (subs < len - 2){
                if(path_buffer[subs+1] == '.' && path_buffer[subs+2] == '/'){
                    level-=2;
                    if(subs > 0){
                        if(path_buffer[subs-1] != '/')
                            level+=2;
                    }
                }
            }
        }
    }
    free(format_t);
    if(path_buffer[len-1] == '/')
        level--;
    return level;
}

/** get filename of path by one level **/
char* fs_get_path_name(char* path_buffer, path_level target_level){
    char* format_t = 0;         /** format string **/
    fs_path_format(path_buffer, &format_t);
    if(!format_t)
        return 0;
    path_buffer = format_t;

    if(fs_get_path_level(path_buffer) == -1)
        return 0;
    char buff[FS_FILENAME_MAX] = {0};
    int buff_subs = 0, buff_len = 0;
    int level = -1, len = strlen(path_buffer), subs = 0;
    for(; subs < len; subs++){
        if(path_buffer[subs] == '/'){
            if(subs > 0){
                if(path_buffer[subs - 1] != '/') {
                    level++;
                    buff_subs=0;
                    if(level == target_level) buff_len=0;
                }
            }else{
                level++;
                buff_subs=0;
                if(level == target_level) buff_len=0;
            }
        }else if(path_buffer[subs] == '.'){
            if(subs < len - 2){
                if(path_buffer[subs+1] == '.' && path_buffer[subs+2] == '/'){
                    int last_level = level;
                    level-=2;
                    if(subs > 0){
                        if(path_buffer[subs-1] != '/')
                            level+=2;
                    }
                    if(last_level != level){
                        buff_subs=0;
                        if(level == target_level) buff_len=0;
                    }
                }
            }
        }
        if(level == target_level){
            buff[buff_subs] = path_buffer[subs];
            buff_subs++;
            buff_len++;
        }
    }
    free(format_t);
    if(buff_len == 0)
        return 0;
    char* ret = (char*)malloc(buff_len+1);
    memset(ret,0,buff_len+1);
    memcpy(ret, &buff[1], buff_len - 1);
    return ret;
}

/** merge work path and provided path, return full path length **/
int fs_get_path_merge(char* work_path, char* path_buffer, char** merge_buffer){
    if(!work_path || !path_buffer || !merge_buffer)
        return -1;

    char* format_t = 0;         /** format string **/
    fs_path_format(path_buffer, &format_t);
    if(!format_t)
        return -1;
    path_buffer = format_t;
    format_t = 0;
    fs_path_format(work_path, &format_t);
    if(!format_t){
        free(path_buffer);
        return -1;
    }
    work_path = format_t;

    int work_path_len = strlen(work_path);
    int path_len = strlen(path_buffer), p_start = 0;
    int merge_len = 0, loop_t = 0;
    char merge_tmp[FS_PATH_MAX+1] = {0};
    for(;loop_t < path_len; loop_t++){
        if(path_buffer[loop_t] != '/') {    /** like "/path/a.txt" to "path/a.txt" **/
            p_start = loop_t;
            break;
        }
    }
    memcpy(merge_tmp, work_path, work_path_len);
    if(work_path[work_path_len-1] != '/'){      /** like "/var/tmp" to "/var/tmp/" **/
        merge_tmp[work_path_len] = '/';
        work_path_len++;
    }
    memcpy(&merge_tmp[work_path_len], &path_buffer[p_start], path_len - p_start); /**  merge '/var/tmp' and './path/a.txt' to '/var/tmp/path/a.txt'  **/
    merge_len = strlen(merge_tmp);

    int last_split = -1;
    bool undone;     //flag
    do{
        loop_t = 0;
        undone = false;
        for(; loop_t < merge_len; loop_t++){
            if(merge_tmp[loop_t] == '/'){
                if(loop_t <= merge_len-2){
                    if(merge_tmp[loop_t+1] == '.' && merge_tmp[loop_t+2] == '.'){
                        undone = true;
                        if(last_split == -1){   /** like "/../xxx" is illegal **/
                            *merge_buffer = 0;
                            free(work_path);
                            free(path_buffer);
                            return -1;
                        }
                        memcpy(&merge_tmp[last_split], &merge_tmp[loop_t+3], merge_len - (loop_t+3));
                        merge_len-= loop_t - last_split + 3;
                        break;
                    }else{
                        last_split = loop_t;
                    }
                }
            }
        }
    }while (undone);
    free(work_path);
    free(path_buffer);
    char* ret = (char*)malloc(merge_len+1);
    memset(ret,0,merge_len+1);
    memcpy(ret, merge_tmp, merge_len);
    *merge_buffer = ret;
    return merge_len;
}

/** simple path merge **/
int make_path(char** full_path_buffer, char* dir, char* filename){
    if(!filename || !full_path_buffer || !dir)
        return -1;

    char* format_t = 0;         /** format string **/
    fs_path_format(dir, &format_t);
    if(!format_t)
        return -1;
    dir = format_t;
    format_t = 0;
    fs_path_format(filename, &format_t);
    if(!format_t) {
        free(dir);
        return -1;
    }
    filename = format_t;

    char tmp[FS_PATH_MAX+1] = {0};
    int fl = strlen(filename), dl = strlen(dir);
    if(fl + dl > FS_PATH_MAX)
        return -2;

    memcpy(tmp, dir, dl);
    if(dir[dl-1] != '/'){
        tmp[dl] = '/';
        dl++;
    }

    for(int i = 0; i < fl; i++){
        if(filename[i] != '/'){
            memcpy(&tmp[dl], &filename[i], fl - i);
            break;
        }
    }
    int len = strlen(tmp);
    char* ret = (char*)malloc(len+1);
    memset(ret, 0, len+1);
    memcpy(ret, tmp, len);
    *full_path_buffer = ret;
    return len;
}

int fs_path_check_illegal(char* path){
    if(!path)
        return -1;
    int len = strlen(path);
    for(int i = 0; i < len; i++){
        if(((path[i] > 'a' && path[i] < 'z') || (path[i] > 'A' && path[i] < 'Z') || (path[i] > '0' && path[i] < '9')));
        else if((path[i]=='.' || path[i]=='-' || path[i]=='+' || path[i]=='=' || path[i]=='_' || path[i]=='\\' || path[i]=='/'));
        else return 1;
    }
    return 0;
}

int fs_string_cmp(char* a, char* b){
    if(!a || !b)
        return -1;
    int al = strlen(a);
    int bl = strlen(b);
    if(al != bl)
        return 1;
    for(int i = 0; i < al; i++){
        if(a[i] != b[i])
            return 1;
    }
    return 0;
}

#endif //OS_VFS_DEBUG_FS_PATH_H

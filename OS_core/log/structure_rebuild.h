/*******************************************
 * Log system basic structure
 * Rebuild ver
 * :shinsya
 *******************************************/
#ifndef LOG_STRUCTURE
#define LOG_STRUCTURE

#define winPlatform //debug in windows

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <time.h>
#include <errno.h>

#include "logtime.h"

#ifdef winPlatform  //platform chosen
#include <windows.h>

typedef HANDLE _core_mutex;
#define _core_mutex_create(mutex) (mutex = CreateMutex(nullptr, false, NULL), \
                                    GetLastError() == ERROR_ALREADY_EXISTS)
#define _core_mutex_lock(mutex) WaitForSingleObject(mutex, INFINITE)
#define _core_mutex_trylock(mutex) WaitForSingleObject(mutex, 50)
#define _core_mutex_unlock(mutex) ReleaseMutex(mutex)
#define _core_mutex_destroy(mutex) CloseHandle(mutex)
#else   //linux
#include <pthread.h>

typedef pthread_mutex_t _core_mutex;
#define _core_mutex_create(mutex) (pthread_mutex_init(&mutex, NULL), mutex_addr)
#define _core_mutex_lock(mutex) pthread_mutex_lock(&mutex)
#define _core_mutex_trylock(mutex) pthread_mutex_trylock(&mutex)
#define _core_mutex_unlock(mutex) pthread_mutex_unlock(&mutex)
#define _core_mutex_destroy(mutex) pthread_mutex_destroy(&mutex)
#endif

#ifndef strnlen
#define strnlen(x,y) strlen(x)
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#endif

#ifndef uint16_t
#define uint16_t unsigned short
#endif

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef nullptr
#define nullptr 0
#endif

#define LOG_MAX_COUNT 1000
#define LOG_MAX_LEN 512

//log system basic structure
struct _core_log_struct{
    char* log_details;
    struct _core_log_date* log_date;
};

struct _core_log{
    uint32_t log_id;
    struct _core_log_struct* log_struct;
    struct _core_log* last;
};

struct{
    struct _core_log* log_list;
    uint32_t log_count, log_id_new;
    _core_mutex _log_operate_mutex;
    bool inited;
}_core_log_sys;

using namespace std;


void _core_log_clean_all();


//this log sys have to init
int _core_log_sys_init(){
    if(_core_log_sys.inited){
        printf("Log sys: log sys init failed. (It was already inited)\n");
        return -1;
    }
    if(_core_mutex_create(_core_log_sys._log_operate_mutex)){
        printf("Log sys: log sys init failed. (Can not create mutex)\n");
        return -1;
    }
    _core_log_sys.log_list = nullptr;
    _core_log_sys.log_count = 0;
    _core_log_sys.log_id_new = 0;
    _core_log_sys.inited = true;
    return 0;
}

int _core_log_sys_unload(){
    if(!_core_log_sys.inited)
        return -1;
    printf("log sys unloading.\n");
    _core_log_clean_all();
    printf("log cleaned.\n");
    if(!_core_mutex_destroy(_core_log_sys._log_operate_mutex)){
        printf("log service: can not destroy operation mutex of log sys.\n");
        return -1;
    }
    _core_log_sys.inited = false;
    return 0;
}

//get log data by log id
_core_log_struct* _core_log_get_by_id(uint32_t id){
    struct _core_log* tmp = _core_log_sys.log_list;
    if(!tmp)return 0;
    while(tmp){
        if(tmp->log_id == id)return tmp->log_struct;
        tmp = tmp->last;
    }
    return 0;
}

_core_log* _core_log_get_first(){
    if(!_core_log_sys.log_list)
        return nullptr;
    struct _core_log* tmp = _core_log_sys.log_list;
    while(tmp->last)
        tmp = tmp->last;
    return tmp;
}

_core_log* _core_log_get_second(){
    if(!_core_log_sys.log_list || (_core_log_sys.log_count < 2))
        return nullptr;
    struct _core_log* tmp = _core_log_sys.log_list;
    if(!(tmp->last))
        return nullptr;
    while(tmp->last->last){
        tmp = tmp->last;
    }
    return tmp;
}

//destroy first log in sys when log count will cross count limit
void _core_log_free_first(){
    if(!_core_log_sys.log_list || (_core_log_sys.log_count == 0) || !_core_log_sys.inited)
        return;
    _core_mutex_lock(_core_log_sys._log_operate_mutex);
    struct _core_log* tmp_first = _core_log_get_first(), * tmp_second = _core_log_get_second();
    if(tmp_first->log_struct->log_details) free(tmp_first->log_struct->log_details);
    if(tmp_first->log_struct->log_date) free(tmp_first->log_struct->log_date);
    free(tmp_first->log_struct);
    free(tmp_first);
    _core_log_sys.log_list = tmp_second;
    if(tmp_second) {
        tmp_second->last = 0;
        //_core_log_sys.log_list = tmp_second;
    }
    _core_mutex_unlock(_core_log_sys._log_operate_mutex);
    //debug
    //printf("cleaned\n");
    _core_log_sys.log_count--;
}

void _core_log_clean_all(){
    uint32_t tmp_c = _core_log_sys.log_count;
    for(;tmp_c > 0; tmp_c--)
        //debug
        //printf("%u\n",tmp_c);
        _core_log_free_first();
}

//append log to sys, return log id, any exception return 0
uint32_t _core_log_append(const char* log_detail, struct _core_log_date* log_date){
    uint32_t tmp_log_id = 0;
    if(!_core_log_sys.inited)return 0;
    struct _core_log_date* date_tmp = log_date;

    //Exception check every initial step
    if(!log_detail) //log detail can't be null
        return 0;
    if(!log_date){  //if not provide date then set time when append
        date_tmp = _core_log_date_get();
    }
    _core_mutex_lock(_core_log_sys._log_operate_mutex);
    struct _core_log_struct* struct_tmp = (struct _core_log_struct*)malloc(sizeof(_core_log_struct));
    if(!struct_tmp){
        _core_mutex_unlock(_core_log_sys._log_operate_mutex);
        printf("Exception: bad malloc when append log.\n");
        return 0;
    }
    struct _core_log* log_tmp = (struct _core_log*)malloc(sizeof(_core_log));
    if(!log_tmp){
        _core_mutex_unlock(_core_log_sys._log_operate_mutex);
        printf("Exception: bad malloc when append log.\n");
        free(struct_tmp);
        free(date_tmp);
    }
    struct_tmp->log_details = (char*)malloc(strnlen(log_detail, LOG_MAX_LEN) + 1);
    if(!struct_tmp->log_details){
        _core_mutex_unlock(_core_log_sys._log_operate_mutex);
        printf("Exception: bad malloc when append log.\n");
        free(struct_tmp);
        free(date_tmp);
        free(log_tmp);
    }

    //Exception checked. Set value next step
    tmp_log_id = ++_core_log_sys.log_id_new;
    log_tmp->log_struct = struct_tmp;
    struct_tmp->log_date = date_tmp;
    memcpy(struct_tmp->log_details, log_detail, strnlen(log_detail, LOG_MAX_LEN));
    struct_tmp->log_details[strnlen(log_detail, LOG_MAX_LEN)] = '\0';
    log_tmp->log_id = tmp_log_id;

    //Set link list
    if(_core_log_sys.log_count >= LOG_MAX_COUNT){
        _core_mutex_unlock(_core_log_sys._log_operate_mutex);
        _core_log_free_first();
        _core_mutex_lock(_core_log_sys._log_operate_mutex);
    }
    if(_core_log_sys.log_list){
        log_tmp->last = _core_log_sys.log_list;
    }else{
        log_tmp->last = 0;
    }
    _core_log_sys.log_list = log_tmp;
    _core_log_sys.log_count++;
    _core_mutex_unlock(_core_log_sys._log_operate_mutex);
    return tmp_log_id;
}

#endif
//gcc new line

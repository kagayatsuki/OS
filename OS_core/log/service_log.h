/*************************************************
 * This module let all log module be a entirety
 * to provide sys api for log service
 * :shinsya
 ************************************************/

#ifndef OS_CORE_SERVICE_LOG_H
#define OS_CORE_SERVICE_LOG_H

#include "structure.h"
#include "local.h"
#include "remote.h"

int flag_log_service_running = 0;

/** start log service, the arg switch init log remote module or not (if 0 , then don't init remote)
    All failed return -1, local failed return 1, remote failed return 2, all done return 0 **/
int _core_service_log_start(int arg_remote){
    //init local module:
    int tmp_init_local = _core_log_local_init();
    //init remote module:
    int tmp_init_remote = 0;
    if(arg_remote)
        tmp_init_remote = _core_log_remote_init();

    if(tmp_init_local && tmp_init_remote)
        return -1;
    flag_log_service_running = 1;
    if(tmp_init_local)
        return 1;
    else if(tmp_init_remote)
        return 2;
    return 0;
}

//if service was not running, return -2, close remote timeout return -1, else return 0
int _core_service_log_stop(){
    if(!flag_log_service_running)return -2;
    int remote_close = 0;
    _core_log_date* tmp_t = _core_log_get_time();
    int tmp_log_id = _core_log_append("log service: log service will be stop.", tmp_t);
    if(tmp_t) free(tmp_t);
    _core_log_local_write(_core_log_get_by_id(tmp_log_id));
    _core_log_local_close();
    remote_close = _core_log_remote_service_stop();
    if(remote_close)return -1;
    _core_log_clean_all();
    return 0;
}

void _core_service_log_print(char* log_details){
    if(!log_details)return;
    struct _core_log_date* tmp_date = _core_log_get_time();
    unsigned int tmp_log_id = _core_log_append(log_details, tmp_date);
    if(tmp_date) free(tmp_date);
    struct _core_log_struct* tmp_struct = _core_log_get_by_id(tmp_log_id);
    _core_log_local_write(tmp_struct);
    _core_log_remote_event_append(tmp_struct);
    return;
}

#endif //OS_CORE_SERVICE_LOG_H

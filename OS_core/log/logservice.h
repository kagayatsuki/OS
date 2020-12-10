/*************************************************
 * This module let all log module be a entirety
 * to provide sys api for log service
 * Rebuild ver
 * :shinsya
 ************************************************/

#ifndef CORE_LOG_SERVICE
#define CORE_LOG_SERVICE

#include "structure_rebuild.h"
#include "local_rebuild.h"
#include "remote_rebuild.h"

int flag_log_service_running = 0;

int _core_service_log_start(){
    //debug:
    _core_log_set_echo_mode(true);
    if(flag_log_service_running)
        return -1;
    _core_log_socket_init();
    /** init basic structure **/
    int _base_sys = _core_log_sys_init();
    if(_base_sys)
        return -1;
    int _local_sys = _core_log_local_init();
    if(_local_sys){
        _core_log_sys_unload();
        return -1;
    }
    int _local_service = _core_log_remote_local_server_start();
    int _remote_service = _core_log_remote_server_connect();
    if(_local_service)
        printf("local server@log remote module: init failed.\n");
    if(_remote_service)
        printf("remote server@log remote module: connect failed.\n");
    flag_log_service_running = 1;
    return 0;
}

int _core_service_log_stop(){
    if(!flag_log_service_running)
        return -1;
    printf("Waiting for log sys.\n");
    if(_core_log_remote_service_stop())
        printf("remote log service stop failed.\n");
    else
        printf("remote log service stopped.\n");
    if(_core_log_remote_local_server_stop())
        printf("local log service stop failed.\n");
    else
        printf("local log service stopped.\n");
    if(_core_log_local_close())
        printf("local log close failed.\n");
    else
        printf("local log closed.\n");
    if(_core_log_sys_unload())
        printf("log sys stop failed.\n");

    printf("log sys stopped.\n");
    return 0;
}

void _core_service_log_print(const char* log_detail){
    int id_t = _core_log_append(log_detail, _core_log_date_get());
    _core_log_local_append(_core_log_get_by_id(id_t));
    _core_log_remote_message_append(_core_log_get_by_id(id_t), 2);
    //_core_log_remote_message_append(_core_log_get_by_id(id_t), 0);
}

#endif //CORE_LOG_SERVICE

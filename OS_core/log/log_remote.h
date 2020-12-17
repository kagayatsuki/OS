/******************************************************************
 * log module, remote part (rebuild ver)
 * This module create a socket connection to net log catch client
 * And send log data to client by log service
 * :shinsya
 *****************************************************************/

#ifndef OS_CORE_REMOTE_REBUILD_H
#define OS_CORE_REMOTE_REBUILD_H

#include <unistd.h>

#include "log_structure.h"
#include "log_local.h"
#include "server_info.h"

/** set default server if "server_info.h not be include" **/
#ifndef LOG_SERVER_INFO
#define LOG_SERVER_INFO
#define LOG_REMOTE_SERVER_IP "xxx.xxx.xxx.xxx"
#define LOG_REMOTE_SERVER_PORT 20000
#endif

#define LOG_LOCAL_PORT 20218

#ifdef winPlatform  //windows
#include <winsock2.h>
#include <mmsystem.h>
#pragma comment(lib,"ws2_32.lib")
WORD sockVersion;
WSADATA wsaData;
/** basic func define **/
#define _core_socket_init() (sockVersion = MAKEWORD(2, 2), WSAStartup(sockVersion, &wsaData) == 0)
typedef SOCKET _core_socket_struct;
typedef HANDLE _core_thread;
#define _core_socket(family, type) socket(family, type, IPPROTO_TCP)
#define _core_INVALID_SOCKET INVALID_SOCKET
#define _core_SOCKET_ERROR SOCKET_ERROR
#define _core_sleep(seconds) Sleep(seconds * 1000)
#define _core_socket_close(sockHandle) closesocket(sockHandle)
typedef DWORD _core_thread_ret;
#define _core_thread_create(HANDLE, entry, arg) (HANDLE = CreateThread(NULL, 0, entry, arg, 0, NULL), !HANDLE)
#define _core_thread_join(HANDLE, RetRecv) (RetRecv = WaitForSingleObject(HANDLE, INFINITE), (RetRecv == 0xFFFFFFFF))
#define _core_thread_close(HANDLE) CloseHandle(HANDLE)
/** func name define **/
#define _core_log_remote_thread DWORD WINAPI _core_log_remote_service_thread(LPVOID lpParam)
#define _core_log_local_thread DWORD WINAPI _core_log_local_service_thread(LPVOID lpParam)

#else   //linux

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
/** socket don't need to init on linux **/
/** basic func define **/
#define _core_socket_init() true
typedef int _core_socket_struct;
typedef pthread_t _core_thread;
#define _core_socket(family, type) socket(family, type, 0)
#define _core_INVALID_SOCKET -1
#define _core_SOCKET_ERROR -1
#define _core_sleep(seconds) sleep(seconds)
#define _core_socket_close(sockHandle) close(sockHandle)
typedef void* _core_thread_ret;
#define _core_thread_create(HANDLE, entry, arg) pthread_create(&HANDLE, NULL, entry, arg)
#define _core_thread_join(HANDLE, RetRecv) pthread_join(HANDLE, &RetRecv)
#define _core_thread_close(HANDLE) pthread_cancel(HANDLE)
/** func name define **/
#define _core_log_remote_thread void* _core_log_remote_service_thread(void* lpParam)
#define _core_log_local_thread void* _core_log_local_service_thread(void* lpParam)

#endif

#define _REMOTE_INIT_CHECK(x) if(!_remote_log_sys.inited) return x
#define _REMOTE_EXIT_CHECK(x) if(_log_remote_exit_flag) return x

#define _REMOTE_CONNECT_ERROR "remote log server: connect failed. Retry after 10 seconds."
#define _REMOTE_CONNECT_SUCCESS "remote log server: connecting success."
#define _REMOTE_CONNECT_CLOSE "remote log server: module closed."
#define _REMOTE_LOCAL_SERVER_CLOSE "local log server: closed."

struct _core_log_event_node{
    char* string;
    struct _core_log_event_node* next;
};

typedef _core_log_event_node log_local_event;
typedef _core_log_event_node log_remote_event;

struct {
    log_local_event *local_message_list;
    log_remote_event *remote_message_list;
    uint32_t local_message_count, remote_message_count;
}_remote_log_info;

struct{
     _core_mutex list_mutex;
     _core_socket_struct log_server;
     _core_socket_struct local_server;
     _core_thread remote_service;
     _core_thread local_service;
     bool inited;
}_remote_log_sys;

int _log_remote_exit_flag = 0;
int _core_flag_socket_inited = 0;

/** get last node of link list **/
_core_log_event_node* _core_log_remote_message_list_get_last(struct _core_log_event_node* node){
    if(!node) return nullptr;
    _REMOTE_INIT_CHECK(nullptr);
    _core_mutex_lock(_remote_log_sys.list_mutex);
    while(node->next)
        node = node->next;
    _core_mutex_unlock(_remote_log_sys.list_mutex);
    return node;
}

/** destroy first message data now **/
void _core_log_remote_message_done(int Remote_orLocal){
    _REMOTE_INIT_CHECK();
    struct _core_log_event_node** operator_list;
    uint32_t* message_count;

    /** operation mode **/
    if(Remote_orLocal == 1){
        operator_list = &_remote_log_info.remote_message_list;
        message_count = &_remote_log_info.remote_message_count;
    }else if(!Remote_orLocal){
        operator_list = &_remote_log_info.local_message_list;
        message_count = &_remote_log_info.local_message_count;
    }else {   /** Remote_orLocal not equal to 0 or 1, it is denied **/
        return;
    }if(!(*operator_list))
        return;

    _core_mutex_lock(_remote_log_sys.list_mutex);
    /** adjust link list and list info **/
    struct _core_log_event_node* next_node = (*operator_list)->next;
    free((*operator_list)->string);
    *operator_list = next_node;
    (*message_count)--;
    _core_mutex_unlock(_remote_log_sys.list_mutex);
}

/** append log data to message list If Remote_orLocal equal to 1 then append to remote list **/
int _core_log_remote_message_append(struct _core_log_struct* log_data, int Remote_orLocal){
    if(!log_data || !_remote_log_sys.inited)
        return -1;
    _REMOTE_INIT_CHECK(-1);
    struct _core_log_event_node** operator_list, * last_node = 0, * alloc_tmp = 0;
    uint32_t* message_count;

    /** operation mode **/
    if(Remote_orLocal == 1) {
        operator_list = &_remote_log_info.remote_message_list;
        message_count = &_remote_log_info.remote_message_count;
    }else if(!Remote_orLocal) {
        operator_list = &_remote_log_info.local_message_list;
        message_count = &_remote_log_info.local_message_count;
    }else{   /** if Remote_orLocal is not 0 or 1 then finish them all **/
        if (_core_log_remote_message_append(log_data, 1)){
            _core_log_append("remote log module: can't append remote log", _core_log_date_get());
        }
        return _core_log_remote_message_append(log_data, 0);
    }

    _core_mutex_lock(_remote_log_sys.list_mutex);
    /** message memory data init **/
    alloc_tmp = (_core_log_event_node*)malloc(sizeof(_core_log_event_node));
    if(!alloc_tmp) {
        _core_mutex_unlock(_remote_log_sys.list_mutex);
        return -1;
    }
    alloc_tmp->next = 0;
    alloc_tmp->string = _core_log_local_data_merge(log_data);
    if(!alloc_tmp->string){
        free(alloc_tmp);
        _core_mutex_unlock(_remote_log_sys.list_mutex);
        return -1;
    }

    /** if last node exists **/
    if((last_node = _core_log_remote_message_list_get_last(*operator_list))){
        last_node->next = alloc_tmp;
    }else{  /** no any node exists **/
        *operator_list = alloc_tmp;
    }
    (*message_count)++;
    _core_mutex_unlock(_remote_log_sys.list_mutex);
    return 0;
}

/** universal send func **/
void _core_log_remote_send(_core_socket_struct client, int Remote_orLocal){
    _core_mutex_lock(_remote_log_sys.list_mutex);
    struct _core_log_event_node** operator_list;
    /** operation mode **/
    if(Remote_orLocal) {
        operator_list = &_remote_log_info.remote_message_list;
    }else{
        operator_list = &_remote_log_info.local_message_list;
    }
    if((*operator_list)){
        if((*operator_list)->string) {
            send(client, (*operator_list)->string, strnlen((*operator_list)->string, LOG_MAX_LEN), 0);
        }
    }
    _core_mutex_unlock(_remote_log_sys.list_mutex);
    _core_log_remote_message_done(!(!Remote_orLocal));
}

/** local log server for logcat(local mode) **/
_core_log_local_thread{
    /** server socket info **/
    _core_socket_struct sClient;
    struct sockaddr_in clientAddr, localAddr;
    int clientAddrLen = sizeof(clientAddr);

    int log_id_t = _core_log_append("local log server: initializing.", _core_log_date_get());
    _core_log_local_append(_core_log_get_by_id(log_id_t));

    /** socket info set **/
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(LOG_LOCAL_PORT);
    localAddr.sin_addr.S_un.S_addr = INADDR_ANY;

    /** socket init **/
    //_remote_log_sys.local_server = _core_socket(AF_INET, SOCK_STREAM);
    while((_remote_log_sys.local_server = _core_socket(AF_INET, SOCK_STREAM)) == _core_INVALID_SOCKET){
        log_id_t = _core_log_append("local log server socket init failed. Retry after 10 seconds.", _core_log_date_get());
        _core_log_local_append(_core_log_get_by_id(log_id_t));
        _core_sleep(10);
        _REMOTE_EXIT_CHECK(0);
        //_remote_log_sys.local_server = _core_socket(AF_INET, SOCK_STREAM);
    }
    log_id_t = _core_log_append("local log server socket init success.", _core_log_date_get());
    _core_log_local_append(_core_log_get_by_id(log_id_t));
    while(bind(_remote_log_sys.local_server, (struct sockaddr*)&localAddr, sizeof(localAddr)) == _core_SOCKET_ERROR){
        log_id_t = _core_log_append("local log server port bind failed. Retry after 10 seconds.", _core_log_date_get());
        _core_log_local_append(_core_log_get_by_id(log_id_t));
        _core_sleep(10);
        _REMOTE_EXIT_CHECK(0);
    }
    while(listen(_remote_log_sys.local_server, 3) == _core_SOCKET_ERROR){
        log_id_t = _core_log_append("local log server listen failed. Retry after 10 seconds.", _core_log_date_get());
        _core_log_local_append(_core_log_get_by_id(log_id_t));
        _core_sleep(10);
        _REMOTE_EXIT_CHECK(0);
    }
    log_id_t = _core_log_append("local log server started.", _core_log_date_get());
    _core_log_local_append(_core_log_get_by_id(log_id_t));

    /** main loop **/
    while(true){
        sClient = accept(_remote_log_sys.local_server, (SOCKADDR*)&clientAddr, &clientAddrLen);
        /** received a client, go to loop **/
        log_id_t = _core_log_append("local log server: client connected.", _core_log_date_get());
        _core_log_local_append(_core_log_get_by_id(log_id_t));

        while(true){
            /** check exit signal every circle **/
            if(_log_remote_exit_flag){
                _core_socket_close(_remote_log_sys.local_server);
                _remote_log_sys.local_server = _core_INVALID_SOCKET;
                log_id_t = _core_log_append(_REMOTE_LOCAL_SERVER_CLOSE, _core_log_date_get());
                _core_log_local_append(_core_log_get_by_id(log_id_t));
                _core_sleep(2);
                return 0;
            }
            /** check message not was send **/
            if(_remote_log_info.local_message_count > 0){
                _core_log_remote_send(sClient, 0);
            }
        }
    }
    return 0;
}

/** remote server connection thread **/
_core_log_remote_thread{
    uint32_t log_id_t = 0;
    log_id_t = _core_log_append("remote log server: initializing.", _core_log_date_get());
    _core_log_local_append(_core_log_get_by_id(log_id_t));
    /** Server info **/
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(LOG_REMOTE_SERVER_PORT);
    serverAddr.sin_addr.S_un.S_addr = inet_addr(LOG_REMOTE_SERVER_IP);

    /** init socket **/
    log_id_t = _core_log_append("remote log server: socket init.", _core_log_date_get());
    _core_log_local_append(_core_log_get_by_id(log_id_t));
    while ((_remote_log_sys.log_server = _core_socket(AF_INET, SOCK_STREAM)) == _core_INVALID_SOCKET){
        if(_log_remote_exit_flag)
            return 0;
        log_id_t = _core_log_append("remote log server: socket init failed. Retry after 10 seconds", _core_log_date_get());
        _core_log_local_append(_core_log_get_by_id(log_id_t));
        _core_sleep(10);
    }

    /** try to connect server **/
    while(_core_SOCKET_ERROR == connect(_remote_log_sys.log_server, (struct sockaddr*)&serverAddr, sizeof(serverAddr))){
        log_id_t = _core_log_append(_REMOTE_CONNECT_ERROR, _core_log_date_get());
        _core_log_local_append(_core_log_get_by_id(log_id_t));
        printf("%s\n", _REMOTE_CONNECT_ERROR);
        _core_sleep(10);
        if(_log_remote_exit_flag)
            return 0;
    }
    log_id_t = _core_log_append("remote log server: connected.", _core_log_date_get());
    _core_log_local_append(_core_log_get_by_id(log_id_t));
    _core_log_remote_message_append(_core_log_get_by_id(log_id_t), 2);

    while(true){
        /** check interrupt **/
        if(_log_remote_exit_flag){
            log_id_t = _core_log_append("remote log server: quit.", _core_log_date_get());
            _core_log_local_append(_core_log_get_by_id(log_id_t));
            _core_log_remote_message_append(_core_log_get_by_id(log_id_t), 2);
            _core_log_remote_send(_remote_log_sys.log_server, 1);
            _core_socket_close(_remote_log_sys.log_server);
            _remote_log_sys.log_server = _core_INVALID_SOCKET;
            return 0;
        }

        /** message post **/
        if(_remote_log_info.remote_message_count > 0){
            //debug
            //printf("sent: %s", _remote_log_info.remote_message_list->string);
            _core_log_remote_send(_remote_log_sys.log_server, 1);
        }
    }
    return 0;
};

int _core_log_remote_local_server_start(){
    _log_remote_exit_flag = 0;
    if(_remote_log_sys.local_service)
        return -1;
    if(_core_thread_create(_remote_log_sys.local_service, _core_log_local_service_thread, 0)){
        return -1;
    }
    return 0;
}

int _core_log_remote_server_connect(){
    _log_remote_exit_flag = 0;
    if(_remote_log_sys.remote_service)
        return -1;
    if(_core_thread_create(_remote_log_sys.remote_service, _core_log_remote_service_thread, 0)){
        return -1;
    }
    return 0;
}

int _core_log_remote_local_server_stop(){
    if(!_remote_log_sys.local_service){
        return -1;
    }
    _log_remote_exit_flag = 1;
    _core_thread_ret ret;
    return !_core_thread_close(_remote_log_sys.local_service);
}

int _core_log_remote_service_stop(){
    if(!_remote_log_sys.remote_service){
        return -1;
    }
    _log_remote_exit_flag = 1;
    _core_thread_ret ret;
    return _core_thread_join(_remote_log_sys.remote_service, ret);
}

int _core_log_socket_init(){
    if(_core_flag_socket_inited)
        return 1;
    _core_flag_socket_inited = _core_socket_init();
    _remote_log_sys.inited = true;
    return _core_flag_socket_inited;
}

#endif //OS_CORE_REMOTE_REBUILD_H

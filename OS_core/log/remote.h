/******************************************************************
 * log module, remote part
 * This module create a socket connection to net log catch client
 * And send log data to client by log service
 * :shinsya
 *****************************************************************/

#ifndef OS_CORE_LOG_REMOTE_H
#define OS_CORE_LOG_REMOTE_H

#define LOG_REMOTE_PORT 20206
#define LOG_REMOTE_INIT_FAILED_DATA "remote part of log module init failed."
#define LOG_REMOTE_BIND_FAILED_DATA "remote@log module: socket bind error."
#define LOG_REMOTE_LISTEN_FAILED_DATA "remote@log module: socket listen failed."
#define LOG_REMOTE_WAITING_DATA "remote@log module: waiting for client."
#define LOG_REMOTE_DISCONNECT_DATA "remote@log module: client disconnected."
#define LOG_REMOTE_BUFFER_BAD_ALLOC "remote@log module: bad buffer alloc, try again after 2 seconds."

#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include "structure.h"
#include "local.h"

struct _core_log_remote_event{
    struct _core_log_struct* data;
    struct _core_log_remote_event* last;
};

struct{
    SOCKET log_server = INVALID_SOCKET;
    unsigned long send = 0;
    HANDLE service = 0;
}remote_info;

DWORD WINAPI _core_log_remote_service_thread(LPVOID lpParam);
void _core_log_remote_event_done();

struct _core_log_remote_event* message_list = 0;
unsigned int message_count = 0;
HANDLE list_mutex;

int _core_log_remote_init(){
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    struct _core_log_date* tmp_date
    if(WSAStartup(sockVersion, &wsaData) != 0) return -1;

    //------------------------------init socket------------------------------------
    remote_info.log_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(remote_info.log_server == INVALID_SOCKET){
        tmp_date = _core_log_get_time();
        int tmp_log_id = _core_log_append(LOG_REMOTE_INIT_FAILED_DATA, tmp_date);
        _core_log_local_write(_core_log_get_by_id(tmp_log_id));
        if(tmp_date) free(tmp_date);
        return -2;
    }

    struct sockaddr_in sockAddrIn;
    sockAddrIn.sin_family = AF_INET;
    sockAddrIn.sin_port = htons(LOG_REMOTE_PORT);
    sockAddrIn.sin_addr.S_un.S_addr = INADDR_ANY;
    if(bind(remote_info.log_server, (LPSOCKADDR)&sockAddrIn, sizeof(sockAddrIn)) == SOCKET_ERROR){
        closesocket(remote_info.log_server);
        tmp_date = _core_log_get_time();
        int tmp_log_id = _core_log_append(LOG_REMOTE_BIND_FAILED_DATA, tmp_date);
        _core_log_local_write(_core_log_get_by_id(tmp_log_id));
        if(tmp_date) free(tmp_date);
        return -2;
    }

    if(listen(remote_info.log_server, 5) == SOCKET_ERROR){
        closesocket(remote_info.log_server);
        tmp_date = _core_log_get_time();
        int tmp_log_id = _core_log_append(LOG_REMOTE_LISTEN_FAILED_DATA, tmp_date);
        _core_log_local_write(_core_log_get_by_id(tmp_log_id));
        if(tmp_date) free(tmp_date);
        return -2;
    }
    //-------------------------------socket inited-------------------------------------

    list_mutex = CreateMutex(0,0,0);
    //start service thread
    remote_info.service = CreateThread(NULL, 0, _core_log_remote_service_thread, NULL, 0, NULL);
    return 0;
}

DWORD WINAPI _core_log_remote_service_thread(LPVOID lpParam){
    SOCKET sClient;
    struct sockaddr_in remoteAddr;
    int nAddrlen = sizeof(remoteAddr);

    struct tcp_info info;
    int if_len = sizeof(info);
    struct _core_log_date* date_tmp = 0;

    while(true){    //always waiting for client
        date_tmp = _core_log_get_time();
        int tmp_log_id = _core_log_append(LOG_REMOTE_WAITING_DATA, date_tmp);
        _core_log_local_write(_core_log_get_by_id(tmp_log_id));
        if(date_tmp){
            free(date_tmp);
            date_tmp = 0;
        }

        char* send_tmp = (char*)malloc(LOG_MAX_DETAILS_LEN);
        while(!send_tmp){       //alloc send buffer
            date_tmp = _core_log_get_time();
            tmp_log_id = _core_log_append(LOG_REMOTE_BUFFER_BAD_ALLOC, date_tmp);
            _core_log_local_write(_core_log_get_by_id(tmp_log_id));
            if(date_tmp){
                free(date_tmp);
                date_tmp = 0;
            }
            Sleep(2000);
            send_tmp = (char*)malloc(LOG_MAX_DETAILS_LEN);
        }

        sClient = accept(remote_info.log_server, (SOCKADDR * ) & remoteAddr, &nAddrlen);   //waiting for client

        while(true){
            if(sClient == INVALID_SOCKET){  //client not connect
                Sleep(400);
                continue;
            }
            getsockopt(sClient, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
            if(info.tcpi_state!=TCP_ESTABLISHED){       //client was disconnect
                date_tmp = _core_log_get_time();
                tmp_log_id = _core_log_append(LOG_REMOTE_DISCONNECT_DATA, date_tmp);
                _core_log_local_write(_core_log_get_by_id(tmp_log_id));
                if(date_tmp){
                    free(date_tmp);
                    date_tmp = 0;
                }
            }
            if(message_count > 0){      //do event list
                if(send_tmp){
                    memset(send_tmp, 0, LOG_MAX_DETAILS_LEN);
                    char* date_string_tmp = _core_log_get_time_string_except_ymd(message_list->data->log_date);
                    send_tmp[0] = '[';
                    memcpy_s(&send_tmp[1],8,date_string_tmp,strnlen_s(date_string_tmp,8));
                    send_tmp[9] = ']';
                    memcpy_s(&send_tmp[10], LOG_MAX_DETAILS_LEN - 10, message_list->data->log_details, strnlen_s(message_list->data->log_details, LOG_MAX_DETAILS_LEN));
                    send(sClient, send_tmp, strnlen_s(send_tmp, LOG_MAX_DETAILS_LEN), 0);
                    _core_log_remote_event_done();
                }
            }
            //TODO: create message event list
        }
        if(send_tmp){
            free(send_tmp);
            send_tmp = 0;
        }

    }
}

void _core_log_remote_event_append(struct _core_log_struct* log_data){      //append a new event
    if(!log_data)return;
    WaitForSingleObject(list_mutex, INFINITE);
    struct _core_log_remote_event* tmp_event = (_core_log_remote_event*)malloc(sizeof(_core_log_remote_event));
    if(tmp_event){
        tmp_event->data = log_data;
        tmp_event->last = message_list;
        message_list = tmp_event;
        message_count++;
    }
    ReleaseMutex(list_mutex);
    return;
}

void _core_log_remote_event_done(){     //last event had been done
    if(!message_list)return;
    WaitForSingleObject(list_mutex, INFINITE);
    struct _core_log_remote_event* tmp_event = message_list;
    message_list = message_list->last;
    free(tmp_event);
    message_count--;
    ReleaseMutex(list_mutex);
    return;
}

#endif //OS_CORE_LOG_REMOTE_H

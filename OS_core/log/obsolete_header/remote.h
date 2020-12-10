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
#define LOG_REMOTE_SERVER_THREAD "remote@log module: thread of connect to server was stated."
#define LOG_REMOTE_SERVER_IP "47.99.117.142"
#define LOG_REMOTE_SERVER_PORT 20215

//#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <Windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <time.h>

#include "structure.h"
#include "local.h"
//#include <WinSock2.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment( lib,"winmm.lib" )

struct _core_log_remote_event{
    struct _core_log_struct* data;
    struct _core_log_remote_event* last;
};

struct _core_log_server_event{
    struct _core_log_struct* data;
    struct _core_log_server_event* next;
};

struct{
    SOCKET log_server = INVALID_SOCKET;
    unsigned long send = 0;
    HANDLE service = 0;
    HANDLE remote_service = 0;
    SOCKET remote_server = INVALID_SOCKET;
}remote_info;

DWORD WINAPI _core_log_remote_service_thread(LPVOID lpParam);
void _core_log_remote_event_done();

struct _core_log_remote_event* message_list = 0;
struct _core_log_server_event* remote_message = 0;
unsigned int message_count = 0;
unsigned int remote_message_count = 0;
HANDLE list_mutex, exit_mutex;
int exit_flag = 0, client_flag = 0;

bool thread_exit_flag = false;


void _core_log_server_event_append(struct _core_log_struct* log_data){
    WaitForSingleObject(list_mutex, INFINITE);
    struct _core_log_server_event* tmp = (_core_log_server_event*)malloc(sizeof(_core_log_server_event));
    if(!tmp) return;
    tmp->next = 0;
    tmp->data = log_data;
    if(remote_message){
        struct _core_log_server_event* tmp2 = remote_message;
        while(tmp2->next){
            tmp2 = tmp2->next;
        }
        tmp2->next = tmp;
    }else{
        remote_message = tmp;
    }
    remote_message_count++;
    ReleaseMutex(list_mutex);
}

void _core_log_server_event_done(){
    if(!remote_message)return;
    WaitForSingleObject(list_mutex, INFINITE);
    struct _core_log_server_event* tmp = remote_message->next;
    free(remote_message);
    remote_message = tmp;
    remote_message_count--;
    ReleaseMutex(list_mutex);
}

DWORD WINAPI _core_log_server_init(LPVOID lpara){
    struct _core_log_date* tmp_date = _core_log_get_time();
    _core_log_append(LOG_REMOTE_SERVER_THREAD, tmp_date);
    if(tmp_date) free(tmp_date);
    remote_info.log_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    while (remote_info.log_server == INVALID_SOCKET){
        if(thread_exit_flag){
            thread_exit_flag = false;
            return 0;
        }
        printf("[remote log server] socket init failed. Retry after 10 seconds.\n");
        Sleep(10000);
        remote_info.log_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    struct sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(LOG_REMOTE_SERVER_PORT);
    serAddr.sin_addr.S_un.S_addr = inet_addr(LOG_REMOTE_SERVER_IP);

    int cn_status = connect(remote_info.log_server, (struct sockaddr *)&serAddr, sizeof(serAddr));
    while (cn_status == SOCKET_ERROR){
        if(thread_exit_flag){
            closesocket(remote_info.log_server);
            remote_info.log_server = INVALID_SOCKET;
            thread_exit_flag = false;
            return 0;
        }
        printf("Connect to remote log server failed. Retry after 10 seconds.\n");
        Sleep(10000);
        cn_status = connect(remote_info.log_server, (struct sockaddr *)&serAddr, sizeof(serAddr));
    }

    while(!thread_exit_flag){

        if(remote_message_count > 0){
            if(remote_message){
                WaitForSingleObject(list_mutex, INFINITE);
                send(remote_info.log_server, remote_message->data->log_details, strlen(remote_message->data->log_details), 0);
                ReleaseMutex(list_mutex);
                _core_log_server_event_done();
            }
        }
    }
    closesocket(remote_info.log_server);
    remote_info.log_server = INVALID_SOCKET;
    //CloseHandle(remote_info.remote_service);
    thread_exit_flag = false;
    return 0;
}

int _core_log_remote_init(){
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    struct _core_log_date* tmp_date;
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
    exit_mutex = CreateMutex(0,0,0);
    //start service thread
    remote_info.service = CreateThread(NULL, 0, _core_log_remote_service_thread, NULL, 0, NULL);

    //remote bridge server connection
    remote_info.remote_service = CreateThread(NULL, 0, _core_log_server_init, NULL, 0, NULL);

    return 0;
}

int _core_log_remote_service_stop(){    //if timeout return -1  (default 20s)
    //if (client_flag)CloseHandle(remote_info.service);
    CloseHandle(remote_info.service);
    if(WaitForSingleObject(exit_mutex, 20000) == WAIT_TIMEOUT){
        return -1;
    }else{
        DWORD start_t = timeGetTime();
        while(!exit_flag){
            Sleep(200);
            if(timeGetTime() - start_t > 10000){
                ReleaseMutex(exit_mutex);
                return -1;
            }
        }
        exit_flag = 0;
    }
    return 0;
}

bool IsSocketClosed(SOCKET clientSocket)
{
    bool ret = false;
    HANDLE closeEvent = WSACreateEvent();
    WSAEventSelect(clientSocket, closeEvent, FD_CLOSE);

    DWORD dwRet = WaitForSingleObject(closeEvent, 0);

    if (dwRet == WSA_WAIT_EVENT_0)
        ret = true;
    else if (dwRet == WSA_WAIT_TIMEOUT)
        ret = false;

    WSACloseEvent(closeEvent);
    return ret;
}

DWORD WINAPI _core_log_remote_service_thread(LPVOID lpParam){
    SOCKET sClient;
    struct sockaddr_in remoteAddr;
    int nAddrlen = sizeof(remoteAddr);

    //TODO: heart pack thread
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

        if(WaitForSingleObject(exit_mutex, 200) == WAIT_TIMEOUT){       //if stop signal when accept client
            if(remote_info.log_server != INVALID_SOCKET)closesocket(remote_info.log_server);
            remote_info.log_server = INVALID_SOCKET;
            remote_info.service = 0;
            free(send_tmp);
            exit_flag = 1;
            return 0;
        }else{
            ReleaseMutex(exit_mutex);
        }

        client_flag = 0;
        sClient = accept(remote_info.log_server, (SOCKADDR * ) & remoteAddr, &nAddrlen);   //waiting for client

        while(true){
            if (WaitForSingleObject(exit_mutex, 200) == WAIT_TIMEOUT) {       //if stop signal when linking
                if (remote_info.log_server != INVALID_SOCKET)
                    closesocket(remote_info.log_server);
                if (sClient != INVALID_SOCKET)
                    closesocket(sClient);
                remote_info.log_server = INVALID_SOCKET;
                remote_info.service = 0;
                free(send_tmp);
                exit_flag = 1;
                CloseHandle(remote_info.remote_service);
                return 0;
            }
            else {
                ReleaseMutex(exit_mutex);
            }

            if(sClient == INVALID_SOCKET){  //client not connect
                Sleep(400);
                continue;
            }

            client_flag = 1;
            if(message_count > 0){      //do event list
                if(send_tmp){
                    memset(send_tmp, 0, LOG_MAX_DETAILS_LEN);
                    char* date_string_tmp = _core_log_get_time_string_except_ymd(message_list->data->log_date);
                    send_tmp[0] = '[';
                    memcpy_s(&send_tmp[1],8,date_string_tmp,strnlen_s(date_string_tmp,8));
                    send_tmp[9] = ']';
                    memcpy_s(&send_tmp[10], LOG_MAX_DETAILS_LEN - 10, message_list->data->log_details, strnlen_s(message_list->data->log_details, LOG_MAX_DETAILS_LEN));
                    send_tmp[strnlen_s(send_tmp, LOG_MAX_DETAILS_LEN)] = '\n';
                    send(sClient, send_tmp, strnlen_s(send_tmp, LOG_MAX_DETAILS_LEN), 0);
                    _core_log_remote_event_done();
                    //debug
                    //printf("Remote data was send.\n");
                }
            }

        }
        if(send_tmp){
            free(send_tmp);
            send_tmp = 0;
        }

    }
}

void _core_log_remote_event_append(struct _core_log_struct* log_data){      //append a new event
    if (!log_data) { 
        //printf("remote event: nullptr.\n");
        return; 
    }
    WaitForSingleObject(list_mutex, INFINITE);
    struct _core_log_remote_event* tmp_event = (_core_log_remote_event*)malloc(sizeof(_core_log_remote_event));
    if(tmp_event){
        tmp_event->data = log_data;
        tmp_event->last = message_list;
        message_list = tmp_event;
        message_count++;
        //debug
        //printf("remote event added.\n");
    }
    else {
        //printf("remote event adding failed.\n");
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

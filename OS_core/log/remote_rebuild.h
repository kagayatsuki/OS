/******************************************************************
 * log module, remote part (rebuild ver)
 * This module create a socket connection to net log catch client
 * And send log data to client by log service
 * :shinsya
 *****************************************************************/

#ifndef OS_CORE_REMOTE_REBUILD_H
#define OS_CORE_REMOTE_REBUILD_H

#include <unistd.h>

#include "structure_rebuild.h"
#include "local_rebuild.h"
#include "server_info.h"

/** set default server if "server_info.h not be include" **/
#ifndef LOG_SERVER_INFO
#define LOG_SERVER_INFO
#define LOG_REMOTE_SERVER_IP "xxx.xxx.xxx.xxx"
#define LOG_REMOTE_SERVER_PORT 20000
#endif

#ifdef winPlatform  //windows
#include <winsock2.h>
#include <mmsystem.h>
#pragma comment(lib,"ws2_32.lib")
WORD sockVersion;
WSADATA wsaData;
/** basic func define **/
#define _core_socket_init() (sockVersion = MAKEWORD(2, 2), WSAStartup(sockVersion, &wsaData) == 0)
#define _core_socket_struct SOCKET
#define _core_socket(family, type) socket(family, type, IPPROTO_TCP)
#define _core_INVALID_SOCKET INVALID_SOCKET
#define _core_SOCKET_ERROR SOCKET_ERROR
#define _core_sleep(seconds) Sleep(seconds)
#define _core_socket_close(sockHandle) closesocket(sockHandle)
#define _core_thread_ret DWORD
#define _core_thread_create(HANDLE, entry, arg) (HANDLE = CreateThread(NULL, 0, entry, arg, 0, NULL), !HANDLE)
#define _core_thread_join(HANDLE, RetRecv) (RetRecv = WaitForSingleObject(HANDLE, INFINITE), (RetRecv == 0xFFFFFFFF))
#define _core_thread_close(HANDLE) CloseHandle(HANDLE)
/** func name define **/
#define _core_log_remote_thread DWORD WINAPI _core_log_remote_service_thread(LPVOID lpParam)

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
#define _core_socket_struct int
#define _core_socket(family, type) socket(family, type, 0)
#define _core_INVALID_SOCKET -1
#define _core_SOCKET_ERROR -1
#define _core_sleep(seconds) sleep(seconds)
#define _core_socket_close(sockHandle) close(sockHandle)
#define _core_thread_ret void*
#define _core_thread_create(HANDLE, entry, arg) pthread_create(&HANDLE, NULL, entry, arg)
#define _core_thread_join(HANDLE, RetRecv) pthread_join(HANDLE, &RetRecv)
#define _core_thread_close(HANDLE) pthread_cancel(HANDLE)
/** func name define **/
#define _core_log_remote_thread void* _core_log_remote_service_thread(void* lpParam)
#endif

struct _core_log_event_node{
    char* string;
    struct _core_log_event_node* next;
};

typedef _core_log_event_node log_local_event;
typedef _core_log_event_node log_remote_event;



#endif //OS_CORE_REMOTE_REBUILD_H

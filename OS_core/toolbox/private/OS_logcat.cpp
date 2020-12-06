//
// Created by shinsya on 2020/12/6.
//

#define LOG_REMOTE_SERVER_IP "47.99.117.142"
#define LOG_REMOTE_SERVER_PORT 20218

#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment( lib,"winmm.lib" )

int main(){
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    if(WSAStartup(sockVersion, &wsaData) != 0) {
        printf("Can not init Windows Socket.\n");
        getchar();
        return 0;
    }
    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    while(server == INVALID_SOCKET){
        printf("Socket create failed. Retry after 10 seconds.\n");
        Sleep(10000);
        server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    struct sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(LOG_REMOTE_SERVER_PORT);
    serAddr.sin_addr.S_un.S_addr = inet_addr(LOG_REMOTE_SERVER_IP);

    int cn_status = connect(server, (struct sockaddr *)&serAddr, sizeof(serAddr));
    while (cn_status == SOCKET_ERROR){
        printf("Connect to remote log server failed. Retry after 10 seconds.\n");
        Sleep(10000);
        cn_status = connect(server, (struct sockaddr *)&serAddr, sizeof(serAddr));
    }
    char buffer[1024] = {0};
    int nlen = recv(server, buffer, 1023, 0);
    printf("%s\n", buffer);
    while (true){
        memset(buffer, 0, 1024);
        send(server, "gets", 4, 0);
        nlen = recv(server, buffer, 1023, 0);
        //printf("debug: received.\n");
        if((errno != EINTR) && (nlen <= 0)){
            closesocket(server);
            printf("Server losed.\n");
            getchar();
            return 0;
        }
        printf("[log] %s\n", buffer);
        Sleep(20);
    }
    return 0;
}

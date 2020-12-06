// This program run as a bridge to exchange log data between client and local server
// Created by shinsya on 2020/12/6.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#define _MAX_LOG_DATA_LEN 1024
#define _MAX_MESSAGE 3000
#define _RETRY_TIME 10
#define _LISTEN_PORT_BRIDGE 20215
#define _LISTEN_CLIENT_PORT 20218

struct{
    int _bridge_sock = 0, _bridge_connection = 0;
    int _client_sock = 0, _client_connection = 0;
    int _client_visit_count = 0;
}server_info;

struct _message{
    char* _data;
    struct _message* next;
};

void _message_free_first();
void _message_list_append(char* message);

struct _message* _message_list = 0;
unsigned int _message_count = 0;
unsigned long _message_appended = 0;

pthread_t thread_server, thread_client;
pthread_mutex_t list_lock = PTHREAD_MUTEX_INITIALIZER;

bool exit_bridge = false;

void* _server_receive(void*){
    server_info._bridge_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(server_info._bridge_sock<0){
        printf("[bridge_receive]Receive socket create failed.\n");
        printf("[bridge_receive]Retry after %d seconds.\n", _RETRY_TIME);
        while(server_info._bridge_sock<0){
            sleep(_RETRY_TIME);
            server_info._bridge_sock = socket(PF_INET, SOCK_STREAM, 0);
        }
    }
    //server_info._client_sock = server_info._bridge_sock;
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(_LISTEN_PORT_BRIDGE);
    serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
    while(bind(server_info._bridge_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        printf("[bridge_receive]Bind port to socket failed.\n");
        printf("[bridge_receive]Retry after %d seconds.\n", _RETRY_TIME);
        sleep(_RETRY_TIME);
    }
    printf("[bridge_receive]Bind port success.\n");
    while(listen(server_info._bridge_sock,2) < 0){
        printf("[bridge_receive]Listen to socket failed.\n");
        printf("[bridge_receive]Retry after %d seconds.\n", _RETRY_TIME);
        sleep(_RETRY_TIME);
    }
    printf("[bridge_receive]Listen to socket success.\n");
    printf("[bridge_receive]Socket init success.\n");
    //Waiting for local machine
    struct sockaddr_in client_addr;
    socklen_t client_addrlength = sizeof(client_addr);

    char buffer[_MAX_LOG_DATA_LEN] = {0};
    bool flag_exit = false;
    while(true){
        server_info._bridge_connection = accept(server_info._bridge_sock, (struct sockaddr*)&client_addr, &client_addrlength);
        if(server_info._bridge_connection >= 0){    //local machine was connected
            _message_list_append("Local machine was connected");
            printf("Local machine was connected\n");
            flag_exit = false;
            bool debug_exit = false;
            while(!flag_exit){
                memset(buffer,0,_MAX_LOG_DATA_LEN);
                int data_len = recv(server_info._bridge_connection, buffer, _MAX_LOG_DATA_LEN-1, NULL);
                if((errno != EINTR) && (data_len <= 0)){
                    flag_exit = true;
                    close(server_info._bridge_connection);
                }else{
                    if(strncmp("exit", buffer, strlen(buffer))==0){
                        flag_exit = true;
                        close(server_info._bridge_connection);
                        server_info._bridge_connection = -1;
                    }
                    else if(strncmp("[order]_close_remote_log", buffer, strlen(buffer)) == 0){
                        flag_exit = true;
                        close(server_info._bridge_connection);
                        server_info._bridge_connection = -1;
                        debug_exit = true;
                    }
                    else{
                        printf("[log] %s\n", buffer);
                        _message_list_append(buffer);
                    }
                }
            }
            if(debug_exit){
                close(server_info._bridge_sock);
                break;
            }
        }
    }
    
    exit_bridge = true;
    pthread_cancel(thread_server);
    pthread_join(thread_client, NULL);
    printf("Bridge closed.\n");
    return 0;
}

void itoa(int num, char* buff){
    int tmp = num;
    char tmp2[9] = {0};
    tmp2[0] = '0';
    int i = 0;
    while(tmp % 10){
        tmp2[i] = tmp % 10 + '0';
        i++;
        tmp%=10;
    }
    for(int a = 0; a < i; a++){
        buff[a] = tmp2[i-a-1];
    }
}

void* _server_client(void*){
    //socket init
    server_info._client_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(server_info._client_sock<0){
        printf("[bridge_client]Client socket create failed.\n");
        printf("[bridge_client]Retry after %d seconds.\n", _RETRY_TIME);
        while(server_info._client_sock<0){
            sleep(_RETRY_TIME);
            server_info._client_sock = socket(PF_INET, SOCK_STREAM, 0);
        }
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(_LISTEN_CLIENT_PORT);
    serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
    while(bind(server_info._client_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        printf("[bridge_client]Bind port to socket failed.\n");
        printf("[bridge_client]Retry after %d seconds.\n", _RETRY_TIME);
        sleep(_RETRY_TIME);
    }
    printf("[bridge_client]Bind port success.\n");
    while(listen(server_info._client_sock,2) < 0){
        printf("[bridge_client]Listen to socket failed.\n");
        printf("[bridge_client]Retry after %d seconds.\n", _RETRY_TIME);
        sleep(_RETRY_TIME);
    }
    printf("[bridge_client]Listen to socket success.\n");
    printf("[bridge_client]Socket init success.\n");
    //Waiting for local machine
    struct sockaddr_in client_addr;
    socklen_t client_addrlength = sizeof(client_addr);
    
    //bool client_online = false;
    char buffer[512] = {0};
    while(true){
        //struct _message* tmp = 0;
        if(exit_bridge){
            break;
        }
        
        server_info._client_connection = accept(server_info._client_sock, (struct sockaddr*)&client_addr, &client_addrlength);
        if(server_info._client_connection >= 0){    //向新连入客户端汇报以往被连入次数
            printf("Client was connected.\n");
            server_info._client_visit_count++;
            memcpy(buffer, "Welcome to remote log server.", 29);
            //itoa(server_info._client_visit_count, &buffer[20]);
            //memcpy(&buffer[20]+strlen(&buffer[20]), " client(s)", 10);
            send(server_info._client_connection, buffer, strlen(buffer), NULL);
        }
        printf("Client loop start.\n");
        while(true){
            if(exit_bridge){
                if(server_info._client_connection >= 0){
                    close(server_info._client_sock);
                    close(server_info._client_connection);
                }
                break;
            }
            if(_message_count > 0) {
                if (server_info._client_connection >= 0) {    //客户端连线时，传输数据
                    memset(buffer, 0, 512);
                    int len = recv(server_info._client_connection, buffer, 511, NULL);
                    if ((errno != EINTR) && (len <= 0)) {
                        close(server_info._client_connection);
                        server_info._client_connection = -1;
                        break;
                    } else if (strncmp("gets", buffer, strlen(buffer)) == 0) {
                        //printf("Client send a gets command\n");
                        send(server_info._client_connection, _message_list->_data, strlen(_message_list->_data), NULL);
                        _message_free_first();

                    }
                }
            }
            
        }
        
    }
    return 0;
}

void _message_free_first(){
    pthread_mutex_lock(&list_lock);
    struct _message* tmp = _message_list;
    if(tmp){
        _message_list = tmp->next;
        if(tmp->_data)free(tmp->_data);
        free(tmp);
        _message_count--;
    }
    pthread_mutex_unlock(&list_lock);
}

void _message_list_append(char* message){
    struct _message* tmp =  (_message*)malloc(sizeof(_message));
    if(!tmp)return;
    pthread_mutex_lock(&list_lock);
    //初始化信息
    tmp->_data = (char*)malloc(strlen(message)+1);
    if(tmp->_data){
        tmp->_data[strlen(message)] = '\0';
        memcpy(tmp->_data, message, strlen(message));
    }
    tmp->next = 0;

    if(_message_list){  //向最后插入信息
        struct _message* tmpstruct = _message_list;
        while(tmpstruct->next){
            tmpstruct = tmpstruct->next;
        }
        tmpstruct->next = tmp;
    }else{
        _message_list = tmp;
    }

    if(_message_count >= _MAX_MESSAGE)  //判断信息总量是否达到限制，如果达到则删除第一条信息
        _message_free_first();
    _message_count++;
    _message_appended++;
    pthread_mutex_unlock(&list_lock);
}

int main(){
    int ret = 0;
    ret = pthread_create(&thread_server, NULL, _server_receive, NULL);
    if(ret != 0){
        printf("Receive server start failed.\n");
        return 0;
    }
    ret = pthread_create(&thread_client, NULL, _server_client, NULL);
    if(ret != 0){
        printf("Client server start failed.\n");
    }
    pthread_join(thread_server, NULL);

    return 0;
}

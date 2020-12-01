//
// Created by shinsya on 2020/12/1.
//

#include <stdio.h>
#include "service_log.h"

int main(){
    int tmp_service_start = _core_service_log_start(1);
    if(tmp_service_start == -1){
        printf("Log service start failed.\n");
        Sleep(3000);
        return 0;
    }else if(tmp_service_start == 1){
        printf("Log service: local module init failed.\n");
        Sleep(3000);
    }else if(tmp_service_start == 2){
        printf("Log service: remote module init failed.\n");
        Sleep(3000);
    }else{
        printf("Log service: Started.\n");
    }

    char tmp_input[512] = {0};
    int exit_t = 0;
    while(!exit_t){
        memset(tmp_input, 0, 512);
        gets_s(tmp_input, 512);
        if(tmp_input[0] != '\0') {
            if(!strncmp(tmp_input, "exit", strnlen_s(tmp_input, 512))){
                exit_t = 1;
                int tmp_service_stop = _core_service_log_stop();
                printf("Service stop: %d\n", tmp_service_stop);
                Sleep(3000);
                continue;
            }else{
                _core_service_log_print(tmp_input);
                continue;
            }
        }
    }

    return 0;
}

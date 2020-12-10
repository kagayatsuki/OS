//
// Created by shinsya on 2020/12/1.
//

#include <stdio.h>
//#include "service_log.h"
#include "logservice.h"

int main(){
    if(_core_service_log_start()){
        printf("Log service start failed.\n");
        Sleep(3000);
        return 0;
    }else{
        printf("Log service: Started.\n");
    }

    char tmp_input[512] = {0};
    int exit_t = 0;
    while(!exit_t){
        memset(tmp_input, 0, 512);

        gets(tmp_input);
        if(tmp_input[0] != '\0') {
            if(!strncmp(tmp_input, "exit", strlen(tmp_input))){
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

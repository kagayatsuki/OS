/************************************************
* log module, local part
* This module just for logs saving to local file
* Log module for net should be a other header file
* :shinsya
************************************************/
#ifndef LOG_LOCAL
#define LOG_LOCAL

#define LOG_DATA_ENTER "\r\n"
#define LOG_DATA_BRACKET "["
#define LOG_DATA_BRACKET_RIGHT "]"


#include <time.h>
#include <stdio.h>
#include "structure.h"

struct {
    FILE *local = nullptr;
    unsigned long log_count = 0;
}log_local_info;

int _core_log_local_init(){     //initialize local log sys, return -1 when sys had been inited, return -2 when bad alloc
    if(log_local_info.local)return -1;
    char tmp_filename[36];
    memset(tmp_filename, 0, 36);

    struct _core_log_date* tmp_date = _core_log_get_time();
    if(!tmp_date)return -2;
    char* tmp_date_string = _core_log_get_time_string(tmp_date);
    if(!tmp_date_string){
        free(tmp_date);
        return -2;
    }
    char* tmp_date_string_exc = _core_log_get_time_string_except_ymd(tmp_date);

    memcpy_s(tmp_filename, 36, "./core_", 7);
    memcpy_s(&tmp_filename[7], 20, tmp_date_string, strnlen_s(tmp_date_string, 20));
    tmp_filename[26] = '\0';
    free(tmp_date);
    free(tmp_date_string);

    //printf("log file: %s\n", tmp_filename);
    errno_t tmp_errno = fopen_s(&log_local_info.local, tmp_filename, "w+");
    if(tmp_errno)return -2;
    if (!log_local_info.local) return -2;

    fseek(log_local_info.local, 0, SEEK_SET);
    fwrite("Log sys was inited in ", 22, 1, log_local_info.local);
    fwrite(tmp_date_string_exc, strnlen_s(tmp_date_string_exc, 9), 1, log_local_info.local);
    fwrite(LOG_DATA_ENTER, 2, 1, log_local_info.local);
    if(tmp_date_string_exc) free(tmp_date_string_exc);
    return 0;
}

void _core_log_local_close(){       //close opened log file now
    if(!log_local_info.local)return;
    fclose(log_local_info.local);
    log_local_info.local = 0;
    log_local_info.log_count = 0;
    return;
}

void _core_log_local_write(struct _core_log_struct* log_data){      //write log data to local
    if(!log_local_info.local)return;
    if(!log_data)return;
    char* tmp_date_string = _core_log_get_time_string_except_ymd(log_data->log_date);
    fwrite(LOG_DATA_BRACKET, 1, 1, log_local_info.local);
    fwrite(tmp_date_string, strnlen_s(tmp_date_string, 8), 1, log_local_info.local);
    fwrite(LOG_DATA_BRACKET_RIGHT, 1, 1, log_local_info.local);
    fwrite(log_data->log_details, strnlen_s(log_data->log_details, LOG_MAX_DETAILS_LEN), 1, log_local_info.local);
    fwrite(LOG_DATA_ENTER, 2, 1, log_local_info.local);
    if(tmp_date_string) free(tmp_date_string);
    log_local_info.log_count++;
    return;
}

#endif // 

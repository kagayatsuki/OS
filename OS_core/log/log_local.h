/*******************************************
 * Log system file managing
 * Rebuild ver
 * This module just for logs saving to local file
 * Log module for net should be a other header file
 * :shinsya
 *******************************************/

#ifndef LOG_LOCAL
#define LOG_LOCAL

#include <stdio.h>
#include "log_structure.h"

#define LOG_DATA_NEXT_LINE '\n'
#define LOG_DATE_BRACKET '['
#define LOG_DATE_BRACKET_RIGHT ']'
#define LOG_LOCAL_PATH_MAX_LEN 64
#define LOG_LOCAL_PATH "./sys/log/"
#define LOG_LOCAL_MODULE_LOADING "[log local module] initializing.\n"
#define LOG_LOCAL_MODULE_INIT_FAILED "[log local module] initialize failed.\n"
#define LOG_LOCAL_MODULE_INIT_DONE "[log local module] done.\n"
#define LOG_LOCAL_FIRST_LINE "This system log file created in "

struct {
    FILE *file;
    uint32_t log_count;
}_core_log_local_info;

bool echo_mode = false;

void _core_log_set_echo_mode(bool mode){echo_mode = mode;}

/** Initialize log local module, if init success return 0 **/
int _core_log_local_init(){
    if(_core_log_local_info.file)   /** it was inited and unclosed **/
        return -1;
    _core_log_local_info.file = nullptr;
    _core_log_local_info.log_count = 0;
    char tmp_filename[128] = {0};
    struct _core_log_date* tmp_date = _core_log_date_get();
    printf(LOG_LOCAL_MODULE_LOADING);
    /** merge local filename **/
    memcpy(tmp_filename, LOG_LOCAL_PATH, strnlen(LOG_LOCAL_PATH, LOG_LOCAL_PATH_MAX_LEN));
    _core_log_date_string_get(tmp_date, &tmp_filename[strnlen(tmp_filename, 128)]);

    /** try to open file **/
    FILE* tmp_file = fopen(tmp_filename, "w+");
    if(tmp_file == NULL){
        if(tmp_date) free(tmp_date);
        printf(LOG_LOCAL_MODULE_INIT_FAILED);
        return -1;
    }
    _core_log_local_info.file = tmp_file;

    /** write the first string **/
    char tmp_create_time[32] = {0};
    _core_log_date_string_get_exceptYMD(tmp_date, tmp_create_time);
    tmp_create_time[strnlen(tmp_create_time, 32)] = LOG_DATA_NEXT_LINE;
    fseek(_core_log_local_info.file, 0, SEEK_SET);
    fwrite(LOG_LOCAL_FIRST_LINE, strnlen(LOG_LOCAL_FIRST_LINE, 64), 1, _core_log_local_info.file);
    fwrite(tmp_create_time, strnlen(tmp_create_time, 32), 1, _core_log_local_info.file);

    /** release useless memory and notify init process **/
    if(tmp_date) free(tmp_date);
    printf(LOG_LOCAL_MODULE_INIT_DONE);
    return 0;
}

/** return string like "[xx:xx:xx] log detail" that merged log detail and date (h:m:s)
 this pointer should be free when it was used **/
char* _core_log_local_data_merge(struct _core_log_struct* log_data){
    if(!log_data)
        return nullptr;
    int alloc_len = strnlen(log_data->log_details, LOG_MAX_LEN) + 16;
    char* merged_string = (char*)malloc(alloc_len);
    if(!merged_string)
        return nullptr;
    memset(merged_string, 0, alloc_len);
    merged_string[0] = '[';
    _core_log_date_string_get_exceptYMD(log_data->log_date, merged_string + 1);
    merged_string[9] = ']';
    merged_string[10] = ' ';
    memcpy(&merged_string[11], log_data->log_details, strlen(log_data->log_details));
    merged_string[strnlen(merged_string, alloc_len)] = LOG_DATA_NEXT_LINE;
    //debug:
    //printf("merge string: %s", merged_string);
    return merged_string;
}

/** close opened log local file **/
int _core_log_local_close(){
    if(!_core_log_local_info.file)
        return -1;
    fclose(_core_log_local_info.file);
    _core_log_local_info.log_count = 0;
    _core_log_local_info.file = 0;
    return 0;
}

/** append log string to local log file **/
int _core_log_local_append(struct _core_log_struct* log_data){
    if(!_core_log_local_info.file)
        return -1;
    if(!log_data)
        return -1;
    /** get merged log string **/
    char* tmp_string = _core_log_local_data_merge(log_data);
    if(!tmp_string)
        return -1;
    fwrite(tmp_string, strnlen(tmp_string, LOG_MAX_LEN + 13), 1, _core_log_local_info.file);
    if(echo_mode)
        printf("%s", tmp_string);
    free(tmp_string);
    _core_log_local_info.log_count++;
    return 0;
}

#endif //LOG_LOCAL

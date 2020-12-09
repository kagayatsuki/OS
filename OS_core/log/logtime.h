//
// Created by shinsya on 2020/12/9.
//

#ifndef CORE_LOG_TIME
#define CORE_LOG_TIME

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef uint32_t
#define uint32_t unsigned int
#endif

#ifndef uint16_t
#define uint16_t unsigned short
#endif

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#define LOG_DATE_EXCEPTION "[log date] Warning: can not get system date.\n"

//date structure
struct _core_log_date{
    uint16_t year;
    uint8_t month, day, hour, minute, second;
};

_core_log_date* _core_log_date_get(){
    long tm_tmp = time(NULL);
    struct tm *lt;

    struct _core_log_date* tmp = (_core_log_date*)malloc(sizeof(_core_log_date));
    if(!tmp) {
        printf(LOG_DATE_EXCEPTION);
        return 0;
    }
    lt=localtime(&t);
    tmp->year = (uint16_t)lt->tm_year;
    tmp->month = (uint8_t)lt->tm_mon;
    tmp->day = (uint8_t)lt->tm_mday;
    tmp->hour = (uint8_t)lt->tm_hour;
    tmp->minute = (uint8_t)lt->tm_min;
    tmp->second = (uint8_t)lt->tm_sec;

    return tmp;
}

//it's not safe that twin of fun
int _core_log_date_string_get(struct _core_log_date* date, char* buffer){
    if(!date) return -1;

    int offset = 0;
    int bp[4] = {1000, 100, 10, 1};
    for(;offset < 4; offset++) {
        buffer[offset] = date->year / bp[offset] % 10 + '0';
    }
    buffer[offset++] = '-';
    buffer[offset++] = date->month / 10 + '0';
    buffer[offset++] = date->month % 10 + '0';
    buffer[offset++] = '-'
    buffer[offset++] = date->day / 10 + '0';
    buffer[offset++] = date->day % 10 + '0';
    buffer[offset++] = ' ';
    buffer[offset++] = date->hour / 10 + '0';
    buffer[offset++] = date->hour % 10 + '0';
    buffer[offset++] = '-';
    buffer[offset++] = date->minute / 10 + '0';
    buffer[offset++] = date->minute % 10 + '0';
    buffer[offset++] = '-';
    buffer[offset++] = date->second / 10 + '0';
    buffer[offset++] = date->second % 10 + '0';
    return 0;
}

int _core_log_date_string_get_exceptYMD(struct _core_log_date* date, char* buffer){
    if(!date) return -1;
    int offset = 0;
    buffer[offset++] = date->hour / 10 + '0';
    buffer[offset++] = date->hour % 10 + '0';
    buffer[offset++] = ':';
    buffer[offset++] = date->minute / 10 + '0';
    buffer[offset++] = date->minute % 10 + '0';
    buffer[offset++] = ':';
    buffer[offset++] = date->second / 10 + '0';
    buffer[offset++] = date->second % 10 + '0';
    return 0;
}

#endif //CORE_LOG_TIME

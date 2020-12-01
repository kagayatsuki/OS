/**************************************
* Log structure and link list
* :shinsya
**************************************/
#ifndef LOG_STRUCTURE
#define LOG_STRUCTURE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <time.h>
#include <math.h>
#include <errno.h>

#ifndef nullptr
#define nullptr 0
#endif

/*
#ifdef strnlen_s
#define strnlen_s(a,b) strnlen(a,b)
#endif

#ifndef memcpy_s
#define memcpy_s(a,b,c,d) memcpy(a,c,b)
#endif
*/

#define LOG_MAX_LIMIT 1000
#define LOG_MAX_DETAILS_LEN 256

using namespace std;

struct _core_log_date
{
	int year;
    int month;
	int day;
	int hour;
	int minute;
	int second;
};

struct _core_log_struct		//data of log
{
	char* log_details;
	struct _core_log_date* log_date;
};

struct _core_log		//log list structure
{
	int log_id;
	struct _core_log_struct* log_struct;

	struct _core_log* last;
};


struct _core_log* _var_log_list = nullptr;
unsigned int _var_log_count = 0, _var_log_id = 0;

_core_log* _core_log_get_first(){   //for free first log data when appending new log that log count bigger than max
    if(!_var_log_list)return nullptr;
    struct _core_log* tmp = _var_log_list;
    while (tmp->last){
        tmp = tmp->last;
    }
    return tmp;
}

unsigned int _core_log_append(char* log_details, _core_log_date* log_date) {    //append new log then return log id
	if ((!log_details) || (!log_date))return 0;

	struct _core_log* tmp_link = (_core_log*)malloc(sizeof(_core_log)); //log info structure
	if (!tmp_link)return 0;

	memset(tmp_link, 0, sizeof(_core_log));
	tmp_link->log_id = _var_log_id + 1;
	tmp_link->log_struct = (_core_log_struct*)malloc(sizeof(_core_log_struct));   //log data
	if (!tmp_link->log_struct) {
		free(tmp_link);
		return 0;
	}
	memset(tmp_link->log_struct, 0, sizeof(_core_log_struct));
	int detail_len = strlen(log_details);   //, LOG_MAX_DETAILS_LEN);
	tmp_link->log_struct->log_details = (char*)malloc(detail_len + 1);  //log details data
	if (!tmp_link->log_struct->log_details) {   //log data can't be null
		free(tmp_link->log_struct);
		free(tmp_link);
		return 0;
	}

	memcpy_s(tmp_link->log_struct->log_details, detail_len + 1, log_details, detail_len);
	tmp_link->log_struct->log_details[detail_len] = '\0';

	tmp_link->log_struct->log_date = (_core_log_date*)malloc(sizeof(_core_log_date));   //log date data structure
    if (tmp_link->log_struct->log_date && log_date) //log date can be null that for bad alloc sometime
        memcpy_s(tmp_link->log_struct->log_date, sizeof(_core_log_date), log_date, sizeof(_core_log_date));
	
    
    //set link list
	if(_var_log_list){  //is it first?
	    if(_var_log_count + 1 > LOG_MAX_LIMIT){ //log count will bigger than max, free the first
	        struct _core_log* tmp_first = _core_log_get_first();
	        if(tmp_first){
	            if(tmp_first->log_struct->log_details)free(tmp_first->log_struct->log_details);
	            if(tmp_first->log_struct->log_date)free(tmp_first->log_struct->log_date);
	            free(tmp_first->log_struct);
	            free(tmp_first);
	        }
	        tmp_link->last = _var_log_list;
        }
        else {
            _var_log_count++;
        }

	}else{ //it is first
	    _var_log_count++;
	}
    _var_log_list = tmp_link;
	_var_log_id++;
	return tmp_link->log_id;
}

unsigned int _core_log_append(const char* log_details, _core_log_date* log_date){   //const type declared
    return _core_log_append((char*)log_details, log_date);
}

void _core_log_clean_all(){     //delete all log data in memory
    struct _core_log* tmp = _var_log_list, *tmp2;
    while (tmp){
        tmp2 = tmp;
        tmp = tmp->last;
        if(tmp2->log_struct->log_date)free(tmp2->log_struct->log_date);
        if(tmp2->log_struct->log_details)free(tmp2->log_struct->log_details);
        free(tmp2->log_struct);
        free(tmp2);
    }
    _var_log_count = 0;
}

_core_log_struct* _core_log_get_by_id(unsigned int log_id){     //get log data structure pointer
    struct _core_log* tmp = _var_log_list;
    while (tmp){
        if(tmp->log_id == log_id)
            return tmp->log_struct;
        tmp = tmp->last;
    }
    return nullptr;
}

unsigned int _core_log_count_get(){     //get log count
    return _var_log_count;
}

_core_log_date* _core_log_get_time(){   //get date data now
    struct tm tmp_time;
    time_t gt;
    //get time
    gt = time(NULL);
    errno_t er_t = localtime_s(&tmp_time, &gt);

    if(er_t)return 0;

    struct _core_log_date* tmp = (_core_log_date*)malloc(sizeof(_core_log_date));
    if(!tmp)return 0;
    
    tmp->year=tmp_time.tm_year + 1900;
    tmp->day=tmp_time.tm_mday;
    tmp->hour=tmp_time.tm_hour;
    tmp->minute=tmp_time.tm_min;
    tmp->month=tmp_time.tm_mon + 1;
    tmp->second=tmp_time.tm_sec;

    return tmp;
}

char* _core_log_get_time_string(struct _core_log_date* tm){     //transmit date data to string by the format
    if(!tm)return 0;
    char* tmp = (char*)malloc(20);
    if(!tmp)return 0;

    memcpy_s(tmp, 20, "0000-00-00-00_00_00", 19);
    
    for(int i1 = 0; i1 < 4; i1++){    //get year string
        tmp[i1] = tm->year/(int)pow(10,3-i1)%10 + '0';
        //putchar(tmp[i1]);
    }
  
    int offset = 5;
    for(int i2 = 0; i2 < 5; i2++){      //get month, day, hour, minute, second string
        tmp[offset] = *((int*)tm + 1 + i2) / 10 + '0';
        tmp[offset + 1] = *((int*)tm + 1 + i2) % 10 + '0';
        offset+=3;
    }
    return tmp;
}

int _core_toolbox_get_any_bit(int num, int bit) {
    return (num - num / (int)pow(10, bit) * (int)pow(10, bit)) / (int)pow(10, bit - 1) % 10;
}

char* _core_log_get_time_string_except_ymd(struct _core_log_date* tm){       //transmit date data that excepted year and month and day to string
    if(!tm)return 0;
    char* tmp = (char*)malloc(9);
    if(!tmp)return 0;
    memset(tmp,'_',8);  //format "00_00_00"
    tmp[8] = '\0';
    int offset = 0;
    for(int i = 0; i < 3; i++){
        tmp[offset] = _core_toolbox_get_any_bit(*((int*)tm + 3 + i), 2) + '0';
        tmp[offset + 1] = _core_toolbox_get_any_bit(*((int*)tm + 3 + i), 1) + '0';
        offset += 3;
    }
    //debug
    //printf("time string %s\n", tmp);
    return tmp;
}

#endif // !LOG_STRUCTURE


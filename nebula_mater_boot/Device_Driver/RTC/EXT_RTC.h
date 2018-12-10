#ifndef _EXT_RTC_H
#define _EXT_RTC_H

#include "stdint.h"
#include "config.h"
typedef struct  
{
    uint8_t note_time_year;
    uint8_t note_time_month;
    uint8_t note_time_day;
    uint8_t note_time_hour;
    uint8_t note_time_min;
} st_RTC_info;
uint8_t rtc_hw_check(void);
void write_rtc_data(st_RTC_info * rtc_time);
void read_rtc_data(st_RTC_info * rtc_time);


#endif


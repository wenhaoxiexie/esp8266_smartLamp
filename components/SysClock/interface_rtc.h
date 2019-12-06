//SoftwareRTC
#ifndef __INTERFACE_RTC_H
#define __INTERFACE_RTC_H

#include "stdint.h"
#include "SoftwareRTC.h"

void write_rtc(int year,uint8_t month,uint8_t day,uint8_t hour,uint8_t min,uint8_t sec);
void refresh_rtc(void);
void rtc_set_default_time(void);
void rtc_utc_time_add(void);

#endif 


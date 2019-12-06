#include "interface_rtc.h"
#include "esp_log.h"
#include "esp_timer.h"

#define        rtc_tag           "rtc_time"

int utcSeconds=0;

void refresh_rtc(void)
{
    ConvertToUTCTime(&STU_UTC_time,utcSeconds);

	ESP_LOGI(rtc_tag, "%d-%d-%d %d:%d:%d \n",STU_UTC_time.year,STU_UTC_time.month,
											 STU_UTC_time.day,STU_UTC_time.hour,
											 STU_UTC_time.minutes,STU_UTC_time.seconds
											 );
	
}

void write_rtc(int year,uint8_t month,uint8_t day,uint8_t hour,uint8_t min,uint8_t sec)
{
	STU_UTC_time.year=year;
	STU_UTC_time.month=month;
	STU_UTC_time.day=day;
	STU_UTC_time.hour=hour;
	STU_UTC_time.minutes=min;
	STU_UTC_time.seconds=sec;

	utcSeconds = convert_time_to_Second(STU_UTC_time);
}


/********************************************
 * 
 * 
 */
void rtc_set_default_time(void)
{
 
    STU_UTC_time.year=2019;
    STU_UTC_time.month=1;
    STU_UTC_time.day=1;
    STU_UTC_time.hour=15;
    STU_UTC_time.minutes=26;
    STU_UTC_time.seconds=20;

    utcSeconds = convert_time_to_Second(STU_UTC_time);

    ESP_LOGI(rtc_tag, "rtc_timerr=%d \n",utcSeconds);
}


/*************************
 *  1s add timer
 */
/*
void rtc_utc_time_add(void)
{
    utcSeconds++;
}*/


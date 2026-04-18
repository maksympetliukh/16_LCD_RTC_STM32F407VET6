/*
 * rtc.h
 *
 *  Created on: Apr 11, 2026
 *      Author: maksym-petliukh
 */
#ifndef RTC_H_
#define RTC_H_

#include "stm32f407xx.h"

/*
 * Time format macros
 */
#define TIME_FORMAT_12HRS_AM  0
#define TIME_FORMAT_12HRS_PM  1
#define TIME_FORMAT_24HRS     2

/*
 * Day of week macros
 */
#define MONDAY     1
#define TUESDAY    2
#define WEDNESDAY  3
#define THURSDAY   4
#define FRIDAY     5
#define SATURDAY   6
#define SUNDAY     7

/*
 * RTC return status macros
 */
#define RTC_INIT_OK    0  /*RTC initialized successfully*/
#define RTC_INIT_ERR   1  /*RTC init failed (LSE timeout)*/

/*
 * RTC date structure
 */
typedef struct{
    uint8_t date;   /*Day of month: 1-31*/
    uint8_t month;  /*Month:        1-12*/
    uint8_t year;   /*Year:         0-99 (offset from 2000)*/
    uint8_t day;    /*Day of week:  MONDAY=1 ... SUNDAY=7*/
}RTC_DATE_t;

/*
 * RTC time structure
 */
typedef struct{
    uint8_t sec;          /*Seconds:     0-59*/
    uint8_t min;          /*Minutes:     0-59*/
    uint8_t hrs;          /*Hours:       0-23 (24h) or 1-12 (12h)*/
    uint8_t time_format;  /*TIME_FORMAT_24HRS / 12HRS_AM / 12HRS_PM*/
}RTC_TIME_t;

/*
 * RTC function prototypes
 */
uint8_t RTC_Init(void);
void RTC_SetCurrentTime(RTC_TIME_t *rtc_time);
void RTC_GetCurrentTime(RTC_TIME_t *rtc_time);
void RTC_SetCurrentDate(RTC_DATE_t *rtc_date);
void RTC_GetCurrentDate(RTC_DATE_t *rtc_date);
uint8_t bin_to_bcd(uint8_t value);
uint8_t bcd_to_bin(uint8_t value);

#endif /* RTC_H_ */

/*
 * rtc.h
 *
 *  Created on: Apr 11, 2026
 *      Author: maksym-petliukh
 */

#ifndef RTC_H_
#define RTC_H_

/*Application configurable items*/
#define DS1307_I2C            I2C2
#define DS1307_I2C_GPIO_PORT  GPIOB
#define DS1307_I2C_SDA_PIN    GPIO_PIN_11
#define DS1307_I2C_SCL_PIN    GPIO_PIN_10
#define DS1307_I2C_SPEED      I2C_SCL_SPEED_SM
#define DS1307_I2C_PUPD       GPIO_PIN_PU

/*Register addresses*/
#define DS1307_ADDR_SEC       0x00
#define DS1307_ADDR_MIN       0x01
#define DS1307_ADDR_HRS       0x02
#define DS1307_ADDR_DAY       0x03
#define DS1307_ADDR_DATE      0x04
#define DS1307_ADDR_MONTH     0x05
#define DS1307_ADDR_YEAR      0x06

#define TIME_FORMAT_12HRS_AM  0
#define TIME_FORMAT_12HRS_PM  1
#define TIME_FORMAT_24HRS     2

#define DS1307_I2C_SLAVE_ADDR 0x68

#define SUNDAY                1
#define MONDAY                2
#define TUESDAY               3
#define WEDNESDAY             4
#define THURSDAY              5
#define FRIDAY                6
#define SATURSDAY             7

/*RTC DS1307 date structure*/
typedef struct{
	uint8_t date;
	uint8_t month;
	uint8_t year;
	uint8_t day;
}RTC_DATE_t;

/*RTC DS1307 time structure*/
typedef struct{
	uint8_t sec;
	uint8_t min;
	uint8_t hrs;
	uint8_t time_format;
}RTC_TIME_t;

/*RTC function prototypes*/

uint8_t RTC_DS1307_Init(void);
void RTC_DS1307_SetCurrentTime(RTC_TIME_t*);
void RTC_DS1307_GetCurrentTime(RTC_TIME_t*);
void RTC_DS1307_SetCurrentDate(RTC_DATE_t*);
void RTC_DS1307_GetCurrentDate(RTC_DATE_t*);
void RTC_DS1307_I2C_PinConfig(void);
void RTC_DS1307_I2C_Config(void);
void RTC_DS1307_Write(uint8_t value, uint8_t address);
uint8_t RTC_DS1307_Read(uint8_t address);
uint8_t bin_to_bcd(uint8_t value);
uint8_t bcd_to_bin(uint8_t value);

#endif /* RTC_H_ */

/*
 * rtc.c
 *
 *  Created on: Apr 11, 2026
 *      Author: maksym-petliukh
 */

#include "stm32f407xx.h"
#include "rtc.h"

/*Some helper functions*/
void RTC_DS1307_I2C_PinConfig(void){
	GPIO_Handle_t i2c_sda, i2c_scl;

	memset(&i2c_sda, 0,sizeof(i2c_sda));
	memset(&i2c_scl, 0,sizeof(i2c_scl));

	i2c_sda.pGPIOx = DS1307_I2C_GPIO_PORT;
	i2c_sda.GPIOx_CFG.pin_mode = GPIO_MODE_ALT_FN;
	i2c_sda.GPIOx_CFG.pin_alt_func_mode = 4;
	i2c_sda.GPIOx_CFG.pin_number = DS1307_I2C_SDA_PIN;
	i2c_sda.GPIOx_CFG.pin_op_type = GPIO_OUT_OPDRN;
	i2c_sda.GPIOx_CFG.pin_pu_pd_ctrl = DS1307_I2C_PUPD;
	i2c_sda.GPIOx_CFG.pin_speed = GPIO_OSPEED_HIGH;

	GPIO_Init(&i2c_sda);

	i2c_scl.pGPIOx = DS1307_I2C_GPIO_PORT;
	i2c_scl.GPIOx_CFG.pin_mode = GPIO_MODE_ALT_FN;
	i2c_scl.GPIOx_CFG.pin_alt_func_mode = 4;
	i2c_scl.GPIOx_CFG.pin_number = DS1307_I2C_SCL_PIN;
	i2c_scl.GPIOx_CFG.pin_op_type = GPIO_OUT_OPDRN;
	i2c_scl.GPIOx_CFG.pin_pu_pd_ctrl = DS1307_I2C_PUPD;
	i2c_scl.GPIOx_CFG.pin_speed = GPIO_OSPEED_HIGH;

	GPIO_Init(&i2c_scl);
}

I2C_Handle_t gRTC_DS1307_I2C_Handle;

void RTC_DS1307_I2C_Config(void){
	gRTC_DS1307_I2C_Handle.pI2Cx = DS1307_I2C;
	gRTC_DS1307_I2C_Handle.I2C_Config.I2C_AckControl = I2C_ACK_ENABLE;
	gRTC_DS1307_I2C_Handle.I2C_Config.I2C_SCLSpeed = DS1307_I2C_SPEED;

	I2C_Init(&gRTC_DS1307_I2C_Handle);
}

void RTC_DS1307_Write(uint8_t value, uint8_t address){
	uint8_t tx[2];
	tx[0] = address;
	tx[1] = value;
	I2C_Master_Transmit(&gRTC_DS1307_I2C_Handle, tx, 2, DS1307_I2C_SLAVE_ADDR, 0);
}
uint8_t RTC_DS1307_Read(uint8_t address){
	uint8_t rx;
	I2C_Master_Transmit(&gRTC_DS1307_I2C_Handle, &address, 1, DS1307_I2C_SLAVE_ADDR, 0);
	I2C_Master_Receive(&gRTC_DS1307_I2C_Handle, &rx, 1, DS1307_I2C_SLAVE_ADDR, 0);

	return rx;
}

uint8_t  bin_to_bcd(uint8_t value){
	uint8_t m, n, bcd;

	bcd = value;
	if(value >= 10){
		m = value / 10;
		n = value % 10;
		bcd = (uint8_t)((m << 4) | n);
	}
	return bcd;
}

uint8_t bcd_to_bin(uint8_t value){
	uint8_t m, n, bin;

	m = (uint8_t)((value >> 4) * 10);
	n = value & (uint8_t)0x0F;
	bin = m + n;

	return bin;
}

/**********************************************************************
 *@fn             RTC_DS1307_Init
 *
 * @brief         This function initializes required peripherals
 *
 * return         Clock State (CH: 0 - initiated, CH: 1 - init failed)
 */
uint8_t RTC_DS1307_Init(void){
	//initialize I2C pins
	RTC_DS1307_I2C_PinConfig();

	//Initialize I2C peripheral
	RTC_DS1307_I2C_Config();

	//Enable I2C peripheral
	I2C_PeripheralControl(DS1307_I2C, ENABLE);

	//Make clock halt as 0
	RTC_DS1307_Write(0x00, DS1307_ADDR_SEC);

	//Read back clock halt bit
	uint8_t clockState = RTC_DS1307_Read(DS1307_ADDR_SEC);

	return ((clockState >> 7) & 0x1);
}

/********************************************************************
 * @fn              RTC_DS1307_SetCurrentTime
 *
 * @brief           This function sets sec, min, hrs registers of our RTC module
 *
 * @param[in]       Pointer to the structure which contains all required time registers of RTC module
 *
 * @return          none
 */
void RTC_DS1307_SetCurrentTime(RTC_TIME_t *rtc_time){
	uint8_t seconds, hours;
	seconds = bin_to_bcd(rtc_time->sec);
	seconds &= ~(1 << 7);
	RTC_DS1307_Write(seconds, DS1307_ADDR_SEC);

	RTC_DS1307_Write(bin_to_bcd(rtc_time->min), DS1307_ADDR_MIN);\

	hours = bin_to_bcd(rtc_time->hrs);

	if(rtc_time->time_format == TIME_FORMAT_24HRS){
		hours &= ~(1 << 6);
	}else{
		hours |= (1 << 6);
		hours = (rtc_time->time_format == TIME_FORMAT_12HRS_PM) ? (hours | (1 << 5)) : (hours & ~(1 << 5));
	}
	RTC_DS1307_Write(hours, DS1307_ADDR_HRS);
}
/*******************************************************************
 * @fn              RTC_DS1307_GetCurrentTime
 *
 * @brief           This function gets values from the RTC module registers
 *
 * @param[in]       Pointer to the structure which contains all required time registers of RTC module
 *
 * @return          none
 */
void RTC_DS1307_GetCurrentTime(RTC_TIME_t *rtc_time){
	uint8_t seconds, hours;
	seconds = RTC_DS1307_Read(DS1307_ADDR_SEC);

	seconds &= ~(1 << 7);

	rtc_time->sec = bcd_to_bin(seconds);
	rtc_time->min = bcd_to_bin(RTC_DS1307_Read(DS1307_ADDR_MIN));

	hours = RTC_DS1307_Read(DS1307_ADDR_HRS);
	if(hours & (1 << 6)){
		//12 hrs format
		rtc_time->time_format = !((hours & (1 << 5)) == 0);
		hours &= ~(0x3 << 5); // clear 6th and 5th bits
	}else{
		//24 hrs format
		rtc_time->time_format = TIME_FORMAT_24HRS;
	}

	rtc_time->hrs = bcd_to_bin(hours);
}

/**********************************************************************
 * @fn              RTC_DS1307_SetCurrentDate
 *
 * @brief           This function sets the date and number of a day in RTC module registers
 *
 * @param           Pointer to the structure which contains all required date registers of RTC module
 *
 * @return          none
 */
void RTC_DS1307_SetCurrentDate(RTC_DATE_t *rtc_date){
	RTC_DS1307_Write(bin_to_bcd(rtc_date->date), DS1307_ADDR_DATE);

	RTC_DS1307_Write(bin_to_bcd(rtc_date->month), DS1307_ADDR_MONTH);

	RTC_DS1307_Write(bin_to_bcd(rtc_date->year), DS1307_ADDR_YEAR);

	RTC_DS1307_Write(bin_to_bcd(rtc_date->day), DS1307_ADDR_DAY);
}

/*******************************************************************
 * @fn              RTC_DS1307_GetCurrentTime
 *
 * @brief           This function gets values of date and number of a day in RTC module registers
 *
 * @param[in]       Pointer to the structure which contains all required date registers of RTC module
 *
 * @return          none
 */
void RTC_DS1307_GetCurrentDate(RTC_DATE_t *rtc_date){
	rtc_date->date = bcd_to_bin(RTC_DS1307_Read(DS1307_ADDR_DATE) & 0x3F);
	rtc_date->month = bcd_to_bin(RTC_DS1307_Read(DS1307_ADDR_MONTH) & 0x1F);
	rtc_date->year = bcd_to_bin(RTC_DS1307_Read(DS1307_ADDR_YEAR) & 0xFF);
	rtc_date->day = bcd_to_bin(RTC_DS1307_Read(DS1307_ADDR_DAY) & 0x07);
}

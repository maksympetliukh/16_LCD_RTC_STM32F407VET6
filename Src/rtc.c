/*
 * rtc.c
 *
 *  Created on: Apr 11, 2026
 *      Author: maksym-petliukh
 */
#include "stm32f407xx.h"
#include "rtc.h"

/*
 * Helper functions
 */
uint8_t bin_to_bcd(uint8_t value){
    if(value >= 10)
        return (uint8_t)(((value / 10) << 4) | (value % 10));
    return value;
}

uint8_t bcd_to_bin(uint8_t value){
    return (uint8_t)(((value >> 4) * 10) + (value & 0x0F));
}

/*
 * Static helper: disable RTC write protection
 */
static void RTC_DisableWriteProtection(void){
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
}

/*
 * Static helper: enable RTC write protection
 */
static void RTC_EnableWriteProtection(void){
    RTC->WPR = 0xFF;
}

/*
 * Static helper: enter RTC initialization mode
 */
static void RTC_EnterInitMode(void){
    RTC->ISR |= (1 << RTC_ISR_INIT);
    while(!(RTC->ISR & (1 << RTC_ISR_INITF)));
}

/*
 * Static helper: exit RTC initialization mode
 */
static void RTC_ExitInitMode(void){
    RTC->ISR &= ~(1 << RTC_ISR_INIT);
}

/**********************************************************************
 * @fn          RTC_Init
 *
 * @brief       Initializes the STM32F407 internal RTC using LSE
 *              (32.768 kHz crystal on PC14/PC15).
 *              Prescalers: PREDIV_A=127, PREDIV_S=255 → 1 Hz tick.
 *
 * @return      RTC_INIT_OK (0) on success, RTC_INIT_ERR (1) on LSE timeout
 */
uint8_t RTC_Init(void){

    /* 1. Enable PWR clock and unlock backup domain */
    RCC_PWR_CLK_ENABLE();
    PWR->CR |= (1 << PWR_CR_DBP);

    /* 2. Reset backup domain to clear any stuck state */
    RCC->BDCR |= (1 << RCC_BDCR_BDRST);
    RCC->BDCR &= ~(1 << RCC_BDCR_BDRST);

    /* 3. Enable LSE and wait for it to be ready */
    RCC->BDCR |= (1 << RCC_BDCR_LSEON);
    uint32_t timeout = 1000000U;
    while(!(RCC->BDCR & (1 << RCC_BDCR_LSERDY))){
        if(--timeout == 0U) return RTC_INIT_ERR;
    }

    /* 4. Select LSE as RTC clock source (RTCSEL = 01) */
    RCC->BDCR &= ~(3 << RCC_BDCR_RTCSEL);   /*clear RTCSEL*/
    RCC->BDCR |=  (1 << RCC_BDCR_RTCSEL);   /*set LSE*/

    /* 5. Enable RTC clock */
    RCC_RTC_CLK_ENABLE();

    /* 6. Unlock RTC write protection */
    RTC_DisableWriteProtection();

    /* 7. Enter initialization mode */
    RTC_EnterInitMode();

    /* 8. Configure prescalers: PREDIV_A=127, PREDIV_S=255
     *    f_ck_spre = 32768 / (127+1) / (255+1) = 1 Hz */
    RTC->PRER = (127U << 16) | (255U << 0);

    /* 9. Set 24-hour format */
    RTC->CR &= ~(1 << RTC_CR_FMT);

    /* 10. Exit initialization mode and restore write protection */
    RTC_ExitInitMode();
    RTC_EnableWriteProtection();

    return RTC_INIT_OK;
}

/********************************************************************
 * @fn          RTC_SetCurrentTime
 *
 * @brief       Writes hours, minutes and seconds into RTC_TR register.
 *              Input values are in binary, converted to BCD internally.
 *
 * @param[in]   rtc_time  Pointer to RTC_TIME_t structure with time to set
 *
 * @return      none
 */
void RTC_SetCurrentTime(RTC_TIME_t *rtc_time){
    uint8_t sec = bin_to_bcd(rtc_time->sec);
    uint8_t min = bin_to_bcd(rtc_time->min);
    uint8_t hrs = bin_to_bcd(rtc_time->hrs);

    uint32_t tr = 0;
    tr |= ((sec & 0x7FU) << RTC_TR_SU);
    tr |= ((min & 0x7FU) << RTC_TR_MNU);
    tr |= ((hrs & 0x3FU) << RTC_TR_HU);

    if(rtc_time->time_format == TIME_FORMAT_12HRS_PM)
        tr |= (1U << RTC_TR_PM);

    RTC_DisableWriteProtection();
    RTC_EnterInitMode();
    RTC->TR = tr;
    RTC_ExitInitMode();
    RTC_EnableWriteProtection();
}

/*******************************************************************
 * @fn          RTC_GetCurrentTime
 *
 * @brief       Reads hours, minutes and seconds from RTC_TR register.
 *              RTC_TR is read once to avoid split-second capture issue.
 *              Output values are in binary.
 *
 * @param[in]   rtc_time  Pointer to RTC_TIME_t structure to fill
 *
 * @return      none
 */
void RTC_GetCurrentTime(RTC_TIME_t *rtc_time){
    uint32_t tr = RTC->TR;  /*single read: latches DR simultaneously*/

    rtc_time->sec = bcd_to_bin((tr >> RTC_TR_SU) & 0x7FU);
    rtc_time->min = bcd_to_bin((tr >> RTC_TR_MNU) & 0x7FU);
    rtc_time->hrs = bcd_to_bin((tr >> RTC_TR_HU) & 0x3FU);

    if(tr & (1U << RTC_TR_PM))
        rtc_time->time_format = TIME_FORMAT_12HRS_PM;
    else if(RTC->CR & (1U << RTC_CR_FMT))
        rtc_time->time_format = TIME_FORMAT_12HRS_AM;
    else
        rtc_time->time_format = TIME_FORMAT_24HRS;
}

/**********************************************************************
 * @fn          RTC_SetCurrentDate
 *
 * @brief       Writes day, month, year and weekday into RTC_DR register.
 *              Input values are in binary, converted to BCD internally.
 *              Year is stored as offset from 2000 (e.g. 2026 → 26).
 *
 * @param[in]   rtc_date  Pointer to RTC_DATE_t structure with date to set
 *
 * @return      none
 */
void RTC_SetCurrentDate(RTC_DATE_t *rtc_date){
    uint32_t dr = 0;
    dr |= ((bin_to_bcd(rtc_date->date)  & 0x3FU) << RTC_DR_DU);
    dr |= ((bin_to_bcd(rtc_date->month) & 0x1FU) << RTC_DR_MU);
    dr |= ((rtc_date->day               & 0x07U) << RTC_DR_WDU);
    dr |= ((bin_to_bcd(rtc_date->year)  & 0xFFU) << RTC_DR_YU);

    RTC_DisableWriteProtection();
    RTC_EnterInitMode();
    RTC->DR = dr;
    RTC_ExitInitMode();
    RTC_EnableWriteProtection();
}

/*******************************************************************
 * @fn          RTC_GetCurrentDate
 *
 * @brief       Reads day, month, year and weekday from RTC_DR register.
 *              Must be called after RTC_GetCurrentTime() because reading
 *              RTC_TR latches RTC_DR shadow register.
 *              Output values are in binary.
 *
 * @param[in]   rtc_date  Pointer to RTC_DATE_t structure to fill
 *
 * @return      none
 */
void RTC_GetCurrentDate(RTC_DATE_t *rtc_date){
    uint32_t dr = RTC->DR;

    rtc_date->date  = bcd_to_bin((dr >> RTC_DR_DU)  & 0x3FU);
    rtc_date->month = bcd_to_bin((dr >> RTC_DR_MU)  & 0x1FU);
    rtc_date->day   =            (dr >> RTC_DR_WDU) & 0x07U;
    rtc_date->year  = bcd_to_bin((dr >> RTC_DR_YU)  & 0xFFU);
}

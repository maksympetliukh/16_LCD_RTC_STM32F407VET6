/*
 * lcd.h
 *
 *  Created on: Apr 11, 2026
 *      Author: maksym-petliukh
 */
#include "stm32f407xx.h"

#ifndef LCD_H_
#define LCD_H_

/*LCD APIs*/
void LCD_Init(void);
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_SendString(char *message);
void LCD_ClearDisplay(void);
void LCD_ReturnHome(void);
void LCD_SetCursor(uint8_t row, uint8_t column);
void mdelay(uint32_t cnt);
void udelay(uint32_t cnt);

/*Application configurable items (macros)*/
#define LCD_GPIO_PORT GPIOD
#define LCD_GPIO_RS GPIO_PIN_0
#define LCD_GPIO_RW GPIO_PIN_1
#define LCD_GPIO_EN GPIO_PIN_2
#define LCD_GPIO_D4 GPIO_PIN_3
#define LCD_GPIO_D5 GPIO_PIN_4
#define LCD_GPIO_D6 GPIO_PIN_5
#define LCD_GPIO_D7 GPIO_PIN_6

/*LCD commands*/
#define LCD_CMD_4DL_2N_5X8F       0x28 //bata length 4 bits, 2 lines and font size
#define LCD_CMD_DISP_ON_CURS_ON   0x0F //display on, cursor on
#define LCD_CMD_INCADDR           0x06 //increment ddram address
#define LCD_CMD_DISP_CLEAR        0x01 //display clear
#define LCD_CMD_RETURN_HOME       0x02 //return home (set ddram address as 0)

#endif /* LCD_H_ */

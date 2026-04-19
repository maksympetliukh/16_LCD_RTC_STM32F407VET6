/*
 * lcd.c
 *
 *  Created on: Apr 11, 2026
 *      Author: maksym-petliukh
 */

#include "stm32f407xx.h"
#include "lcd.h"

/*Helper functions*/
void mdelay(uint32_t cnt){
	for(volatile uint32_t i = 0; i < (cnt * 1000); i++);
}

void udelay(uint32_t cnt){
	for(volatile uint32_t i = 0; i < (cnt * 1); i++);
}

static void lcd_enable(void){
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_EN, SET);
	udelay(10);
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_EN, RESET);
	udelay(100);
}

static void write_4_bits(uint8_t value){
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D4, ((value >> 0) & 0x1));
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D5, ((value >> 1) & 0x1));
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D6, ((value >> 2) & 0x1));
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D7, ((value >> 3) & 0x1));

	lcd_enable();
}

void LCD_ClearDisplay(void){
	LCD_SendCommand(LCD_CMD_DISP_CLEAR);
	mdelay(2);
}

void LCD_ReturnHome(void){
	LCD_SendCommand(LCD_CMD_RETURN_HOME);
	mdelay(2);
}

void LCD_SetCursor(uint8_t row, uint8_t column){
	column--;
	switch(row){
	case 1:
		//set cursor to 1st row address and add index
		LCD_SendCommand((0x80 | column));
		break;
	case 2:
		//set cursor to 2nd row address and add index
		LCD_SendCommand((0xC0 | column));
		break;
	default:
		break;
	}
}

/********************************************************
 * @fn        LCD_SendCommand
 *
 * @brief     This function writes user commands into data lines D4-D7 accordingly to the LCD datasheet
 *
 * @param[in] Variable which contains a user command
 *
 * @return    none
 */
void LCD_SendCommand(uint8_t cmd){
	//RS and RW lines must be 0
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RS, RESET);
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RW, RESET);

	//send higher nibble
	write_4_bits(cmd >> 4);

	//send lower nibble
	write_4_bits(cmd & 0x0F);
}

/********************************************************
 * @fn        LCD_SendData
 *
 * @brief     This function writes the data on to data lines of LCD
 *
 * @param[in] User data variable
 *
 * @return    none
 */
void LCD_SendData(uint8_t data){
	//make RS = 1 to send user data
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RS, SET);

	//make RW = 0 for write
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RW, RESET);

	//higher nibble
	write_4_bits(data >> 4);

	//lower nibble
	write_4_bits(data & 0x0F);
}

/********************************************************
 * @fn        LCD_SendString
 *
 * @brief     This function writes the user string into data lines of LCD
 *
 * @param[in]     User data string pointer
 *
 * @return    none
 */
void LCD_SendString(char *message){
	while(*message != '\0'){
		LCD_SendData((uint8_t)*message++);
	}
}

/********************************************************
 * @fn        LCD_Init
 *
 * @brief     This function configures GPIO pins and initializes LCD
 *
 * @param[in] none
 *
 * @return    none
 */
void LCD_Init(void){
	//Configure required GPIO pins
	GPIO_Handle_t lcd_gpio;

	GPIO_ClockControl(GPIOD, ENABLE);
	lcd_gpio.pGPIOx = LCD_GPIO_PORT;
	lcd_gpio.GPIOx_CFG.pin_mode = GPIO_MODE_OUT;
	lcd_gpio.GPIOx_CFG.pin_number = LCD_GPIO_RS;
	lcd_gpio.GPIOx_CFG.pin_op_type = GPIO_OUT_PP;
	lcd_gpio.GPIOx_CFG.pin_pu_pd_ctrl = GPIO_NO_PUPD;
	lcd_gpio.GPIOx_CFG.pin_speed = GPIO_OSPEED_HIGH;
	GPIO_Init(&lcd_gpio);

	lcd_gpio.GPIOx_CFG.pin_number = LCD_GPIO_RW;
	GPIO_Init(&lcd_gpio);

	lcd_gpio.GPIOx_CFG.pin_number = LCD_GPIO_EN;
	GPIO_Init(&lcd_gpio);

	lcd_gpio.GPIOx_CFG.pin_number = LCD_GPIO_D4;
	GPIO_Init(&lcd_gpio);

	lcd_gpio.GPIOx_CFG.pin_number = LCD_GPIO_D5;
	GPIO_Init(&lcd_gpio);

	lcd_gpio.GPIOx_CFG.pin_number = LCD_GPIO_D6;
	GPIO_Init(&lcd_gpio);

	lcd_gpio.GPIOx_CFG.pin_number = LCD_GPIO_D7;
	GPIO_Init(&lcd_gpio);

	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RS, RESET);
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RW, RESET);
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_EN, RESET);
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D4, RESET);
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D5, RESET);
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D6, RESET);
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_D7, RESET);

	//Initialize the LCD with 4-bit interface

	//40 ms delay
	mdelay(40);

	//RS = 0, RW = 0 for initial command and writing on to LCD
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RS, RESET);
	GPIO_WritePin(LCD_GPIO_PORT, LCD_GPIO_RW, RESET);
	//Write initial data/command on to D4-D7 lines
	write_4_bits(0x3);

	//wait min 4.1 ms
	mdelay(5);

	write_4_bits(0x3);

	//100 microseconds
	udelay(150);

	write_4_bits(0x3);
	udelay(150);
	write_4_bits(0x2);
	udelay(150);

	//function set command
	LCD_SendCommand(LCD_CMD_4DL_2N_5X8F);

	//Make display and cursor ON
	LCD_SendCommand(LCD_CMD_DISP_ON_CURS_ON);

	//Clear the display and wait required amount of time
	LCD_ClearDisplay();

	//Entry mode set
	LCD_SendCommand(LCD_CMD_INCADDR);
}


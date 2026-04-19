# RTC and LCD on STM32F407VET6

### What this project is
This project was created to learn basics, logic and core operating principles of Real Time Clock on STM32 MCU. This project
also was created to learn how to connect LCD and design its logic and driver for STM32 MCU.
This project is developed accordingly to online course [(MCU1) Learn bare metal driver development using Embedded C: Writing drivers for STM32 GPIO,I2C,SPI,USART from scratch](https://www.udemy.com/course/mastering-microcontroller-with-peripheral-driver-development/)
by Kiran Nayak (FastBit Embedded Brain Academy).

### Goals
- learn basic principles and how RTC works
- learn how to design RTC driver and APIs
- learn LCD basics, its architecture and how to connect it to MCU
- learn how to display information on LCD 
- learn how to display real time on LCD

### What actually program does
- configures RTC
- configures LCD
- sets user time and date
- initiates LCD
- writes user time and date on to LCD data lines
- displays user time and date on LCD

### Used devices
- MCU: **STM32F407VET6**
- CPU: **ARM Cortex-M4**
- RTC: **Internal STM32F407 RTC module**
- LCD: **HD44780 QC16802B HITACHI**
- Debugger: **ST-LINK V2.1**

### Used software and documentation
- Programming language: **C**
- Implementation: **bare-metal**
- IDE: **STM32CubeIDE**
- Reference manual: **RM0090**
- User manual: **ARM Cortex-M4**
- Datasheet: **HD44780 HITACHI**
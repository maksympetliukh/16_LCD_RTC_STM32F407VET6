/*
 * usart.h
 *
 *  Created on: Apr 11, 2026
 *      Author: maksym-petliukh
 */

#include "stm32f407xx.h"

#ifndef USART_H_
#define USART_H_

/*
 * Configuration structure for USARTx peripheral
 */
typedef struct{
	uint8_t USART_Mode;
	uint32_t USART_Baud;
	uint8_t USART_NoOfStopBits;
	uint8_t USART_WordLength;
	uint8_t USART_ParityControl;
	uint8_t USART_HWFlowControl;
}USART_Config_t;

/*
 * Handle structure for USARTx peripheral
 */
typedef struct{
	USART_REG_t *pUSARTx;
	USART_Config_t USART_Config;
	uint8_t *pTxBuffer;
	uint8_t *pRxBuffer;
	uint8_t TxState;
	uint8_t RxState;
	uint32_t TxLen;
	uint32_t RxLen;
}USART_Handle_t;

/*
 * @USART_Mode
 */
#define USART_MODE_ONLY_TX   0
#define USART_MODE_ONLY_RX   1
#define USART_MODE_TXRX      2

/*
 * @USART_Baud
 */
#define USART_STD_BAUD_1200           1200
#define USART_STD_BAUD_2400           2400
#define USART_STD_BAUD_9600           9600
#define USART_STD_BAUD_19200          19200
#define USART_STD_BAUD_38400          38400
#define USART_STD_BAUD_57600          57600
#define USART_STD_BAUD_115200         115200
#define USART_STD_BAUD_230400         230400
#define USART_STD_BAUD_460800         460800
#define USART_STD_BAUD_921600         921600
#define USART_STD_BAUD_2M             2000000
#define USART_STD_BAUD_3M             3000000

/*
 * @USART_ParityControl
 */
#define USART_PARITY_EN_ODD    2
#define USART_PARITY_EN_EVEN   1
#define USART_PARITY_DISABLE   0

/*
 * @USART_WordLength
 */
#define USART_WORDLEN_8BITS 0
#define USART_WORDLEN_9BITS 1

/*
 * @USART_NoOfStopBits
 */
#define USART_STOPBITS_1      0
#define USART_STOPBITS_0_5    1
#define USART_STOPBITS_2      2
#define USART_STOPBITS_1_5    3

/*
 * @USART_HWFlowControl
 */
#define USART_HW_FLOW_CTRL_NONE       0
#define USART_HW_FLOW_CTRL_CTS        1
#define USART_HW_FLOW_CTRL_RTS        2
#define USART_HW_FLOW_CTRL_CTS_RTS    3

/*
 * @TxRxState
 */
#define USART_READY             0
#define USART_BUSY_IN_TX        1
#define USART_BUSY_IN_RX        2

/*
 * USART events and errors macros
 */
#define USART_EV_TX_CMPLT        0
#define USART_EV_RX_CMPLT        1
#define USART_EV_IDLE            2
#define USART_EV_CTS             3
#define USART_ERR_PE             4
#define USART_ERR_FE             5
#define USART_ERR_NE             6
#define USART_ERR_ORE            7

/*
 * USART Status Register Flags (Bit Masks)
 */
#define USART_FLAG_PE      (1 << 0)
#define USART_FLAG_FE      (1 << 1)
#define USART_FLAG_NE      (1 << 2)
#define USART_FLAG_ORE     (1 << 3)
#define USART_FLAG_IDLE    (1 << 4)
#define USART_FLAG_RXNE    (1 << 5)
#define USART_FLAG_TC      (1 << 6)
#define USART_FLAG_TXE     (1 << 7)
#define USART_FLAG_LBD     (1 << 8)
#define USART_FLAG_CTS     (1 << 9)

/*
 * USART APIs
 */
void USART_Init(USART_Handle_t *pUSART_Handle);
void USART_DeInit(USART_REG_t *pUSARTx);
void USART_ClockControl(USART_REG_t *pUSARTx, uint8_t enDi);
void USART_PeripheralControl(USART_REG_t *pUSARTx, uint8_t enDi);
void USART_Transmit(USART_Handle_t *pUSART_Handle, uint8_t *pTxBuffer, uint32_t len);
void USART_Receive(USART_Handle_t *pUSART_Handle, uint8_t *pRxBuffer, uint32_t len);
uint8_t USART_TransmitIT(USART_Handle_t *pUSART_Handle, uint8_t *pTxBuffer, uint32_t len);
uint8_t USART_ReceiveIT(USART_Handle_t *pUSART_Handle, uint8_t *pRxBuffer, uint32_t len);
uint8_t USART_GetFlagStatus(USART_REG_t *pUSARTx, uint32_t flag);
void USART_ClearFlag(USART_Handle_t *pUSART_Handle, uint16_t flag);
void USART_IRQInterruptConfig(uint8_t IRQNumber, uint8_t enDi);
void USART_IRQIPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);
void USART_IRQHandle(USART_Handle_t *pUSART_Handle);
void USART_ApplicationEventCallback(USART_Handle_t *pUSART_Handle, uint8_t event);
uint32_t RCC_USART_GetPCLK1Value(void);
uint32_t RCC_USART_GetPCLK2Value(void);
uint32_t RCC_USART_GetPLLOutputClock(void);
void USART_SetBaudRate(USART_REG_t *pUSARTx, uint32_t BaudRate);
void USART_EnableInterrupt(USART_Handle_t *pUSART_Handle, uint16_t flag, uint8_t enDi);

#endif /* USART_H_ */

/*
 * i2c.h
 *
 *  Created on: Apr 11, 2026
 *      Author: maksym-petliukh
 */

#include "stm32f407xx.h"

#ifndef I2C_H_
#define I2C_H_

//Configuration structure for I2Cx peripheral
typedef struct{
	uint32_t I2C_SCLSpeed;
	uint8_t I2C_DeviseAddress;
	uint8_t I2C_AckControl;
	uint16_t I2C_FMDutyCycle;
}I2C_Config_t;


//Handle structure for I2C peripheral
typedef struct{
	I2C_REG_t *pI2Cx;
	I2C_Config_t I2C_Config;
	uint8_t *pTxBuffer;
	uint8_t *pRxBuffer;
	uint32_t TxLen;
	uint32_t RxLen;
	uint8_t TxRxState;
	uint8_t DevAddr;
	uint32_t RxSize;
	uint8_t Sr;
}I2C_Handle_t;


//@I2C_SLCSpeed
#define I2C_SCL_SPEED_10 10000
#define I2C_SCL_SPEED_SM 100000
#define I2C_SCL_SPEED_FM 400000
#define I2C_SCL_SPEED_2K 200000

//@I2C_AckControl
#define I2C_ACK_ENABLE  1
#define I2C_ACK_DISABLE 0

//@I2C_FMDutyCycle
#define I2C_FM_DUTY_2     0
#define I2C_FM_DUTY_16_9  1

//APIs supported by I2C

void I2C_Init(I2C_Handle_t *pI2C_Handle);
void I2C_DeInit(I2C_REG_t *pI2Cx);
void I2C_ClockControl(I2C_REG_t *pI2Cx, uint8_t en_di_mode);
void I2C_IRQ_InterruptConfig(uint8_t IRQ_Number, uint8_t en_di_mode);
void I2C_IRQ_PriorityConfig(uint8_t IRQ_Number, uint8_t IRQ_Priority);
void I2C_IRQ_EV_Handler(I2C_Handle_t *pI2C_Handle);
void I2C_IRQ_ER_Handler(I2C_Handle_t *pI2C_Handle);
void I2C_PeripheralControl(I2C_REG_t *pI2Cx, uint8_t en_di_mode);
void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t event);
uint8_t I2C_GetFlagStatus(I2C_REG_t *pI2Cx, uint32_t flag);
uint32_t RCC_I2C_GetPCLK1Value(void);
uint32_t RCC_GetPLLOutputClock(void);
void I2C_Master_Transmit(I2C_Handle_t *pI2C_Handle, uint8_t *pTxBuffer, uint8_t len, uint8_t SlaveAddr, uint8_t sr);
void I2C_Master_Receive(I2C_Handle_t *pI2C_Handle, uint8_t *pRxBuffer, uint8_t len, uint8_t SlaveAddr, uint8_t sr);
void I2C_GenerateStartCondition(I2C_REG_t *pI2Cx);
void I2C_GenerateStopCondition(I2C_REG_t *pI2Cx);
void I2C_ExecuteAddressPhase_TX(I2C_REG_t *pI2Cx, uint8_t SlaveAddr);
void I2C_ExecuteAddressPhase_RX(I2C_REG_t *pI2Cx, uint8_t SlaveAddr);
void I2C_ClearADDRFlag(I2C_Handle_t *pI2C_Handle);
void I2C_AckControl(I2C_REG_t *pI2Cx, uint8_t en_di_mode);
uint8_t I2C_Master_Transmit_IT(I2C_Handle_t *pI2C_Handle, uint8_t *pTxBuffer, uint8_t len, uint8_t SlaveAddr, uint8_t sr);
uint8_t I2C_Master_Receive_IT(I2C_Handle_t *pI2C_Handle, uint8_t *pTxBuffer, uint8_t len, uint8_t SlaveAddr, uint8_t sr);
void I2C_Slave_Transmit(I2C_REG_t *pI2Cx,  uint8_t data);
uint8_t I2C_Slave_Receive(I2C_REG_t *pI2Cx);
void I2C_CloseTransmission(I2C_Handle_t *pI2C_Handle);
void I2C_CloseReception(I2C_Handle_t *pI2C_Handle);
void I2C_ClearSTOPF(I2C_Handle_t *pI2C_Handle);
void I2C_Slave_EnableInterrupts(I2C_REG_t *pI2Cx, uint8_t enDi);

/*
 * I2C Flags
 */
#define I2C_SB_FLAG          (1 << I2C_SR1_SB)
#define I2C_ADDR_FLAG        (1 << I2C_SR1_ADDR)
#define I2C_BTF_FLAG         (1 << I2C_SR1_BTF)
#define I2C_ADD10_FLAG       (1 << I2C_SR1_ADD10)
#define I2C_STOPF_FLAG       (1 << I2C_SR1_STOPF)
#define I2C_RXNE_FLAG        (1 << I2C_SR1_RXNE)
#define I2C_TXE_FLAG         (1 << I2C_SR1_TXE)
#define I2C_BERR_FLAG        (1 << I2C_SR1_BERR)
#define I2C_ARLO_FLAG        (1 << I2C_SR1_ARLO)
#define I2C_AF_FLAG          (1 << I2C_SR1_AF)
#define I2C_OVR_FLAG         (1 << I2C_SR1_OVR)
#define I2C_PECERR_FLAG      (1 << I2C_SR1_PECERR)
#define I2C_TIMEOUT_FLAG     (1 << I2C_SR1_TIMEOUT)
#define I2C_SMBALERT_FLAG    (1 << I2C_SR1_SMBALERT)

/*
 * I2C Repeated Start flag macros
 */
#define I2C_SR_EN       1
#define I2C_SR_DI       0

/*
 * I2C application states
 */
#define I2C_READY       0
#define I2C_BUSY_IN_RX  1
#define I2C_BUSY_IN_TX  2

/*
 * I2C event macros
 */
#define I2C_EV_TX_CMPLT    0
#define I2C_EV_RX_CMPLT    1
#define I2C_EV_STOP        2
#define I2C_ERROR_BERR     3
#define I2C_ERROR_ARLO     4
#define I2C_ERROR_AF       5
#define I2C_ERROR_OVR      6
#define I2C_ERROR_TIMEOUT  7
#define I2C_EV_DATA_REQ    8
#define I2C_EV_DATA_RCV    9
#define I2C_EV_ADDR_MATCH  10

#endif /* I2C_H_ */

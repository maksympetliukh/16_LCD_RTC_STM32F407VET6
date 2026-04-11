/*
 * i2c.c
 *
 *  Created on: Apr 11, 2026
 *      Author: maksym-petliukh
 */

#include "i2c.h"

//Unused function
uint32_t RCC_GetPLLOutputClock(void){
	return 0;
}

//Array which contains all division factors of the AHB prescaler
uint16_t AHB_Prescaler[8] = {2, 4, 8, 16, 32, 64, 128, 256, 512};

//Array which contains all division factors of the APB prescaler
uint16_t APB1_Prescaler[4] = {2, 4, 8, 16};

/**************************************
 * @fn          RCC_GetPCLK1Value
 *
 * @Brief       This function returns the value of frequency of the required peripheral clock
 *
 * return       Variable which contains the value of frequency of the peripheral clock
 */
uint32_t RCC_I2C_GetPCLK1Value(void){
	uint32_t pclk1, SystemClk;

	uint8_t clksrc, tmp, ahb_prescaler, apb1_prescaler;
	clksrc = ((RCC->CFGR >> 2) & 0x3); //Right shift by 2 to take bits [3:2] and mask other bits

	if(clksrc == 0){
		SystemClk = 16000000;
	}else if(clksrc == 1){
		SystemClk = 8000000;
	}else if(clksrc == 2){
		SystemClk = RCC_GetPLLOutputClock();
	}

	//AHB bus prescaling
	tmp = ((RCC->CFGR >> 4) & 0xF);

	if(tmp < 8){
		ahb_prescaler = 1;
	}else{
		ahb_prescaler = AHB_Prescaler[tmp - 8];
	}

	//APB prescaling
	tmp = ((RCC->CFGR >> 10) & 0x7);

	if(tmp < 4){
		apb1_prescaler = 1;
	}else{
		apb1_prescaler = APB1_Prescaler[tmp - 4];
	}

	pclk1 = (SystemClk / ahb_prescaler) / apb1_prescaler;

	return pclk1;
}

/**************************************
 * @fn          I2C_Init
 *
 * @brief       This function initializes the I2C port according to the specified settings in the handle structure
 *
 * @param[in]   Pointer to the structure which contains I2C specified configuration settings
 *
 * @return      none
 */
void I2C_Init(I2C_Handle_t *pI2C_Handle){
	uint32_t tempreg = 0;

	//Configure the ACK control bit
	tempreg |= pI2C_Handle->I2C_Config.I2C_AckControl << 10;
	pI2C_Handle->pI2Cx->CR1 = tempreg;

	//Configure the FREQ field of CR2
	tempreg = 0;
	tempreg |= RCC_I2C_GetPCLK1Value() / 1000000U;
	pI2C_Handle->pI2Cx->CR2 = (tempreg & 0x3F);

	//Program the device own address OAR1
	tempreg = 0;
	tempreg |= pI2C_Handle->I2C_Config.I2C_DeviseAddress << 1;
	tempreg |= (1 << 14);//according to the RM0090 14th bit must be set despite it is a reserved bit
	pI2C_Handle->pI2Cx->OAR1 = tempreg;

	//CCR calculations
	uint16_t ccr_value = 0;
	tempreg = 0;

	if(pI2C_Handle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM){

		//In case of Standard mode
		ccr_value = RCC_I2C_GetPCLK1Value() / (2 * pI2C_Handle->I2C_Config.I2C_SCLSpeed);
		tempreg |= (ccr_value & 0xFFF);

	}else if(pI2C_Handle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_FM){

		//In case of Fast mode
		tempreg |= (1 << 15);
		tempreg |= (pI2C_Handle->I2C_Config.I2C_FMDutyCycle << 14);

		if(pI2C_Handle->I2C_Config.I2C_FMDutyCycle == I2C_FM_DUTY_2){
			ccr_value = RCC_I2C_GetPCLK1Value() / (3 * pI2C_Handle->I2C_Config.I2C_SCLSpeed);
		}else if(pI2C_Handle->I2C_Config.I2C_FMDutyCycle == I2C_FM_DUTY_16_9){
			ccr_value = RCC_I2C_GetPCLK1Value() / (25 * pI2C_Handle->I2C_Config.I2C_SCLSpeed);
		}
		tempreg |= (ccr_value & 0xFFF);
	}

	pI2C_Handle->pI2Cx->CCR = tempreg;

	//TRISE configuration
	tempreg = 0;
	if(pI2C_Handle->I2C_Config.I2C_SCLSpeed <= I2C_SCL_SPEED_SM){

		//standard mode
		tempreg = (RCC_I2C_GetPCLK1Value() / 1000000U) + 1;
	}else{
		//fast mode
		tempreg = ((RCC_I2C_GetPCLK1Value() * 300) / 1000000U) + 1;
	}

	pI2C_Handle->pI2Cx->TRISE = (tempreg & 0xFFF);
}

/**************************************
 * @fn          I2C_DeInit
 *
 * @brief       This function disables I2C port
 *
 * @param[in]   Pointer to the structure which contains I2C peripheral registers
 *
 * @return      none
 */
void I2C_DeInit(I2C_REG_t *pI2Cx){
	if(pI2Cx == NULL){return;}

	if(pI2Cx == I2C1){
		I2C1_REG_RESET();
	}else if(pI2Cx == I2C2){
		I2C2_REG_RESET();
	}else if(pI2Cx == I2C3){
		I2C3_REG_RESET();
	}
}

/**************************************
 * @fn          I2C_ClockControl
 *
 * @brief       This function enables/disables a Peripheral Clock for the I2C peripheral
 *
 * @param[in]   Pointer to the structure which contains I2C peripheral registers
 * @param[in]   Enable/Disable mode variable
 *
 * @return      none
 */
void I2C_ClockControl(I2C_REG_t *pI2Cx, uint8_t en_di_mode){
	if(en_di_mode == ENABLE){
		if(pI2Cx == I2C1){
			RCC_I2C1_CLK_ENABLE();
		}else if(pI2Cx == I2C2){
			RCC_I2C2_CLK_ENABLE();
		}else if(pI2Cx == I2C3){
			RCC_I2C3_CLK_ENABLE();
		}
	}else if(en_di_mode == DISABLE){
		if(pI2Cx == I2C1){
			RCC_I2C1_CLK_DISABLE();
		}else if(pI2Cx == I2C2){
			RCC_I2C2_CLK_DISABLE();
		}else if(pI2Cx == I2C3){
			RCC_I2C3_CLK_DISABLE();
		}
	}
}

/*********************************************
 * @fn           I2C_PeripheralControl
 *
 * @brief        This function enable I2C peripheral
 *
 * @param[in]    Pointer to the structure which contains I2C peripheral register base addresses
 * @param[in]    Enable/Disable variable
 *
 * @return       none
 */
void I2C_PeripheralControl(I2C_REG_t *pI2Cx, uint8_t en_di_mode){
	if(en_di_mode == ENABLE){
		pI2Cx->CR1 |= (1 << I2C_CR1_PE);
	}else if(en_di_mode == DISABLE){
		pI2Cx->CR1 &= ~(1 << I2C_CR1_PE);
	}
}

//Helper functions
void I2C_GenerateStartCondition(I2C_REG_t *pI2Cx){
	pI2Cx->CR1 |= (1 << I2C_CR1_START);
}

uint8_t I2C_GetFlagStatus(I2C_REG_t *pI2Cx, uint32_t flag){
	if(pI2Cx->SR1 & flag){
		return SET;
	}
	return RESET;
}

void I2C_ExecuteAddressPhase_TX(I2C_REG_t *pI2Cx, uint8_t SlaveAddr){
	SlaveAddr = SlaveAddr << 1; //make a space for r/w bit
	SlaveAddr &= ~(1); //clear the bit 0
	pI2Cx->DR = SlaveAddr;
}

void I2C_ExecuteAddressPhase_RX(I2C_REG_t *pI2Cx, uint8_t SlaveAddr){
	SlaveAddr = SlaveAddr << 1; //make a space for r/w bit
	SlaveAddr |= 1; //clear the bit 0
	pI2Cx->DR = SlaveAddr;
}

void I2C_ClearADDRFlag(I2C_Handle_t *pI2C_Handle){
    uint32_t dummyRead;
    if(pI2C_Handle->pI2Cx->SR2 & (1 << I2C_SR2_MSL)){
        if(pI2C_Handle->TxRxState == I2C_BUSY_IN_RX){
            if(pI2C_Handle->RxSize == 1){
                I2C_AckControl(pI2C_Handle->pI2Cx, I2C_ACK_DISABLE);
            }
            dummyRead = pI2C_Handle->pI2Cx->SR1;
            dummyRead = pI2C_Handle->pI2Cx->SR2;
            (void)dummyRead;
        }else{
            dummyRead = pI2C_Handle->pI2Cx->SR1;
            dummyRead = pI2C_Handle->pI2Cx->SR2;
            (void)dummyRead;
        }
    }else{
        dummyRead = pI2C_Handle->pI2Cx->SR1;
        dummyRead = pI2C_Handle->pI2Cx->SR2;
        (void)dummyRead;
    }
}

void I2C_GenerateStopCondition(I2C_REG_t *pI2Cx){
	pI2Cx->CR1 |= (1 << I2C_CR1_STOP);
}

void I2C_AckControl(I2C_REG_t *pI2Cx, uint8_t en_di_mode){
	if(en_di_mode == I2C_ACK_ENABLE){
		pI2Cx->CR1 |= (1 << I2C_CR1_ACK);
	}else if(en_di_mode == I2C_ACK_DISABLE){
		pI2Cx->CR1 &= ~(1 << I2C_CR1_ACK);
	}
}

void I2C_CloseTransmission(I2C_Handle_t *pI2C_Handle){
	//disable ITBUFEN control bit
	pI2C_Handle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITBUFEN);

	//disable ITEVTEN control bit
	pI2C_Handle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITEVTEN);

	pI2C_Handle->TxRxState = I2C_READY;
	pI2C_Handle->pTxBuffer = NULL;
	pI2C_Handle->TxLen = 0;
}

void I2C_CloseReception(I2C_Handle_t *pI2C_Handle){
	//disable ITBUFEN control bit
	pI2C_Handle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITBUFEN);

	//disable ITEVTEN control bit
	pI2C_Handle->pI2Cx->CR2 &= ~(1 << I2C_CR2_ITEVTEN);

	pI2C_Handle->TxRxState = I2C_READY;
	pI2C_Handle->pRxBuffer = NULL;
	pI2C_Handle->RxLen = 0;
	pI2C_Handle->RxSize = 0;

	if(pI2C_Handle->I2C_Config.I2C_AckControl == I2C_ACK_ENABLE){
		I2C_AckControl(pI2C_Handle->pI2Cx, ENABLE);
	}
}

void I2C_ClearSTOPF(I2C_Handle_t *pI2C_Handle){
	uint32_t dummyRead;
	dummyRead = pI2C_Handle->pI2Cx->SR1;
	dummyRead = pI2C_Handle->pI2Cx->CR1;
	(void)dummyRead;
}

void I2C_Slave_EnableInterrupts(I2C_REG_t *pI2Cx, uint8_t enDi){
	if(enDi == ENABLE){
    pI2Cx->CR2 |= (1 << I2C_CR2_ITEVTEN);
    pI2Cx->CR2 |= (1 << I2C_CR2_ITBUFEN);
    pI2Cx->CR2 |= (1 << I2C_CR2_ITERREN);
	}else{
		 pI2Cx->CR2 &= ~(1 << I2C_CR2_ITEVTEN);
		 pI2Cx->CR2 &= ~(1 << I2C_CR2_ITBUFEN);
		 pI2Cx->CR2 &= ~(1 << I2C_CR2_ITERREN);
	}
}
/*********************************************
 * @fn           I2C_Master_Transmit
 *
 * @brief        This function sends data from Master to slave via I2C peripheral
 *
 * @param[in]    Pointer to the structure which contains I2C specified configuration settings
 * @param[in]    Pointer to the TX buffer with data which we want to send
 * @param[in]    Length of the data
 * @param[in]    Address of the slave device
 * @param[in]    Enable/Disable repeated start
 *
 * @return       none
 */
void I2C_Master_Transmit(I2C_Handle_t *pI2C_Handle, uint8_t *pTxBuffer, uint8_t len, uint8_t SlaveAddr, uint8_t sr){

	//Generate START condition
	I2C_GenerateStartCondition(pI2C_Handle->pI2Cx);

	//Confirm that start generation is completed by checking the SB flag in the SR1
	//Until SB is cleared SCL will be stretched (pulled to LOW)
	while(!I2C_GetFlagStatus(pI2C_Handle->pI2Cx, I2C_SB_FLAG));

	//Send address of the slave with r/w bit set to w(0) (total 8 bits)
	I2C_ExecuteAddressPhase_TX(pI2C_Handle->pI2Cx, SlaveAddr);

	//Confirm that address phase is completed by checking the ADDR flag in the SR1
	while(!I2C_GetFlagStatus(pI2C_Handle->pI2Cx, I2C_ADDR_FLAG));

	//Clear the ADDR flag according to its software sequence
	//Until ADDR is cleared SCL will be stretched (pulled to LOW)
	I2C_ClearADDRFlag(pI2C_Handle);

	//Transmit the data until the length becomes 0
	while(len > 0){
		while(!I2C_GetFlagStatus(pI2C_Handle->pI2Cx, I2C_TXE_FLAG));
		pI2C_Handle->pI2Cx->DR = *pTxBuffer;
		pTxBuffer++;
		len--;
	}

	//When len becomes 0, wait for TXE = 1 and BTF = 1 before generating the STOP condition
	//TXE = 1, BTF = 1 means that both SR1 and DR are empty and next transmission should begin
	//When BTF = 1 SCL will be stretched (pulled to low)
	while(!I2C_GetFlagStatus(pI2C_Handle->pI2Cx, I2C_TXE_FLAG));
	while(!I2C_GetFlagStatus(pI2C_Handle->pI2Cx, I2C_BTF_FLAG));

	//Generate the STOP condition and master need not to wait for the completion of stop condition
	//Generating STOP automatically clears the BTF
	if(sr == I2C_SR_DI){
	I2C_GenerateStopCondition(pI2C_Handle->pI2Cx);
	}
}

/*********************************************************
 * fn@              I2C_Master_Receive
 *
 * @brief           This function receives the data from slave to master
 *
 * @param[in]       Pointer to the structure with peripheral register base addresses
 * @param[in]       Pointer to buffer with received data
 * @param[in]       Length of the received data
 *
 * @return          none
 */

void I2C_Master_Receive(I2C_Handle_t *pI2C_Handle, uint8_t *pRxBuffer, uint8_t len, uint8_t SlaveAddr, uint8_t sr){
	//generate the start condition
	I2C_GenerateStartCondition(pI2C_Handle->pI2Cx);

	//Confirm that start generation is completed by checking the SB flag in the SR1
	//Until SB is cleared SCL will be stretched (pulled to LOW)
	while(!I2C_GetFlagStatus(pI2C_Handle->pI2Cx, I2C_SB_FLAG));

	//Send address of the slave with r/w bit set to w(1) (total 8 bits)
	I2C_ExecuteAddressPhase_RX(pI2C_Handle->pI2Cx, SlaveAddr);

	//Confirm that address phase is completed by checking the ADDR flag in the SR1
	while(!I2C_GetFlagStatus(pI2C_Handle->pI2Cx, I2C_ADDR_FLAG));

	//procedure to read only 1 byte from slave
	if(len == 1){
		//Disable ACK
		I2C_AckControl(pI2C_Handle->pI2Cx, I2C_ACK_DISABLE);

		//Clear the ADDR flag
		I2C_ClearADDRFlag(pI2C_Handle);

		//wait until RXNE becomes 1
		while(!I2C_GetFlagStatus(pI2C_Handle->pI2Cx, I2C_RXNE_FLAG));

		//generate stop condition
		if(sr == I2C_SR_DI){
		I2C_GenerateStopCondition(pI2C_Handle->pI2Cx);
		}
		//read the data into buffer
		*pRxBuffer = pI2C_Handle->pI2Cx->DR;
	}else if(len > 1){

		//Clear the ADDR flag
		I2C_ClearADDRFlag(pI2C_Handle);

		//read the data until len becomes zero
		for(uint32_t i = len; i > 0; i--){
			//wait until len becomes 1
			while(!I2C_GetFlagStatus(pI2C_Handle->pI2Cx, I2C_RXNE_FLAG));

			if(i == 2){
				//Disable ACK
				I2C_AckControl(pI2C_Handle->pI2Cx, I2C_ACK_DISABLE);

				//generate stop condition
				if(sr == I2C_SR_DI){
				I2C_GenerateStopCondition(pI2C_Handle->pI2Cx);
				}
			}

			//read the data into buffer
			*pRxBuffer = pI2C_Handle->pI2Cx->DR;
			pRxBuffer++;
		}
	}

	//Enable ACK
	if(pI2C_Handle->I2C_Config.I2C_AckControl == ENABLE){
	I2C_AckControl(pI2C_Handle->pI2Cx, I2C_ACK_ENABLE);
	}
}
/**************************************************************
 * @fn            I2C_IRQ_InterruptConfig
 *
 * @brief         This function enables required IRQ number
 *
 * @param[in]     IQR number
 * @param[in]     enable/disable mode
 *
 * @return        none
 */
void I2C_IRQ_InterruptConfig(uint8_t IRQ_Number, uint8_t en_di_mode){
	if(en_di_mode == ENABLE){
			if(IRQ_Number <= 31){
				*NVIC_ISER0 |= (1 << IRQ_Number);
			}else if(IRQ_Number > 31 && IRQ_Number < 64){
				*NVIC_ISER1 |= (1 << (IRQ_Number % 32));
			}else if(IRQ_Number > 64 && IRQ_Number < 96){
				*NVIC_ISER2 |= (1 << (IRQ_Number % 64));
			}
		}else{
			if(IRQ_Number <= 31){
				*NVIC_ICER0 |= (1 << IRQ_Number);
			}else if(IRQ_Number > 31 && IRQ_Number < 64){
				*NVIC_ICER1 |= (1 << (IRQ_Number % 32));
			}else if(IRQ_Number > 64 && IRQ_Number < 96){
				*NVIC_ICER2 |= (1 << (IRQ_Number % 64));
			}
		}
}

/*****************************************************************
 * @fn              I2C_IRQ_PriorityConfig
 *
 * @brief           This function sets the priority of required IRQ number for I2C peripheral
 *
 * @param[in]       IRQ number
 * @param[in]       IRQ priority number
 *
 * @return          none
 */
void I2C_IRQ_PriorityConfig(uint8_t IRQ_Number, uint8_t IRQ_Priority){
	if(IRQ_Priority > 15) IRQ_Priority = 15;

		uint8_t reg_index = IRQ_Number / 4;
		uint8_t offset = IRQ_Number % 4;
		uint8_t shift_amount = (8 * offset) + (8 - PRIOR_BITS_IMPLEMENTED);

		*(NVIC_IRQ_PRIOR_BASE + (reg_index * 4)) &= ~(0xFFUL << (offset * 8));
		*(NVIC_IRQ_PRIOR_BASE + (reg_index * 4)) |= (IRQ_Priority << shift_amount);
}

/*****************************************************************
 * @fn              I2C_Master_Transmit_IT
 *
 * @brief           This function sends the data from master to slave with IRQ
 *
 * @param[in]    Pointer to the structure which contains I2C specified configuration settings
 * @param[in]    Pointer to the TX buffer with data which we want to send
 * @param[in]    Length of the data
 * @param[in]    Address of the slave device
 * @param[in]    Enable/Disable repeated start
 *
 * @return       T or F
 */
uint8_t I2C_Master_Transmit_IT(I2C_Handle_t *pI2C_Handle, uint8_t *pTxBuffer, uint8_t len, uint8_t SlaveAddr, uint8_t sr){
	uint8_t busystate = pI2C_Handle->TxRxState;

	if((busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX)){
		pI2C_Handle->pTxBuffer = pTxBuffer;
		pI2C_Handle->TxLen = len;
		pI2C_Handle->TxRxState = I2C_BUSY_IN_TX;
		pI2C_Handle->DevAddr = SlaveAddr;
		pI2C_Handle->Sr = sr;

		//Enable ITBUFEN control bit
		pI2C_Handle->pI2Cx->CR2 |= (1 << I2C_CR2_ITBUFEN);

		//Enable ITEVTEN control bit
		pI2C_Handle->pI2Cx->CR2 |= (1 << I2C_CR2_ITEVTEN);

		//Enable ITERREN control bit
		pI2C_Handle->pI2Cx->CR2 |= (1 << I2C_CR2_ITERREN);

		I2C_GenerateStartCondition(pI2C_Handle->pI2Cx);
	}

	return busystate;
}

/*****************************************************************
 * @fn              I2C_Master_Receive_IT
 *
 * @brief        This function receives the data from slave to master with IRQ
 *
 * @param[in]    Pointer to the structure which contains I2C specified configuration settings
 * @param[in]    Pointer to the TX buffer with data which we want to send
 * @param[in]    Length of the data
 * @param[in]    Address of the slave device
 * @param[in]    Enable/Disable repeated start
 *
 * @return       T or F
 */
uint8_t I2C_Master_Receive_IT(I2C_Handle_t *pI2C_Handle, uint8_t *pRxBuffer, uint8_t len, uint8_t SlaveAddr, uint8_t sr){
	uint8_t busystate = pI2C_Handle->TxRxState;

	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
	{
		pI2C_Handle->pRxBuffer = pRxBuffer;
		pI2C_Handle->RxLen = len;
		pI2C_Handle->TxRxState = I2C_BUSY_IN_RX;
		pI2C_Handle->RxSize = len; //RxSize is used in the ISR code to manage the data reception
		pI2C_Handle->DevAddr = SlaveAddr;
		pI2C_Handle->Sr = sr;

		pI2C_Handle->pI2Cx->CR2 |= (1 << I2C_CR2_ITBUFEN);
		pI2C_Handle->pI2Cx->CR2 |= (1 << I2C_CR2_ITEVTEN);
		pI2C_Handle->pI2Cx->CR2 |= (1 << I2C_CR2_ITERREN);

		I2C_GenerateStartCondition(pI2C_Handle->pI2Cx);
	}

	return busystate;
}

/*********************************************************
 * @fn             I2C_IRQ_EV_Handler
 *
 * @brief          This function configures IRQ via events
 *
 * @param[in]     Pointer to the structure with I2C peripheral register addresses and some parameters
 *
 * @return        none
 */
void I2C_IRQ_EV_Handler(I2C_Handle_t *pI2C_Handle){
	//Interrupt handling for both master and slave mode of a device
	uint32_t tmp1, tmp2, tmp3;

	tmp1 = pI2C_Handle->pI2Cx->CR2 & (1 << I2C_CR2_ITEVTEN);
	tmp2 = pI2C_Handle->pI2Cx->CR2 & (1 << I2C_CR2_ITBUFEN);
	tmp3 = pI2C_Handle->pI2Cx->SR1 & (1 << I2C_SR1_SB);

	//handle for interrupt generated by SB event (SB flag is only applicable in master mode)
	if(tmp1 && tmp3){
		//SB flag is set
		if(pI2C_Handle->TxRxState == I2C_BUSY_IN_TX){
			I2C_ExecuteAddressPhase_TX(pI2C_Handle->pI2Cx, pI2C_Handle->DevAddr);
		}else if(pI2C_Handle->TxRxState == I2C_BUSY_IN_RX){
			I2C_ExecuteAddressPhase_RX(pI2C_Handle->pI2Cx, pI2C_Handle->DevAddr);
		}
	}

	//handle for interrupt generated by ADDR event (master - address is set, slave - address matched with own address)
	tmp3 = pI2C_Handle->pI2Cx->SR1 & (1 << I2C_SR1_ADDR);

	if(tmp1 && tmp3){
		//ADDR flag is set
		//Clear the ADDR flag
		I2C_ClearADDRFlag(pI2C_Handle);
		I2C_ApplicationEventCallback(pI2C_Handle, I2C_EV_ADDR_MATCH);
	}

	//handle for interrupt generated by BTF event
	tmp3 = pI2C_Handle->pI2Cx->SR1 & (1 << I2C_SR1_BTF);

	if(tmp1 && tmp3){
	    if(pI2C_Handle->TxRxState == I2C_BUSY_IN_TX){
	        if(pI2C_Handle->TxLen == 0){
	            if(pI2C_Handle->pI2Cx->SR1 & (1 << I2C_SR1_TXE)){
	                if(pI2C_Handle->Sr == I2C_SR_DI){
	                    I2C_GenerateStopCondition(pI2C_Handle->pI2Cx);
	                }
	                I2C_CloseTransmission(pI2C_Handle);
	                I2C_ApplicationEventCallback(pI2C_Handle, I2C_EV_TX_CMPLT);
	            }
	        }
	    }else if(pI2C_Handle->TxRxState == I2C_BUSY_IN_RX){
	        ;
	    }
	}

	//handle for interrupt generated by STOPF event (available only for slave mode)
	tmp3 = pI2C_Handle->pI2Cx->SR1 & (1 << I2C_SR1_STOPF);

	if(tmp1 && tmp3){
		//STOPF flag is set

		//Clear the STOPF flag (SR1 is read and we need to write smt into CR1 register
		I2C_ClearSTOPF(pI2C_Handle);

		 I2C_ApplicationEventCallback(pI2C_Handle, I2C_EV_STOP);
	}

	//handle for interrupt generated by TXE event
	tmp3 = pI2C_Handle->pI2Cx->SR1 & (1 << I2C_SR1_TXE);

	if(tmp1 && tmp2 && tmp3){
	    //TXE flag is set

	    if(pI2C_Handle->pI2Cx->SR2 & (1 << I2C_SR2_MSL)){
	        //master
	        if(pI2C_Handle->TxRxState == I2C_BUSY_IN_TX){
	            if(pI2C_Handle->TxLen > 0){
	                pI2C_Handle->pI2Cx->DR = *(pI2C_Handle->pTxBuffer);
	                pI2C_Handle->TxLen--;
	                pI2C_Handle->pTxBuffer++;
	            }
	        }
	    }else{
	        //slave
	        if(pI2C_Handle->pI2Cx->SR2 & (1 << I2C_SR2_TRA)){
	            I2C_ApplicationEventCallback(pI2C_Handle, I2C_EV_DATA_REQ);
	        }
	    }
	}

	//handle for interrupt generated by RXNE event
	tmp3 = pI2C_Handle->pI2Cx->SR1 & (1 << I2C_SR1_RXNE);

	if(tmp1 && tmp2 && tmp3){
	    //RXNE flag is set

	    if(pI2C_Handle->pI2Cx->SR2 & (1 << I2C_SR2_MSL)){
	        //master
	        if(pI2C_Handle->TxRxState == I2C_BUSY_IN_RX){
	            if(pI2C_Handle->RxLen == 1){
	                *pI2C_Handle->pRxBuffer = pI2C_Handle->pI2Cx->DR;
	                pI2C_Handle->RxLen--;
	            }

	            if(pI2C_Handle->RxLen > 1){
	                if(pI2C_Handle->RxLen == 2){
	                    I2C_AckControl(pI2C_Handle->pI2Cx, I2C_ACK_DISABLE);
	                }
	                *pI2C_Handle->pRxBuffer = pI2C_Handle->pI2Cx->DR;
	                pI2C_Handle->pRxBuffer++;
	                pI2C_Handle->RxLen--;
	            }

	            if(pI2C_Handle->RxLen == 0){
	                if(pI2C_Handle->Sr == I2C_SR_DI){
	                    I2C_GenerateStopCondition(pI2C_Handle->pI2Cx);
	                }
	                I2C_CloseReception(pI2C_Handle);
	                I2C_ApplicationEventCallback(pI2C_Handle, I2C_EV_RX_CMPLT);
	            }
	        }
	    }else{
	        //slave
	        if(!(pI2C_Handle->pI2Cx->SR2 & (1 << I2C_SR2_TRA))){
	            I2C_ApplicationEventCallback(pI2C_Handle, I2C_EV_DATA_RCV);
	        }
	    }
	}
}

/*********************************************************
 * @fn             I2C_IRQ_ER_Handler
 *
 * @brief          This function configures IRQ via errors
 *
 * @param[in]     Pointer to the structure with I2C peripheral register addresses and some parameters
 *
 * @return        none
 */
void I2C_IRQ_ER_Handler(I2C_Handle_t *pI2C_Handle){
	uint32_t tmp1, tmp2;

	//know the status of ITERREN control bit
	tmp2 = (pI2C_Handle->pI2Cx->CR2) & ( 1 << I2C_CR2_ITERREN);

	//check for bus error
	tmp1 = (pI2C_Handle->pI2Cx->SR1) & ( 1 << I2C_SR1_BERR);

	if(tmp1 && tmp2){
		//this is bus error

		//clear the bus error flag
		pI2C_Handle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_BERR);

		//notify the application about the error
		I2C_ApplicationEventCallback(pI2C_Handle, I2C_ERROR_BERR);
	}

	//check for arbitration lost error
	tmp1 = (pI2C_Handle->pI2Cx->SR1) & ( 1 << I2C_SR1_ARLO );
	if(tmp1 && tmp2){
		//this is ARLO

		//clear the ARLO flag
		pI2C_Handle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_ARLO);

		//notify the application
		I2C_ApplicationEventCallback(pI2C_Handle, I2C_ERROR_ARLO);
	}

	//check for ACK failure
	tmp1 = (pI2C_Handle->pI2Cx->SR1) & ( 1 << I2C_SR1_AF);
	if(tmp1 && tmp2){
		//this is an ACK failure

		//clear the ACK flag
		pI2C_Handle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_AF);

		//notify the application
		I2C_ApplicationEventCallback(pI2C_Handle, I2C_ERROR_AF);
	}

	//check for over/underrun error
	tmp1 = (pI2C_Handle->pI2Cx->SR1) & ( 1 << I2C_SR1_OVR);
	if(tmp1 && tmp2){
		//overrun/underrun error

		//clear the OVR flag
		pI2C_Handle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_OVR);

		//notify the application
		I2C_ApplicationEventCallback(pI2C_Handle, I2C_ERROR_OVR);
	}

	//check for timeout error
	tmp1 = (pI2C_Handle->pI2Cx->SR1) & ( 1 << I2C_SR1_TIMEOUT);
	if(tmp1  && tmp2){
		//timeout error

		//clear the TIMEOUT flag
		pI2C_Handle->pI2Cx->SR1 &= ~( 1 << I2C_SR1_TIMEOUT);

		//notify the application
		I2C_ApplicationEventCallback(pI2C_Handle, I2C_ERROR_TIMEOUT);
	}
}

/*******************************************************************
 * @fn           I2C_Slave_Transmit
 *
 * @brief        This function sends the data from slave to master
 *
 * @param[in]    Pointer to the structure with I2C peripheral register base addresses
 * @param[in]    required data
 *
 * @return       none
 */
void I2C_Slave_Transmit(I2C_REG_t *pI2Cx, uint8_t data){
	pI2Cx->DR = data;
}

/********************************************************************
 * @fn            I2C_Slave_Receive
 *
 * @brief         This function returns the value from the DR
 *
 * @param         Pointer to the structure with I2C peripheral register base addresses
 *
 * return         data from the DR
 */
uint8_t I2C_Slave_Receive(I2C_REG_t *pI2Cx){
	return pI2Cx->DR;
}

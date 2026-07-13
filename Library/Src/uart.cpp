/*
 * uart.cpp
 *
 *  Created on: Feb 3, 2026
 *      Author: wataoxp
 */

#include "uart.h"

using namespace Serial;

UART::UART(USART_TypeDef *UARTPort) :USARTx(UARTPort)
{
	;
}

inline void UART::CR1Config(UART_InitTypedef *init)
{
	uint32_t value = init->DataWidth | init->Parity | init->Direction | init->OverSampling;

	MODIFY_REG(USARTx->CR1
			,(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | USART_CR1_RE | USART_CR1_OVER8)
			,value);
}

void UART::Config(UART_InitTypedef *init,uint32_t SysClk)
{
	if(USARTx == USART1)
	{
		LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
	}
	else if(USARTx == USART2)
	{
		// G0ではUSART2のクロックソースは選択不可(PCLKのみ)
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
	}

	LL_USART_Disable(USARTx);

	CR1Config(init);

	LL_USART_SetStopBitsLength(USARTx, init->StopBits);
	LL_USART_SetHWFlowCtrl(USARTx, init->HardWareControl);

	// 直接クロック値を渡す
//	uint32_t clk = LL_RCC_GetUSARTClockFreq(LL_RCC_USART1_CLKSOURCE);
	uint32_t clk = SysClk;

	if(clk != 0)
	{
		LL_USART_SetBaudRate(USARTx, clk, init->PreScalerDiv,
				init->OverSampling, init->BaudRate);
	}

	LL_USART_SetPrescaler(USARTx, init->PreScalerDiv);

	if(init->FifoEN == SerialParameters::Fifo_Enable)
	{
		LL_USART_SetTXFIFOThreshold(USARTx, LL_USART_FIFOTHRESHOLD_1_8);
		LL_USART_SetRXFIFOThreshold(USARTx, LL_USART_FIFOTHRESHOLD_1_8);
		LL_USART_EnableFIFO(USARTx);
	}

	LL_USART_DisableFIFO(USARTx);

	if(init->SelectMode == SerialParameters::AsyncMode)
	{
		LL_USART_ConfigAsyncMode(USARTx);
	}

	LL_USART_Enable(USARTx);

	WaitEnableUART(init->Direction);
}

void UART::WaitEnableUART(uint32_t Direction)
{
	if(Direction == LL_USART_DIRECTION_TX_RX)
	{
		while((LL_USART_IsActiveFlag_TEACK(USARTx) != 1) || (LL_USART_IsActiveFlag_REACK(USARTx) != 1));
	}
	else if(Direction == LL_USART_DIRECTION_TX)
	{
		while(LL_USART_IsActiveFlag_TEACK(USARTx) != 1);
	}
	else
	{
		while(LL_USART_IsActiveFlag_REACK(USARTx) != 1);
	}
}

void UART::TransmitData(uint8_t *buf,uint8_t size)
{
	uint8_t *buffer = buf;

	for(uint32_t i = 0;i < size;i++)
	{
		LL_USART_TransmitData9(USARTx, buffer[i]);
		while(LL_USART_IsActiveFlag_TXE_TXFNF(USARTx) == 0);
	}
	LL_USART_ClearFlag_TC(USARTx);
	while(LL_USART_IsActiveFlag_TC(USARTx) == 0);
}

uint16_t UART::ReceiveData(void)
{
	while(LL_USART_IsActiveFlag_RXNE_RXFNE(USARTx) == 0);

	return LL_USART_ReceiveData9(USARTx);
}

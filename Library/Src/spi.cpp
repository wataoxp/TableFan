/*
 * spi.c
 *
 *  Created on: Feb 21, 2025
 *      Author: wataoxp
 */
#include "spi.h"

SPI::SPI(SPI_TypeDef *SPIPORT) :SPIx(SPIPORT)
{
	;
}
void SPI::Config(SPI_InitTypedef *pConfig)
{
	if(LL_SPI_IsEnabled(SPIx))
	{
		LL_SPI_Disable(SPIx);
	}

	if(SPIx == SPI1)
	{
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
	}
#ifdef STM32G0xx
	else
	{
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
	}
#endif
	LL_SPI_SetTransferDirection(SPIx, pConfig->TransferDirection);
	LL_SPI_SetClockPolarity(SPIx, pConfig->ClockPolarity);
	LL_SPI_SetClockPhase(SPIx, pConfig->ClockPhase);
	LL_SPI_SetTransferBitOrder(SPIx, pConfig->BitOrder);
	LL_SPI_SetBaudRatePrescaler(SPIx, pConfig->BaudRate);
	LL_SPI_SetMode(SPIx, pConfig->Mode);

	LL_SPI_SetDataWidth(SPIx, pConfig->DataWidth);
	LL_SPI_SetNSSMode(SPIx, pConfig->NSS);

    if(pConfig->DataWidth < LL_SPI_DATAWIDTH_9BIT)		//データサイズが8ビット以下ならFRXTHをセットする
    {
    	LL_SPI_SetRxFIFOThreshold(SPIx, LL_SPI_RX_FIFO_TH_QUARTER);
    }
    LL_SPI_DisableCRC(SPIx);
    LL_I2S_Disable(SPIx);								//I2SとSPIは排他的な関係

	LL_SPI_SetStandard(SPIx, LL_SPI_PROTOCOL_MOTOROLA);
	LL_SPI_DisableNSSPulseMgt(SPIx);
}

/*** SPI Tx&Rx ***/
uint32_t SPI::MasterTransmit(uint8_t *data,uint16_t length)
{
	while(LL_SPI_IsActiveFlag_BSY(SPIx) != 0);

	for(uint16_t i = 0;i < length;i++)
	{
		while(LL_SPI_IsActiveFlag_TXE(SPIx) == 0);
		LL_SPI_TransmitData8(SPIx, data[i]);
	}
	while(LL_SPI_IsActiveFlag_BSY(SPIx) != 0);

	return LL_SPI_GetTxFIFOLevel(SPIx);
}

//スレーブモード時はSPI有効をこの関数が呼ばれる前に実行しておくこと
void SPI::Receive(uint8_t *RXbuf,uint16_t length)
{
	LL_SPI_Enable(SPIx);

	while(LL_SPI_IsActiveFlag_BSY(SPIx) != 0);

	for(uint16_t i = 0;i < length;i++)
	{
		while(LL_SPI_IsActiveFlag_TXE(SPIx) == 0);
		while(LL_SPI_IsActiveFlag_RXNE(SPIx) == 0);
		LL_SPI_TransmitData8(SPIx, 0);
		RXbuf[i] = LL_SPI_ReceiveData8(SPIx);
	}
	while(LL_SPI_IsActiveFlag_BSY(SPIx) != 0);
	while(LL_SPI_GetTxFIFOLevel(SPIx) != 0);

	LL_SPI_Disable(SPIx);
}

#if 0
/*** Arduino SPI ***/
uint8_t SPI::Transfer(uint8_t data)
{
	LL_SPI_TransmitData8(SPIx, data);
	while(LL_SPI_IsActiveFlag_TXE(SPIx) == 0);
	while(LL_SPI_IsActiveFlag_RXNE(SPIx) == 0);

	return LL_SPI_ReceiveData8(SPIx);
}

void SPI::ClearFIFO(void)
{
	while(LL_SPI_IsActiveFlag_BSY(SPIx) != 0);
	while(LL_SPI_GetRxFIFOLevel(SPIx) != LL_SPI_RX_FIFO_EMPTY)
	{
		LL_SPI_ReceiveData8(SPIx);
	}
}

#endif

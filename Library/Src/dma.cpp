/*
 * dma.cpp
 *
 *  Created on: Jun 8, 2025
 *      Author: wataoxp
 */

#include "dma.h"

DMA::DMA(DMA_TypeDef *DMAport) :DMAx(DMAport)
{
	;
}

void DMA::Config(DMA_InitTypdef *pConfig,uint32_t Channel)
{
	if(LL_AHB1_GRP1_IsEnabledClock(LL_AHB1_GRP1_PERIPH_DMA1) == 0)		//DMA1しかない
	{
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
	}
	LL_DMA_SetPeriphRequest(DMAx, Channel, pConfig->RequestID);	//DMAMUX,REQID
	LL_DMA_SetDataTransferDirection(DMAx, Channel, pConfig->Direction);
	LL_DMA_SetChannelPriorityLevel(DMAx, Channel, LL_DMA_PRIORITY_HIGH);
	LL_DMA_SetMode(DMAx, Channel, pConfig->TransferMode);			//Normal or Circular

	LL_DMA_SetChannelPriorityLevel(DMAx,Channel,pConfig->Priority);

	LL_DMA_SetPeriphIncMode(DMAx, Channel, pConfig->PeriphInc);
	LL_DMA_SetMemoryIncMode(DMAx, Channel, pConfig->MemoryInc);

	LL_DMA_SetPeriphSize(DMAx, Channel, pConfig->PeriphSize);
	LL_DMA_SetMemorySize(DMAx, Channel, pConfig->MemorySize);
}

void DMA::SetISR(uint32_t Channel)
{
	LL_DMA_DisableChannel(DMAx, Channel);
	LL_DMA_EnableIT_TE(DMAx, Channel);
	LL_DMA_EnableIT_TC(DMAx, Channel);
	LL_DMA_DisableIT_HT(DMAx, Channel);
}

uint32_t DMA::AddressSet(uint32_t Channel,uint32_t *MemoryAddress,uint32_t *PeriphAddress)
{
	if(MemoryAddress == nullptr || PeriphAddress == nullptr)
	{
		return 1;
	}

	LL_DMA_SetMemoryAddress(DMAx, Channel, (uint32_t)MemoryAddress);
	LL_DMA_SetPeriphAddress(DMAx, Channel, (uint32_t)PeriphAddress);	//レジスタのアドレスを渡すので32ビット

	return 0;
}

uint32_t DMA::AddressSetM2M(uint32_t Channel,uint8_t *SrcAddress,uint8_t *DstAddress)
{
	if(SrcAddress == nullptr || DstAddress == nullptr)
	{
		return 1;
	}

	//メモリ間通信はバッファのサイズに合わせる
	LL_DMA_SetM2MSrcAddress(DMAx, Channel, (uint32_t)SrcAddress);
	LL_DMA_SetM2MDstAddress(DMAx, Channel, (uint32_t)DstAddress);

	return 0;
}

/**** SPI DMA ****/
//void DMA::SPItoDMA(SPI_TypeDef *SPIx,uint8_t *MemoryAddress)
//{
//	LL_SPI_EnableDMAReq_TX(SPIx);
//	AddressSet(MemoryAddress, (uint32_t*)LL_SPI_DMA_GetRegAddr(SPIx));
//
//	if(LL_SPI_IsEnabled(SPIx) == 0)
//	{
//		LL_SPI_Enable(SPIx);
//	}
//}

/**** DMA Func ***/

void DMA::StartDMA(uint32_t Channel,uint32_t length)
{
	LL_DMA_SetDataLength(DMAx, Channel, length);		//転送完了後は0になっているので再セット
	LL_DMA_EnableChannel(DMAx, Channel);
}

uint32_t DMA::StopDMA(uint32_t Channel)
{
	uint32_t ret = 0;

	switch(Channel)
	{
	case LL_DMA_CHANNEL_1:
		ret = LL_DMA_IsActiveFlag_TE1(DMAx);
		break;
	case LL_DMA_CHANNEL_2:
		ret = LL_DMA_IsActiveFlag_TE2(DMAx);
		break;
	default:
		return 2;
		break;
	}
	LL_DMA_DisableChannel(DMAx, Channel);

	return ret;
}

uint32_t DMA::StopDMAisChannel1(void)
{
	while(LL_DMA_IsActiveFlag_TC1(DMAx) == 0);
	LL_DMA_ClearFlag_TC1(DMAx);
	LL_DMA_DisableChannel(DMAx, LL_DMA_CHANNEL_1);

	return LL_DMA_IsActiveFlag_TE1(DMAx);
}

uint32_t DMA::StopDMAisChannel2(void)
{
	while(LL_DMA_IsActiveFlag_TC2(DMAx) == 0);
	LL_DMA_ClearFlag_TC2(DMAx);
	LL_DMA_DisableChannel(DMAx, LL_DMA_CHANNEL_2);

	return LL_DMA_IsActiveFlag_TE2(DMAx);
}

#if 0
void DMA::Config(uint32_t Request,uint32_t Channel)
{
	if(LL_AHB1_GRP1_IsEnabledClock(LL_AHB1_GRP1_PERIPH_DMA1) == 0)		//DMA1しかない
	{
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
	}
	LL_DMA_SetPeriphRequest(DMAx, Channel, Request);	//DMAMUX,REQID
	LL_DMA_SetDataTransferDirection(DMAx, Channel, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
	LL_DMA_SetChannelPriorityLevel(DMAx, Channel, LL_DMA_PRIORITY_HIGH);
	LL_DMA_SetMode(DMAx, Channel, LL_DMA_MODE_NORMAL);			//Normal or Circular

	LL_DMA_SetPeriphIncMode(DMAx, Channel, LL_DMA_PERIPH_NOINCREMENT);
	LL_DMA_SetMemoryIncMode(DMAx, Channel, LL_DMA_MEMORY_INCREMENT);

	LL_DMA_SetPeriphSize(DMAx, Channel, LL_DMA_PDATAALIGN_WORD);
	LL_DMA_SetMemorySize(DMAx, Channel, LL_DMA_MDATAALIGN_BYTE);
}
//タイマー設定も行っていたもの。tim.cppに分離
uint32_t DMA::TIMtoDMA(TIM_TypeDef *TIMx,uint32_t PWMchannel,uint8_t *MemoryAddress)
{
	__IO uint32_t *Address;

	LL_DMA_SetPeriphRequest(DMAx, Channel, LL_DMAMUX_REQ_TIM2_UP);	//DMAMUX,DMAトリガをカウンタ更新時に設定
	LL_TIM_CC_SetDMAReqTrigger(TIMx, LL_TIM_CCDMAREQUEST_UPDATE);	//TIM,カウンタ更新時にDMAリクエスト
	LL_TIM_EnableDMAReq_UPDATE(TIMx);								//TIM,カウンタ更新時DMA要求を許可

	if(LL_TIM_IsEnabledCounter(TIMx) == 0)
	{
		LL_TIM_EnableCounter(TIMx);
	}
	if(LL_TIM_CC_IsEnabledChannel(TIMx, PWMchannel) == 0)
	{
		LL_TIM_CC_EnableChannel(TIMx, PWMchannel);
	}

	Address = CheckPWMChannel(TIMx, PWMchannel);
	if(Address == nullptr)
	{
		return 1;
	}
	AddressSet(MemoryAddress, (uint32_t*)Address);		//__IOを外す

	return 0;
}


__IO uint32_t* DMA::CheckPWMChannel(TIM_TypeDef *TIMx,uint32_t PWMchannel)
{
	__IO uint32_t *Reg;			//各レジスタは__IO付きなので
	uint32_t ccrx;
	uint32_t *ccgx;

	switch(PWMchannel)
	{
	case LL_TIM_CHANNEL_CH1:
		Reg = &TIMx->CCR1;
		ccrx = (uint32_t)&TIMx->CCR1;
		ccgx = (uint32_t*)&TIMx->CCR1;
		break;
	case LL_TIM_CHANNEL_CH2:
		Reg = &TIMx->CCR2;
		break;
	case LL_TIM_CHANNEL_CH3:
		Reg = &TIMx->CCR3;
		break;
	case LL_TIM_CHANNEL_CH4:
		Reg = &TIMx->CCR4;
		break;
	default:
		Reg = nullptr;
	}
	return Reg;
}
#endif







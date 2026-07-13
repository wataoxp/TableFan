/*
 * rcc.c
 *
 *  Created on: Jan 22, 2025
 *      Author: wataoxp
 */
#include "rcc.h"

/* HighSpeedClock */

#if defined(STM32G0)

static inline void SetSystemClockSource(uint32_t src)
{
	LL_RCC_SetSysClkSource(src);
	// src << 3がチェックビット
	while(LL_RCC_GetSysClkSource() != (src << 3));
}
void ConfigPLL(RCC_InitTypedef *rcc)
{
	LL_RCC_PLL_Disable();	// PLL設定前に必ず無効化する

	LL_RCC_PLL_ConfigDomain_SYS(rcc->PLLSrc, rcc->PLLM, rcc->PLLN, rcc->PLLR);
	LL_RCC_PLL_Enable();
	LL_RCC_PLL_EnableDomain_SYS();
	while(LL_RCC_PLL_IsReady() != 1);

	SetSystemClockSource(rcc->SysClkSrc);
}
void ConfigHighClock(RCC_InitTypedef *rcc,rccStatus src)
{
	LL_FLASH_SetLatency(rcc->Latency);

	if(src == rccStatus::InternalClock)
	{
		LL_RCC_HSI_Enable();
		while(LL_RCC_HSI_IsReady() != 1);
	}
	else if(src == rccStatus::ExternalClock)
	{
		LL_RCC_HSE_Enable();
		while(LL_RCC_HSE_IsReady() != 1);
	}
	SetSystemClockSource(rcc->SysClkSrc);

	/* SysTick Clock */
	LL_RCC_SetAHBPrescaler(rcc->AHBdiv);

	LL_RCC_SetAPB1Prescaler(rcc->APBdiv);

}

#elif defined(STM32C0)

void RCC_InitC0(RCC_InitTypedef *rcc)
{
	LL_FLASH_SetLatency(rcc->Latency);
	while(LL_FLASH_GetLatency() != rcc->Latency);

	LL_RCC_HSI_Enable();
	while(LL_RCC_HSI_IsReady() != 1);

	//ここの値でクロックの精度が変わる。デフォは64
	LL_RCC_HSI_SetCalibTrimming(64);
	LL_RCC_SetHSIDiv(rcc->HSIdiv);
	LL_RCC_SetAHBPrescaler(rcc->AHBdiv);

	LL_RCC_SetSysClkSource(rcc->SysClkSrc);
	while(LL_RCC_GetSysClkSource() != rcc->SysClkSrc);

	LL_RCC_SetAPB1Prescaler(rcc->APBdiv);
}
#endif

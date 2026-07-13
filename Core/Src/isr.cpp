/*
 * isr.cpp
 *
 *  Created on: Mar 7, 2026
 *      Author: wataoxp
 */


#include "isr.h"
#include "debounce.h"

static isrFlags iFlag;

/* Getter/Setter */
__IO const isrFlags& GetISRStruct(void)
{
	return (__IO const isrFlags&)iFlag;
}

void ResetExtCommand(void)
{
	iFlag.ExtCommand = 0;
}

void ResetIntCommand(void)
{
	iFlag.IntCommand = 0;
}

/* Interrupt Service Routine */

extern "C"{

void SysTick_Handler(void)
{
	static __IO uint32_t mTick;

	if(mTick++ < Debounce::GetInterval())
	{
		return;
	}
	mTick = 0;
	Debounce::ButtonFilter(iFlag.ExtCommand);
}

void TIM16_IRQHandler(void)
{
	if(LL_TIM_IsActiveFlag_UPDATE(TIM16))
	{
		LL_TIM_ClearFlag_UPDATE(TIM16);
		iFlag.IntCommand |= 1;
	}
}

// <cassert>の上書き
void  __assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
    (void)file;			// アサート発生ファイル名
    (void)line;			// アサート発生行
    (void)func;			// アサート発生関数名
    (void)failedexpr;	// アサート発生条件

    __asm volatile("bkpt #0");

    while (1);
}

void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVC_IRQn 0 */

  /* USER CODE END SVC_IRQn 0 */
  /* USER CODE BEGIN SVC_IRQn 1 */

  /* USER CODE END SVC_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

}// extern "C"


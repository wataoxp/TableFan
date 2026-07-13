/*
 * isr.h
 *
 *  Created on: Mar 7, 2026
 *      Author: wataoxp
 */

#ifndef INC_ISR_H_
#define INC_ISR_H_

#include "debounce.h"

typedef struct{
	uint32_t ExtCommand;
	uint32_t IntCommand;
}isrFlags;

__IO const isrFlags& GetISRStruct(void);

void ResetExtCommand(void);
void ResetIntCommand(void);

extern "C" {

void SysTick_Handler(void);
void EXTI4_15_IRQHandler(void);
void TIM3_TIM4_IRQHandler(void);
void RTC_TAMP_IRQHandler(void);

void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);

}



#endif /* INC_ISR_H_ */

/*
 * periph.h
 *
 *  Created on: Feb 17, 2025
 *      Author: wataoxp
 */

#ifndef SRC_PERIPH_H_
#define SRC_PERIPH_H_

#if defined(__has_include)
	#if __has_include("stm32g0xx.h")
	#include "stm32g0xx.h"

	#include "stm32g0xx_ll_dac.h"
	#include "stm32g0xx_ll_dma.h"
	#include "stm32g0xx_ll_dmamux.h"
	#include "stm32g0xx_ll_exti.h"
	#include "stm32g0xx_ll_gpio.h"
	#include "stm32g0xx_ll_i2c.h"
	#include "stm32g0xx_ll_iwdg.h"
	#include "stm32g0xx_ll_lptim.h"
	#include "stm32g0xx_ll_lpuart.h"
	#include "stm32g0xx_ll_pwr.h"
	#include "stm32g0xx_ll_rcc.h"
	#include "stm32g0xx_ll_rng.h"
	#include "stm32g0xx_ll_rtc.h"
	#include "stm32g0xx_ll_spi.h"
	#include "stm32g0xx_ll_system.h"
	#include "stm32g0xx_ll_tim.h"
	#include "stm32g0xx_ll_ucpd.h"
	#include "stm32g0xx_ll_usart.h"
	#include "stm32g0xx_ll_utils.h"
	#include "stm32g0xx_ll_wwdg.h"
	#include "stm32g0xx_ll_adc.h"
	#include "stm32g0xx_ll_bus.h"
	#include "stm32g0xx_ll_comp.h"
	#include "stm32g0xx_ll_cortex.h"
	#include "stm32g0xx_ll_crc.h"
	#include "stm32g0xx_ll_crs.h"

	#elif __has_include("stm32c0xx.h")
	#include "stm32c0xx.h"

	#include "stm32c0xx_ll_gpio.h"
	#include "stm32c0xx_ll_i2c.h"
	#include "stm32c0xx_ll_iwdg.h"
	#include "stm32c0xx_ll_pwr.h"
	#include "stm32c0xx_ll_rcc.h"
	#include "stm32c0xx_ll_rtc.h"
	#include "stm32c0xx_ll_spi.h"
	#include "stm32c0xx_ll_system.h"
	#include "stm32c0xx_ll_tim.h"
	#include "stm32c0xx_ll_usart.h"
	#include "stm32c0xx_ll_utils.h"
	#include "stm32c0xx_ll_wwdg.h"
	#include "stm32c0xx_ll_adc.h"
	#include "stm32c0xx_ll_bus.h"
	#include "stm32c0xx_ll_cortex.h"
	#include "stm32c0xx_ll_crc.h"
	#include "stm32c0xx_ll_crs.h"
	#include "stm32c0xx_ll_dma.h"
	#include "stm32c0xx_ll_dmamux.h"
	#include "stm32c0xx_ll_exti.h"

	#else
	#error "No supported STM32 LL headers found."

	#endif
#endif

typedef enum{
	HSICLOCK = 16,
	PLLCLOCK = 64,
	C0xHSI	 = 48,
	C0xDivHSI = (48/4),
}CoreClock;

typedef enum{
	Pin0,
	Pin1,
	Pin2,
	Pin3,
	Pin4,
	Pin5,
	Pin6,
	Pin7,
	Pin8,
	Pin9,
	Pin10,
	Pin11,
	Pin12,
	Pin13,
	Pin14,
	Pin15,
}GPIO_Pin;

typedef enum{
	PORTA = 0,
	PORTB = 1,
	PORTC = 2,
	PORTD = 3,
	PORTE = 4,
	PORTF = 5,
	NoPort = UINT8_MAX,
}GPIO_Port;

typedef enum{
	HighPriority = 0,
	MidPriority,
	LowPriority,
}NVIC_Prioritys;

#define GPIO_WRITE(GPIOx,Pin) (GPIOx->BSRR = 1 << Pin)
#define GPIO_CLEAR(GPIOx,Pin) (GPIOx->BRR = 1 << Pin)
#define GPIO_READ(GPIOx,Pin) (GPIOx->IDR & 1 << Pin)

#endif /* SRC_PERIPH_H_ */

/*
 * exti.h
 *
 *  Created on: Jan 28, 2025
 *      Author: wataoxp
 */

#ifndef INC_EXTI_H_
#define INC_EXTI_H_

#include "gpio.h"
#include "periph.h"

namespace ExtiCommands{
	constexpr uint32_t EXTICR_Mask = EXTI_EXTICR1_EXTI0_Msk;

	typedef enum{
		success,
		NotPort,
		NotPin,
	}ExtiReturnCode;

}

class EXTIR{
private:
	GPIO_TypeDef *GPIOx;
	uint32_t PinPos;

	GPIO_Port CheckPort(void);
	uint32_t SetSource(GPIO_Port port);
	void ExtiMode(uint8_t Mode,uint32_t Line);
	void ExtiTrigger(uint8_t Trigger,uint32_t Line);
public:
	EXTIR(GPIO_TypeDef *GPIOPORT,uint32_t pin);
	uint32_t Config(void);
	void ConfigMode_Trigger(uint8_t Mode,uint8_t Trigger);
};

#endif /* INC_EXTI_H_ */

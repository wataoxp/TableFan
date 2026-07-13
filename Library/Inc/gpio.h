/*
 * gpio.h
 *
 *  Created on: Apr 11, 2025
 *      Author: wataoxp
 */

#ifndef GPIO_H_
#define GPIO_H_

#include "periph.h"

namespace GPIOs{
	typedef enum{
		Success,
		Failed,
		nullPort,
		SWDPin,
	}GPIO_Code;
}

typedef struct{
	uint32_t Mode;
	uint32_t Speed;
	uint32_t OutputType;
	uint32_t Pull;
	uint32_t Alternate;
}GPIO_InitTypedef;

class GPIO{
private:
	GPIO_TypeDef *GPIOx;
	GPIO_InitTypedef Config;
	uint32_t PinPos;
	inline void SetPinSpeed(GPIO_TypeDef *GPIOx,uint32_t pinpos,uint32_t speed);
	inline void SetPinPull(GPIO_TypeDef *GPIOx,uint32_t pinpos,uint32_t pull);
	inline void SetPinMode(GPIO_TypeDef *GPIOx,uint32_t pinpos,uint32_t mode);
	inline void SetAlternate0_7(GPIO_TypeDef *GPIOx,uint32_t pinpos,uint32_t alternate);
	inline void SetAlternate8_15(GPIO_TypeDef *GPIOx,uint32_t pinpos,uint32_t alternate);
	inline void SetOutputPinType(GPIO_TypeDef *GPIOx,uint32_t pinmask,uint32_t outputType);
public:
	GPIO(GPIO_TypeDef *GPIOx,uint32_t pin);
	uint32_t Begin(void);
	void SetParameter(uint8_t Pull,uint8_t Mode,uint8_t Speed,uint8_t Type);
	uint32_t GetPortNumber(GPIO_TypeDef *GPIOx);
	void OutputInit(void);
	void InputInit(void);
	void AlternateInit(uint32_t Alternate);
};


#endif /* GPIO_H_ */

/*
 * gpio.c
 *
 *  Created on: Jan 12, 2025
 *      Author: wataoxp
 */

#include "gpio.h"

using namespace GPIOs;

GPIO::GPIO(GPIO_TypeDef *GPIOPORT,uint32_t pin) :GPIOx(GPIOPORT),PinPos(pin)
{
	;
}
inline void GPIO::SetPinSpeed(GPIO_TypeDef *GPIOx,uint32_t pinpos,uint32_t speed)
{
	MODIFY_REG(GPIOx->OSPEEDR,GPIO_OSPEEDR_OSPEED0 << (pinpos * 2),(speed << (pinpos * 2)));
}
inline void GPIO::SetPinPull(GPIO_TypeDef *GPIOx,uint32_t pinpos,uint32_t pull)
{
	MODIFY_REG(GPIOx->PUPDR,GPIO_PUPDR_PUPD0 << (pinpos * 2),(pull << (pinpos * 2)));
}
inline void GPIO::SetPinMode(GPIO_TypeDef *GPIOx,uint32_t pinpos,uint32_t mode)
{
	MODIFY_REG(GPIOx->MODER,GPIO_MODER_MODE0 << (pinpos * 2),(mode << (pinpos * 2)));
}
inline void GPIO::SetAlternate0_7(GPIO_TypeDef* GPIOx,uint32_t pinpos,uint32_t alternate)
{
	MODIFY_REG(GPIOx->AFR[0],GPIO_AFRL_AFSEL0 << (pinpos * 4),alternate << (pinpos * 4));
}
inline void GPIO::SetAlternate8_15(GPIO_TypeDef* GPIOx,uint32_t pinpos,uint32_t alternate)
{
	pinpos -= 8;
	MODIFY_REG(GPIOx->AFR[1],GPIO_AFRH_AFSEL8 << (pinpos * 4),alternate << (pinpos * 4));
}
inline void GPIO::SetOutputPinType(GPIO_TypeDef *GPIOx,uint32_t pinmask,uint32_t outputType)
{
	MODIFY_REG(GPIOx->OTYPER,pinmask,(pinmask * outputType));
}
void GPIO::SetParameter(uint8_t Pull,uint8_t Mode,uint8_t Speed,uint8_t Type)
{
	Config.Pull = Pull;
	Config.Mode = Mode;
	Config.Speed = Speed;
	Config.OutputType = Type;
}
uint32_t GPIO::GetPortNumber(GPIO_TypeDef *GPIOx)
{
	uint32_t Ret;

	if(GPIOx == GPIOA)
	{
		Ret = 1 << PORTA;
	}
	else if(GPIOx == GPIOB)
	{
		Ret = 1 << PORTB;
	}
	else if(GPIOx == GPIOC)
	{
		Ret = 1 << PORTC;
	}
#ifdef GPIOD
	else if(GPIOx == GPIOD)
	{
		Ret = 1 << PORTD;
	}
#endif
#ifdef GPIOE
	else if(GPIOx == GPIOE)
	{
		Ret = 1 << PORTE;
	}
#endif
	else if(GPIOx == GPIOF)
	{
		Ret = 1 << PORTF;
	}
	else
	{
		Ret = 0;
	}

	return Ret;
}
uint32_t GPIO::Begin(void)
{
	uint32_t Periphs = GetPortNumber(GPIOx);
	if(!Periphs)
	{
		return nullPort;
	}

	if(LL_IOP_GRP1_IsEnabledClock(Periphs) == 0)
	{
		LL_IOP_GRP1_EnableClock(Periphs);
	}

	if((PinPos == Pin13) || (PinPos == Pin14))
	{
		if(GPIOx == GPIOA)
		{
			return SWDPin;
		}
	}
	return Success;
}

void GPIO::OutputInit(void)
{
	GPIO_CLEAR(GPIOx,PinPos);
	SetPinSpeed(GPIOx, PinPos, Config.Speed);
	SetOutputPinType(GPIOx, (1 << PinPos), Config.OutputType);

	SetPinPull(GPIOx, PinPos, Config.Pull);
	SetPinMode(GPIOx, PinPos, Config.Mode);
}
void GPIO::AlternateInit(uint32_t Alternate)
{
	OutputInit();
	if(PinPos < Pin8)
	{
		SetAlternate0_7(GPIOx, PinPos, Alternate);
	}
	else
	{
		SetAlternate8_15(GPIOx, PinPos, Alternate);
	}
}
void GPIO::InputInit(void)
{
	SetPinPull(GPIOx, PinPos, Config.Pull);
	SetPinMode(GPIOx, PinPos, Config.Mode);
}

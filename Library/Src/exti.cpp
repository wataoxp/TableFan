/*
 * exti.c
 *
 *  Created on: Jan 28, 2025
 *      Author: wataoxp
 */
#include "exti.h"

using namespace ExtiCommands;

EXTIR::EXTIR(GPIO_TypeDef *GPIOPORT,uint32_t pin) :GPIOx(GPIOPORT),PinPos(pin)
{
	;
}

/*** Private ***/
GPIO_Port EXTIR::CheckPort(void)
{
	GPIO_Port ret;

	if(GPIOx == GPIOA)
	{
		ret = PORTA;
	}
	else if(GPIOx == GPIOB)
	{
		ret = PORTB;
	}
	else if(GPIOx == GPIOC)
	{
		ret = PORTC;
	}
#ifdef GPIOE
	else if(GPIOx == GPIOE)
	{
		ret = PORTE;
	}
#endif
#ifdef GPIOD
	else if(GPIOx == GPIOD)
	{
		ret = PORTD;
	}
#endif
	else if(GPIOx == GPIOF)
	{
		ret = PORTF;
	}
	else
	{
		ret = NoPort;
	}
	return ret;
}

uint32_t EXTIR::SetSource(GPIO_Port port)
{
	uint32_t Reg = PinPos / 4;
	uint32_t Bit = (PinPos % 4) * 8;

	if(PinPos > Pin15)
	{
		return NotPin;
	}

	MODIFY_REG(EXTI->EXTICR[Reg],EXTICR_Mask << Bit, port << Bit);

	return success;
}

void EXTIR::ExtiMode(uint8_t Mode,uint32_t Line)
{
	switch (Mode)
	{
	case LL_EXTI_MODE_IT:
		LL_EXTI_DisableEvent_0_31(Line);
		LL_EXTI_EnableIT_0_31(Line);
		break;
	case LL_EXTI_MODE_EVENT:
		LL_EXTI_DisableIT_0_31(Line);
		LL_EXTI_EnableEvent_0_31(Line);
		break;
	case LL_EXTI_MODE_IT_EVENT:
		LL_EXTI_EnableIT_0_31(Line);
		LL_EXTI_EnableEvent_0_31(Line);
		break;
	default:
		break;
	}
}

void EXTIR::ExtiTrigger(uint8_t Trigger,uint32_t Line)
{
	switch(Trigger)
	{
	case LL_EXTI_TRIGGER_RISING:
		LL_EXTI_DisableFallingTrig_0_31(Line);
		LL_EXTI_EnableRisingTrig_0_31(Line);
		break;
	case LL_EXTI_TRIGGER_FALLING:
		LL_EXTI_DisableRisingTrig_0_31(Line);
		LL_EXTI_EnableFallingTrig_0_31(Line);
		break;
	case LL_EXTI_TRIGGER_RISING_FALLING:
		LL_EXTI_EnableRisingTrig_0_31(Line);
		LL_EXTI_EnableFallingTrig_0_31(Line);
		break;
	default:
		break;
	}
}

/*** Public ***/
uint32_t EXTIR::Config(void)
{
	GPIO_Port Port;

	Port = CheckPort();

	if(Port == NoPort)
	{
		return NotPort;
	}
	if(SetSource(Port) != success)
	{
		return NotPin;
	}

	return success;
}

void EXTIR::ConfigMode_Trigger(uint8_t Mode,uint8_t Trigger)
{
	uint32_t ExtiLine = 1 << PinPos;

	ExtiMode(Mode, ExtiLine);
	ExtiTrigger(Trigger, ExtiLine);
}

#if 0
//NVICの設定は分離する方針(2025-09-30)
inline IRQn_Type EXTIR::CheckExtiLine(void)
{
	IRQn_Type ret = (IRQn_Type)UINT8_MAX;

	if(PinPos <= Pin1)
	{
		 ret = EXTI0_1_IRQn;
	}
	else if(PinPos <= Pin3)
	{
		 ret = EXTI2_3_IRQn;
	}
	else if(PinPos <= Pin15)
	{
		 ret = EXTI4_15_IRQn;
	}
	else
	{
		;
	}
	return ret;
}
#endif

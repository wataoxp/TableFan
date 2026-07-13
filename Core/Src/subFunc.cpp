/*
 * subFunc.cpp
 *
 *  Created on: May 22, 2026
 *      Author: wataoxp
 */

#include "subFunc.h"

using namespace PinAssignment;

uint32_t ConfigPWM(TIM& tim,uint32_t duty)
{
	using namespace TimerParameter;

	uint32_t ret = 0;

	TIM_Config(tim, PWM::Prescaler, PWM::Reload);
	ret = PWM_Config(tim, GPIOA, IOPin::PWMoutPos, IOPin::PWMoutAF, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
	tim.SetCH1CompareValue(duty);
	tim.EnableTimer();
	tim.EnablePulse(LL_TIM_CHANNEL_CH1);

	return ret;
}

void ConfigSamplingTimer(TIM& tim,TIM_TypeDef* pTIM)
{
	using namespace TimerParameter;

	TIM_Config(tim, Sample::Prescaler, Sample::DefaultReload);

	LL_TIM_EnableIT_UPDATE(pTIM);

	__NVIC_SetPriority(TIM16_IRQn, 1);
	__NVIC_EnableIRQ(TIM16_IRQn);

	tim.EnableTimer();
}

uint32_t ConfigInput()
{
	uint32_t ret = 0;
	uint32_t Pull = LL_GPIO_PULL_UP;
	uint32_t Mode = LL_EXTI_MODE_EVENT;
	uint32_t Trigger = LL_EXTI_TRIGGER_FALLING;

	ret += EXTI_Config(GPIOA, IOPin::LowDutyPos, Pull, Mode, Trigger);
	ret += EXTI_Config(GPIOA, IOPin::MiddleDutyPos, Pull, Mode, Trigger);
	ret += EXTI_Config(GPIOA, IOPin::HighDutyPos, Pull, Mode, Trigger);

	ret += EXTI_Config(GPIOA, IOPin::LedEnablePos, Pull, Mode, Trigger);

//	__NVIC_SetPriority(EXTI2_3_IRQn, 0);
//	__NVIC_EnableIRQ(EXTI2_3_IRQn);
//
//	__NVIC_SetPriority(EXTI4_15_IRQn, 0);
//	__NVIC_EnableIRQ(EXTI4_15_IRQn);

	return ret;
}

uint32_t ConfigOutput()
{
	uint32_t ret = 0;

	ret += GPIO_Config(GPIOA, IOPin::IndicatorPos, LL_GPIO_MODE_OUTPUT);
	ret += GPIO_Config(GPIOA, IOPin::SwitchPos, LL_GPIO_MODE_OUTPUT);

	return ret;
}

uint32_t ConfigRGBTimer(TIM& tim)
{
	using namespace LEDParameter;

	uint32_t ret = 0;

	TIM_Config(tim, Prescaler, Reload);
	ret += PWM_Config(tim, RGBPort, RGBPinPos, RGBAF, RGBChannelisTIM, LL_TIM_OCMODE_PWM1);
	ret += tim.ConfigDMA(RGBChannelisTIM,RequestTrigger);
	tim.EnableTimer();
	tim.EnablePulse(RGBChannelisTIM);

	return ret;
}

void SleepMode(bool flag)
{
	LL_EXTI_ClearFallingFlag_0_31(GetExtiLineMask());
	LL_EXTI_EnableFallingTrig_0_31(GetExtiLineMask());
	LL_SYSTICK_DisableIT();

	// 電源電圧がしきい値を上回っている場合は消灯
	if(flag)
	{
		LedOff();
	}

	__SEV();
	__WFE();
	__WFE();

#ifdef DEBUG
	__NOP();
#endif

	LedOn();

	LL_EXTI_ClearFallingFlag_0_31(GetExtiLineMask());
	LL_EXTI_DisableFallingTrig_0_31(GetExtiLineMask());

	LL_SYSTICK_EnableIT();
}

uint32_t GetNewDuty(uint32_t command)
{
	using namespace TimerParameter;

	uint32_t NewDuty = PWM::DutyLow;

	switch(command)
	{
	case IOPin::LowDuty:
		NewDuty = PWM::DutyLow;
		break;
	case IOPin::MiddleDuty:
		NewDuty = PWM::DutyMiddle;
		break;
	case IOPin::HighDuty:
		NewDuty = PWM::DutyHigh;
		break;
	default:
		break;
	}

	return NewDuty;
}

bool ChangeLedMode(bool EnableFlag,TIM& tim)
{
	using namespace TimerParameter;
	using namespace LEDParameter;

	bool NewFlag = !EnableFlag;

	if(NewFlag)
	{
		tim.SetAutoReload(Sample::LedOnReload-1);
	}
	else
	{
		tim.SetAutoReload(Sample::DefaultReload-1);
	}

	// 割り込みで即座にUIFフラグがクリアされるため、UpdateTimer()は使えない
	LL_TIM_GenerateEvent_UPDATE(SamplingTimer);

	return NewFlag;
}

#include <string.h>

// 桁数を最大4として変換
uint32_t ItoA(char* string,uint16_t n)
{
	if(n > 9999)
	{
		return 1;
	}

	/* 桁数を求める場合 */
//	while(n >= radix){
//		n /= radix;
//	}

	char tmp[5];

	tmp[4] = '\0';

	for(int32_t i = 3;i >= 0;i--)
	{
		tmp[i] = (n % 10) + '0';
		n /= 10;
	}

	memcpy(string,tmp,5);

	return 0;
}



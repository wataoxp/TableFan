/*
 * delay.cpp
 *
 *  Created on: Jan 6, 2025
 *      Author: wataoxp
 */
#include "delay.h"

/* SysTick Delay */

sDelay::sDelay(CoreClock source) : ClockSource(source)
{
	InitMillTick();
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}

inline void sDelay::InitMillTick()
{
	uint32_t load = ClockSource * SysTickTiming::MillSecond;

	SysTick->LOAD = load-1;
	SysTick->VAL = 0;
}

inline void sDelay::InitMicroTick()
{
	uint32_t load = ClockSource;

	SysTick->LOAD = load-1;
	SysTick->VAL = 0;
}

void sDelay::mDelay(uint32_t nTime) const
{
	__IO uint32_t tmp = SysTick->CTRL;
	__IO uint32_t mDelay = nTime;
	((void)tmp);

	while(mDelay)
	{
		if (LL_SYSTICK_IsActiveCounterFlag() != 0)
		{
			mDelay--;
		}
	}
}

void sDelay::uDelay(uint32_t nTime) const
{
	SysTick->VAL = 0;						// SysTickのリセット
	uint32_t start = SysTick->VAL;
	uint32_t cycle = ClockSource * nTime;	// 1usのクロック数*待機us=消費クロック数

	while((start - SysTick->VAL) < cycle);
}

/* TIM Delay */

tDelay::tDelay(CoreClock source,TIM& tim) : ClockSource(source),timer(tim)
{
	TimerInit();
}

void tDelay::TimerInit()
{
	TimerPeripheral::TIM_InitTypedef Config = {0};

	// 1us用の設定。ミリ秒は1000回uDelayを呼ぶ設計になっている
	Config.CountMode = LL_TIM_CLOCKDIVISION_DIV1;
	Config.Source = LL_TIM_CLOCKSOURCE_INTERNAL;
	Config.Prescale = ClockSource -1;
	Config.Reload = UINT16_MAX-1;
	Config.CountMode = LL_TIM_COUNTERDIRECTION_UP;
	Config.Division = LL_TIM_CLOCKDIVISION_DIV1;

	timer.ConfigTimer(&Config);
	timer.EnableTimer();
}

void tDelay::mDelay(uint32_t nTime)const
{
	timer.mDelay(nTime);
}
void tDelay::uDelay(uint32_t nTime)const
{
	timer.uDelay(nTime);
}




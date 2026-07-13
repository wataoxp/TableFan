/*
 * delay.h
 *
 *  Created on: Jan 6, 2025
 *      Author: wataoxp
 */

#ifndef INC_DELAY_H_
#define INC_DELAY_H_

#include "periph.h"
#include "tim.h"

// 16MHℤのとき、16*1000サイクルで1ms
namespace SysTickTiming{
	constexpr uint32_t MillSecond = 1000;
	constexpr uint32_t MicroSecond = 1;
}

// すべてのDelay依存ファイルの修正後に消す
typedef void (*uDelay)(uint32_t);
typedef void (*mDelay)(uint32_t);

class sDelay{
private:
	CoreClock ClockSource;
	void InitMillTick();
	void InitMicroTick();
public:
	sDelay(CoreClock source);
	void mDelay(uint32_t nTime)const;
	void uDelay(uint32_t nTime)const;

	void WaitSetUp();
	uint32_t GetWaitFlag();

	inline void EnableISR() { LL_SYSTICK_EnableIT(); }
	inline void DisableISR() { 	LL_SYSTICK_DisableIT(); }
};

class tDelay{
private:
	CoreClock ClockSource;
	TIM& timer;
	void TimerInit();
public:
	tDelay(CoreClock source,TIM& tim);
	void mDelay(uint32_t nTime)const;
	void uDelay(uint32_t nTime)const;

	void WaitSetUp();
	uint32_t GetWaitFlag();
};

inline void sDelay::WaitSetUp()
{
	__IO uint32_t tmp = SysTick->CTRL;
	((void)tmp);
}

inline uint32_t sDelay::GetWaitFlag()
{
	return LL_SYSTICK_IsActiveCounterFlag();
}

inline void tDelay::WaitSetUp()
{
	timer.WaitSetUp();
}

inline uint32_t tDelay::GetWaitFlag()
{
	return timer.GetWaitFlag();
}


/* Delayタイムソースの切り替え */

#if defined(__has_include)
	#if __has_include("FreeRTOS.h")
		using DelaySource = tDelay;
	#else
		using DelaySource = sDelay;
	#endif
#endif

using DelayPoicy = DelaySource;

#endif /* INC_DELAY_H_ */

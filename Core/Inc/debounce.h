/*
 * debouce.h
 *
 *  Created on: Mar 13, 2026
 *      Author: wataoxp
 */

#ifndef INC_DEBOUNCE_H_
#define INC_DEBOUNCE_H_

#include "periph.h"

/* 割り込みハンドラから呼び出すので静的関数としておく */

namespace DebouceUnits{
	typedef struct{
		uint32_t cnt0;	// 垂直カウンタ0ビット目
		uint32_t cnt1;	// 垂直カウンタ１ビット目
		uint32_t PrevState;	// 前回の確定したポート状態
	}VerticalCounter;
}

class Debounce{
private:
	static uint32_t PinMask;
	static uint32_t Interval;
	static GPIO_TypeDef* GPIOx;
	static DebouceUnits::VerticalCounter ct;
public:
	static void FilterSetUp(GPIO_TypeDef* pGPIO,uint32_t mask,uint32_t time);
	static void ButtonFilter(uint32_t& command);
	static uint32_t GetInterval();
};

inline uint32_t Debounce::GetInterval()
{
	return Interval;
}



#endif /* INC_DEBOUNCE_H_ */

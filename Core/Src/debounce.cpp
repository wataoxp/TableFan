/*
 * debounce.cpp
 *
 *  Created on: Mar 13, 2026
 *      Author: wataoxp
 */

#include "debounce.h"

#define VerticalCount1Bit

#ifndef VerticalCount1Bit
#define VerticalCount2Bit
#endif

using namespace DebouceUnits;

// 静的メンバ変数の初期化
uint32_t Debounce::PinMask = 0;
uint32_t Debounce::Interval = 0;
GPIO_TypeDef* Debounce::GPIOx = nullptr;
VerticalCounter Debounce::ct = {};

/* Setup */

void Debounce::FilterSetUp(GPIO_TypeDef* pGPIO,uint32_t mask,uint32_t time)
{
	GPIOx = pGPIO;
	PinMask = mask;
	Interval = time;
}

/* Filter */

#ifdef VerticalCount1Bit
void Debounce::ButtonFilter(uint32_t& command)
{
	// 現在のポートを読み取る
	uint32_t current = ~(LL_GPIO_ReadInputPort(GPIOx)) & PinMask;
	// 前回のポート情報と比較する(変化したピンのみ1が立つ)
	uint32_t diff = ct.PrevState ^ current;

	if(diff)
	{
		// currentでマスクすることで、押下と開放を区別できる
		uint32_t pressed = diff & current;		// 開放時、currentは0なのでクリアされる
		uint32_t released = diff & ~current;	// 押下時、~currentによって0のまま

		if(pressed) command |= pressed;
		if(released) command &= ~released;		// commandは開放でのみクリアされる
	}
	ct.PrevState = current;
}
#endif

// https://note.com/watao_xp/n/nee8e976d3948
#ifdef VerticalCount2Bit
void Debounce::ButtonFilter(uint32_t& command)
{
	uint32_t delta,toggle;
	uint32_t sample = ~(LL_GPIO_ReadInputPort(GPIOx)) & PinMask;

	delta = sample ^ ct.PrevState;
	ct.cnt1 = (ct.cnt1 ^ ct.cnt0) & delta;
	ct.cnt0 = ~ct.cnt0 & delta;

	toggle = ct.cnt0 & ct.cnt1;
	ct.PrevState ^= toggle;

	command |= ct.PrevState & toggle;
	// 解放時を取得すると長押しの判定も可能に
//	uint32_t Released = toggle & ~ct.PrevState;
}
#endif

//#define VerticalCount4Bit
#ifdef VerticalCount4Bit
void Debounce::ButtonFilter(uint32_t& command)
{
	static uint32_t state = 0,cnt0 = 0,cnt1 = 0;
	uint32_t delta,toggle;

	uint32_t sample = ~(LL_GPIO_ReadInputPort(GPIOx)) & PinMask;

	delta = sample ^ state;
	cnt1 = (cnt1 ^ cnt0) & delta;
	cnt0 = ~cnt0 & delta;

	toggle = delta & ~(cnt0 | cnt1);
	state ^= toggle;

	command |= toggle & state;
	// 解放時を取得すると長押しの判定も可能に
//	uint32_t Released = toggle & ~state;
}
#endif




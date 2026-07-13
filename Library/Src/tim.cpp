/*
 * tim.cpp
 *
 *  Created on: May 25, 2025
 *      Author: wataoxp
 */
#include "tim.h"

using namespace TimerPeripheral;

TIM::TIM(TIM_TypeDef *TIMPORT) :TIMx(TIMPORT)
{
	;
}

// G0x1->TIM2(32Bti Timer)
// G0B1->Full Timer
uint32_t TIM::CheckTimerPeriph(void)
{
	uint32_t TimNumber = (uint32_t)TIMx;
	uint32_t ret = success;

	switch(TimNumber)
	{
	case TIM1_BASE:
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
		break;
#ifdef TIM2
	case TIM2_BASE:
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
		break;
#endif
	case TIM3_BASE:
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
		break;
#ifdef TIM4
	case TIM4_BASE:
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
		break;
#endif
#ifdef TIM6
	case TIM6_BASE:
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM6);
		break;
#endif
#ifdef TIM7
	case TIM7_BASE:
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM7);
		break;
#endif
	case TIM14_BASE:
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM14);
		break;
#ifdef TIM15
	case TIM15_BASE:
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM15);
		break;
#endif
	case TIM16_BASE:
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM16);
		break;
	case TIM17_BASE:
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM17);
		break;
	default:
		ret = NotTimer;
	}
	return ret;
}

uint32_t TIM::ConfigTimer(TIM_InitTypedef *Config)
{
	if(CheckTimerPeriph() != success)
	{
		return NotTimer;
	}

	if(IS_TIM_CLOCK_DIVISION_INSTANCE(TIMx))
	{
		LL_TIM_SetClockDivision(TIMx, Config->Division);
	}

	LL_TIM_SetClockSource(TIMx, Config->Source);
	LL_TIM_SetPrescaler(TIMx, Config->Prescale);
	LL_TIM_SetAutoReload(TIMx, Config->Reload);

	if(IS_TIM_COUNTER_MODE_SELECT_INSTANCE(TIMx))
	{
		LL_TIM_SetCounterMode(TIMx, Config->CountMode);
	}
	UpdateTimer();
	LL_TIM_EnableARRPreload(TIMx);

	CLEAR_REG(TIMx->CCER);
	CLEAR_REG(TIMx->CR2);
	CLEAR_REG(TIMx->CCMR1);
	CLEAR_REG(TIMx->CCMR2);

	return success;
}

uint32_t TIM::ConfigPWM(uint32_t Channel,uint32_t mode)
{
	if(Channel > LL_TIM_CHANNEL_CH6)
	{
		return failed;
	}

	LL_TIM_OC_EnablePreload(TIMx, Channel);		//OCxPE、プリロード有効。更新イベント時にCCRxを更新
	LL_TIM_OC_SetMode(TIMx, Channel, mode);		//OCxM、出力モード（PWM等）
	LL_TIM_OC_SetPolarity(TIMx, Channel, LL_TIM_OCPOLARITY_HIGH);	//CCER,CCxP 極性の選択

	if(IS_TIM_BREAK_INSTANCE(TIMx))
	{
		LL_TIM_OC_SetIdleState(TIMx, Channel,LL_TIM_OCIDLESTATE_LOW);	//OISビットがあるTIMの場合
	}

	if(IS_TIM_BREAK_INSTANCE(TIMx))
	{
		LL_TIM_EnableAllOutputs(TIMx);								//BDTR->MOE
	}
	return success;
}

uint32_t TIM::ConfigDMA(uint32_t Channel,uint32_t ReqSource)
{
	uint32_t Ret = success;

	LL_TIM_CC_SetDMAReqTrigger(TIMx, ReqSource);

	if(ReqSource == LL_TIM_CCDMAREQUEST_CC)
	{
		switch(Channel)
		{
		case LL_TIM_CHANNEL_CH1:
			LL_TIM_EnableDMAReq_CC1(TIMx);
			break;
		case LL_TIM_CHANNEL_CH2:
			LL_TIM_EnableDMAReq_CC2(TIMx);
			break;
		case LL_TIM_CHANNEL_CH3:
			LL_TIM_EnableDMAReq_CC3(TIMx);
			break;
		case LL_TIM_CHANNEL_CH4:
			LL_TIM_EnableDMAReq_CC4(TIMx);
			break;
		default:
			Ret = NotChannel;
			break;
		}
	}
	else
	{
		LL_TIM_EnableDMAReq_UPDATE(TIMx);
	}
	return Ret;
}

uint32_t* TIM::GetCCxRegAddr(uint32_t Channel)
{
	uint32_t *CCRxReg = nullptr;								//DMA、ペリフェラルアドレスを返す

	switch(Channel)
	{
	case LL_TIM_CHANNEL_CH1:
		CCRxReg = (uint32_t*)&TIMx->CCR1;
		break;
	case LL_TIM_CHANNEL_CH2:
		CCRxReg = (uint32_t*)&TIMx->CCR2;
		break;
	case LL_TIM_CHANNEL_CH3:
		CCRxReg= (uint32_t*)&TIMx->CCR3;
		break;
	case LL_TIM_CHANNEL_CH4:
		CCRxReg = (uint32_t*)&TIMx->CCR4;
		break;
	default:
		break;
	}
	return CCRxReg;
}

/***************************************************/
/* TIM_InputStructを使用した形に書き換えてテストしてくれ   */

uint32_t TIM::ConfigInput(uint32_t Channel,uint32_t Polarity)
{
	// G0シリーズではTI2までしか機能しない？
	if(Channel > LL_TIM_CHANNEL_CH2)
	{
		return NotChannel;
	}

	if(LL_TIM_CC_IsEnabledChannel(TIMx, Channel) != 0)
	{
		LL_TIM_CC_DisableChannel(TIMx, Channel);
	}

	if(Channel == LL_TIM_CHANNEL_CH1)
	{
		LL_TIM_IC_SetActiveInput(TIMx, Channel, LL_TIM_ACTIVEINPUT_DIRECTTI);		// CC1をTI1入力に
		LL_TIM_SetTriggerInput(TIMx, LL_TIM_TS_TI1FP1);								// TI1FP1をトリガ設定
	}
	else
	{
		LL_TIM_IC_SetActiveInput(TIMx, Channel, LL_TIM_ACTIVEINPUT_DIRECTTI);		// CC2をTI2入力に
		LL_TIM_SetTriggerInput(TIMx, LL_TIM_TS_TI1FP1);								// TI2FP2をトリガ設定
	}

	LL_TIM_IC_SetPrescaler(TIMx, Channel, LL_TIM_ICPSC_DIV1);					// 1エッジでキャプチャ開始
	LL_TIM_IC_SetFilter(TIMx, Channel, LL_TIM_IC_FILTER_FDIV16_N8);				// 8回フィルタ
	LL_TIM_IC_SetPolarity(TIMx, Channel, Polarity);								// 指定した極性のエッジでキャプチャ

	//スレーブモード
	LL_TIM_SetSlaveMode(TIMx, LL_TIM_SLAVEMODE_RESET);							// TIx信号の立ち上がりでCNTリセット

	// URSビットのセット。カウンタのオーバーフローでのみUIFがセット
	LL_TIM_SetUpdateSource(TIMx, LL_TIM_UPDATESOURCE_COUNTER);

	LL_TIM_CC_EnableChannel(TIMx, Channel);

	return success;
}

//uint32_t TIM::ConfigInput(uint32_t Channel,uint32_t Polarity)
//{
//	if(Channel > LL_TIM_CHANNEL_CH6)
//	{
//		return NotChannel;
//	}
//
//	if(LL_TIM_CC_IsEnabledChannel(TIMx, Channel) != 0)
//	{
//		LL_TIM_CC_DisableChannel(TIMx, Channel);
//	}
//
//	LL_TIM_IC_SetActiveInput(TIMx, Channel, LL_TIM_ACTIVEINPUT_DIRECTTI);		// CC1をT1入力に
//	LL_TIM_IC_SetPrescaler(TIMx, Channel, LL_TIM_ICPSC_DIV1);					// 1エッジでキャプチャ開始
//	LL_TIM_IC_SetFilter(TIMx, Channel, LL_TIM_IC_FILTER_FDIV16_N8);				// 8回フィルタ
////	LL_TIM_IC_SetFilter(TIMx, Channel, LL_TIM_IC_FILTER_FDIV1);					// フィルター無し
//	LL_TIM_IC_SetPolarity(TIMx, Channel, Polarity);								// 指定した極性のエッジでキャプチャ
//
//	//スレーブモード
//	LL_TIM_SetTriggerInput(TIMx, LL_TIM_TS_TI1FP1);								// TI1FP1をトリガ設定
//	LL_TIM_SetSlaveMode(TIMx, LL_TIM_SLAVEMODE_RESET);							// TIx信号の立ち上がりでCNTリセット
//
//	// URSビットのセット。カウンタのオーバーフローでのみUIFがセット
//	LL_TIM_SetUpdateSource(TIMx, LL_TIM_UPDATESOURCE_COUNTER);
//
//	LL_TIM_CC_EnableChannel(TIMx, Channel);
//
//	return success;
//}

void TIM::ConfigCombinedCh(uint32_t ch1Pol,uint32_t ch2Pol)
{
	//PWM入力モードはCH1とCH2のみ(P602)

	LL_TIM_CC_DisableChannel(TIMx, LL_TIM_CHANNEL_CH1);
	LL_TIM_CC_DisableChannel(TIMx, LL_TIM_CHANNEL_CH2);

	//Rising Edge CCR
	LL_TIM_IC_SetActiveInput(TIMx, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_DIRECTTI);	//CC1SをTI1に配置
	LL_TIM_IC_SetPrescaler(TIMx, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV1);				//入力プリスケーラ
	LL_TIM_IC_SetFilter(TIMx, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1_N8);			//8回フィルタ
	LL_TIM_IC_SetPolarity(TIMx, LL_TIM_CHANNEL_CH1, ch1Pol);							//極性の選択

	//Falling Edge CCR
	LL_TIM_IC_SetActiveInput(TIMx, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_INDIRECTTI);	//CC2SをTI1に配置
	LL_TIM_IC_SetPrescaler(TIMx, LL_TIM_CHANNEL_CH2, LL_TIM_ICPSC_DIV1);
	LL_TIM_IC_SetFilter(TIMx, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV1_N8);
	LL_TIM_IC_SetPolarity(TIMx, LL_TIM_CHANNEL_CH2,ch2Pol);

	//スレーブモード
	LL_TIM_SetTriggerInput(TIMx, LL_TIM_TS_TI1FP1);								//TI1FP1をトリガ設定
	LL_TIM_SetSlaveMode(TIMx, LL_TIM_SLAVEMODE_RESET);							//TI1FP1の立ち上がりでCNTリセット

	LL_TIM_CC_EnableChannel(TIMx, LL_TIM_CHANNEL_CH1);
	LL_TIM_CC_EnableChannel(TIMx, LL_TIM_CHANNEL_CH2);
}

uint32_t TIM::ConfigEncoderMode(TIM_InputStruct *Ti1,TIM_InputStruct *Ti2,uint32_t Mode)
{
	if((Ti1->Channel > LL_TIM_CHANNEL_CH6) || (Ti2->Channel > LL_TIM_CHANNEL_CH6))
	{
		return NotChannel;
	}

	// 有効であるなら無効にする
	if(LL_TIM_CC_IsEnabledChannel(TIMx, Ti1->Channel) != 0)
	{
		DisablePulse(Ti1->Channel);
	}
	if(LL_TIM_CC_IsEnabledChannel(TIMx, Ti2->Channel) != 0)
	{
		DisablePulse(Ti2->Channel);
	}

	// TI1の設定
	LL_TIM_IC_SetActiveInput(TIMx, Ti1->Channel, Ti1->CCxS);
	LL_TIM_IC_SetPrescaler(TIMx, Ti1->Channel, Ti1->Prescale);
	LL_TIM_IC_SetFilter(TIMx, Ti1->Channel, Ti1->Filter);
	LL_TIM_IC_SetPolarity(TIMx, Ti1->Channel, Ti1->Polarity);

	// TI2の設定
	LL_TIM_IC_SetActiveInput(TIMx, Ti2->Channel, Ti2->CCxS);
	LL_TIM_IC_SetPrescaler(TIMx, Ti2->Channel, Ti2->Prescale);
	LL_TIM_IC_SetFilter(TIMx, Ti2->Channel, Ti2->Filter);
	LL_TIM_IC_SetPolarity(TIMx, Ti2->Channel, Ti2->Polarity);

	LL_TIM_SetEncoderMode(TIMx, Mode);

	EnablePulse(Ti1->Channel);
	EnablePulse(Ti2->Channel);

	return success;
}


/* Timer Delay */

void TIM::mDelay(uint32_t nTime)
{
	for(uint32_t i = 0;i < nTime;i++)
	{
		uDelay(1000);
	}
}

// PSCで1kまたは1MHzに設定。経過時間を比較する。ARRはMAXで
void TIM::uDelay(uint32_t nTime)
{
	uint16_t start = LL_TIM_GetCounter(TIMx);

	while(uint16_t(LL_TIM_GetCounter(TIMx) - start) < nTime);
}


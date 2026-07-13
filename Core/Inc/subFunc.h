/*
 * subFunc.h
 *
 *  Created on: May 22, 2026
 *      Author: wataoxp
 */

#ifndef INC_SUBFUNC_H_
#define INC_SUBFUNC_H_

#include "mylib.h"

namespace PinAssignment{

	struct IOPin{
		IOPin() = delete;

		enum PinPos{
			IndicatorPos	= Pin0,
			ADCinPos		= Pin3,

			PWMoutPos		= Pin4,

			LedEnablePos 	= Pin2,
			LowDutyPos 		= Pin5,
			MiddleDutyPos 	= Pin6,
			HighDutyPos 	= Pin7,

			SwitchPos		= Pin11,
		};

		enum Value{
			Indicator		= 1 << IndicatorPos,
			ADCin			= 1 << ADCinPos,

			PWMout			= 1 << PWMoutPos,

			LedEnable		= 1 << LedEnablePos,
			LowDuty			= 1 << LowDutyPos,
			MiddleDuty		= 1 << MiddleDutyPos,
			HighDuty		= 1 << HighDutyPos,

			LoadSwitch		= 1 << SwitchPos,
		};

		enum Line{
			LedEnableLine	= LL_EXTI_LINE_2,
			LowDutyLine		= LL_EXTI_LINE_5,
			MiddleDutyLine	= LL_EXTI_LINE_6,
			HighDutyLine	= LL_EXTI_LINE_7,
		};

		enum Alternate{
			PWMoutAF		= LL_GPIO_AF_4,
		};
	};

	constexpr uint32_t GetExtiPinMask() { return IOPin::LowDuty | IOPin::MiddleDuty | IOPin::HighDuty | IOPin::LedEnable; }
	constexpr uint32_t GetExtiLineMask() { return IOPin::LowDutyLine | IOPin::MiddleDutyLine | IOPin::HighDutyLine | IOPin::LedEnableLine; }
}

namespace TimerParameter{
	struct PWM{
		PWM() = delete;

		// 12M / 120*4 = 25k
		enum Frequency{
			Prescaler 		= 4,
			Reload			= 120,
		};

		enum Duty{
			DutyLow 		= 30,
			DutyMiddle		= 60,
			DutyHigh		= 120,
		};
	};

	TIM_TypeDef* const SamplingTimer = TIM16;

	struct Sample{
		Sample() = delete;

		// 12M / 12000= 1ms.
		enum Frequency{
			Prescaler 		= 12000,
			DefaultReload	= 60000,
			LedOnReload		= 200,
		};
	};


}

/* ADCデジタル値の決定 */
/*
 * 下限電圧は2.5V(分圧して1.25V)、電源電圧は2.8V
 * Vin/VRef * 255(分解能)によって、電圧下限値でのデジタル値は約114となる
 * */

namespace SystemUnits{
	constexpr uint16_t GetUnderVoltageLimit() { return 114U; }
	constexpr uint32_t GetIdleTimeMillLimit() { return 500U; }
}

namespace LEDParameter{
	TIM_TypeDef* const RGBTimer = TIM3;
	GPIO_TypeDef* const RGBPort = GPIOB;

	enum LedTimer{
		RGBAF = LL_GPIO_AF_12,
		RGBChannelisTIM = LL_TIM_CHANNEL_CH1,
		RGBPinPos = Pin6,
		RequestTrigger = LL_TIM_CCDMAREQUEST_UPDATE,

		// 12MHz設定
		Prescaler 		= 1,
		Reload			= 15,
	};
}

uint32_t ConfigPWM(TIM& tim,uint32_t duty);
void ConfigSamplingTimer(TIM& tim,TIM_TypeDef* pTIM);
uint32_t ConfigRTC(RealClock& rtc);
uint32_t ConfigInput();
uint32_t ConfigOutput();
uint32_t ConfigRGBTimer(TIM& tim);

void SleepMode(bool flag);
uint32_t GetNewDuty(uint32_t command);
bool ChangeLedMode(bool EnableFlag,TIM& tim);

uint32_t ItoA(char* string,uint16_t n);

static inline void LedOn()
{
	GPIO_WRITE(GPIOA,PinAssignment::IOPin::IndicatorPos);
}

static inline void LedOff()
{
	GPIO_CLEAR(GPIOA,PinAssignment::IOPin::IndicatorPos);
}

// TCK108AF.CTRL=Low,Enable

static inline void SwitchOn()
{
	GPIO_CLEAR(GPIOA,PinAssignment::IOPin::SwitchPos);
}

static inline void SwitchOff()
{
	GPIO_WRITE(GPIOA,PinAssignment::IOPin::SwitchPos);
}


#endif /* INC_SUBFUNC_H_ */

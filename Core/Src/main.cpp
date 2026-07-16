#include "mylib.h"

#include "debounce.h"
#include "isr.h"
#include "so1602.h"
#include "subFunc.h"
#include "ws2812.h"

#include <string.h>

constexpr uint32_t NumLed = 23;

struct MainVariable{
	bool VoltageFlag;
	bool LedFlag;
	uint16_t adcValue;
	uint32_t Duty;
	uint32_t IdleCount;
};

void LedMode(TIM& samp,MainVariable* obj,DMA& dma,NeoPixelSystem<NumLed>& pixel,DelaySource& delay);
void SendData(DMA& dma,NeoPixelSystem<NumLed>& pixel,DelaySource& delay);
void AdcValueView(SO1602& lcd,uint16_t value,char* string,uint32_t length);

int main(void)
{
	using namespace PinAssignment;

	constexpr CoreClock ClockSource = C0xDivHSI;

	DelaySource Delay(ClockSource);

	MainVariable obj = {true,false,0,TimerParameter::PWM::DutyMiddle,0};

	RCC_Config(LL_RCC_HSI_DIV_4);

	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);

//	DBG->APBFZ2 |= DBG_APB_FZ2_DBG_TIM14_STOP;
//	DBG->APBFZ2 |= DBG_APB_FZ2_DBG_TIM16_STOP;

	Debounce::FilterSetUp(GPIOA, GetExtiPinMask(),5);
	__IO const isrFlags& iFlag = GetISRStruct();

	TIM tim(LEDParameter::RGBTimer);
	ConfigRGBTimer(tim);

	NeoPixelSystem<NumLed> pixel;
	pixel.ParameterSet(8, LEDParts::Rainbow, 1);
	pixel.CreateBuffer();

	DMA dma(DMA1);
	DMA_Config(dma, LL_DMA_CHANNEL_1, LL_DMAMUX_REQ_TIM3_UP, (uint32_t*)pixel.GetBufferAddress(), (uint32_t*)&(TIM3->CCR1));

	TIM pwm(TimerParameter::PWMTimer);
	ConfigPWM(pwm,obj.Duty);
	// PWM制御ピンはオープンドレイン
	GPIO pwmPin(TimerParameter::PWMPort,IOPin::PWMoutPos);
	pwmPin.SetParameter(LL_GPIO_PULL_NO, LL_GPIO_MODE_ALTERNATE, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_OPENDRAIN);
	pwmPin.OutputInit();

	TIM Sampling(TimerParameter::SamplingTimer);
	ConfigSamplingTimer(Sampling, TimerParameter::SamplingTimer);

	ConfigInput();
	ConfigOutput();

	AnalogConverter adc(ADC1);
	ADC_Config(ADC1, adc, LL_ADC_CHANNEL_3, GPIOA, IOPin::ADCinPos, Delay);

	// PWMが動作してからロードスイッチをONにする。電源もシステムで管理する
	SwitchOn();

	while(1)
	{
		SleepMode(obj.VoltageFlag);

		obj.IdleCount = 0;

		while(iFlag.ExtCommand == 0 && iFlag.IntCommand == 0)
		{
			if(LL_SYSTICK_IsActiveCounterFlag()) obj.IdleCount++;
			if(obj.IdleCount > SystemUnits::GetIdleTimeMillLimit()) break;
		}

		if(iFlag.ExtCommand)
		{
			if(iFlag.ExtCommand == IOPin::LedEnable)
			{
				LedMode(Sampling, &obj, dma, pixel, Delay);
			}
			else
			{
				obj.Duty = GetNewDuty(iFlag.ExtCommand);
				pwm.SetCH1CompareValue(obj.Duty);
			}
		}
		if(iFlag.IntCommand)
		{
			ResetIntCommand();
			obj.adcValue = adc.StartSoftConvert();

			if(obj.adcValue < SystemUnits::GetUnderVoltageLimit())
			{
				obj.VoltageFlag = false;
				SwitchOff();
			}
			else
			{
				obj.VoltageFlag = true;
				SwitchOn();
			}
		}

		if(obj.LedFlag)
		{
			SendData(dma, pixel, Delay);
		}

		obj.IdleCount = 0;

		while(iFlag.ExtCommand != 0)
		{
			if(LL_SYSTICK_IsActiveCounterFlag()) obj.IdleCount++;
			if(obj.IdleCount > SystemUnits::GetIdleTimeMillLimit()) break;
		}
	}
}

void LedMode(TIM& samp,MainVariable* obj,DMA& dma,NeoPixelSystem<NumLed>& pixel,DelaySource& delay)
{
	obj->LedFlag = ChangeLedMode(obj->LedFlag,samp);

	// データ0を送って消灯
	if(!obj->LedFlag)
	{
		pixel.Reset();
		SendData(dma, pixel, delay);
		pixel.CreateBuffer();
	}
}

void SendData(DMA& dma,NeoPixelSystem<NumLed>& pixel,DelaySource& delay)
{
	using namespace LEDParameter;

	dma.StartDMA(LL_DMA_CHANNEL_1, pixel.GetLength());
	pixel.Shift();
	dma.StopDMAisChannel1();
	LL_TIM_ClearFlag_UPDATE(RGBTimer);
	while(LL_TIM_IsActiveFlag_UPDATE(RGBTimer) == 0);
	LL_TIM_ClearFlag_UPDATE(RGBTimer);
}

void AdcValueView(SO1602& lcd,uint16_t value,char* string,uint32_t length)
{
	ItoA(string, value);
	lcd.PointClear(0);
	lcd.StringLCD(string, length);
}

#if 0

// 工事中、最下部に元の処理がある
if(iFlag.IntCommand)
{
	if(iFlag.IntCommand == 1)
	{
		obj.VoltageFlag = CheckVoltage(adc);
	}
	// システムのON/OFFを検出するために両エッジ割り込みとする
	else if(iFlag.IntCommand == (1 << 1))
	{
		SwitchOff();
	}

	ResetIntCommand();
}
while(1)
{
	SleepMode(obj.VoltageFlag);

	obj.IdleCount = 0;

	while(iFlag.ExtCommand == 0 && iFlag.IntCommand == 0)
	{
		if(LL_SYSTICK_IsActiveCounterFlag()) obj.IdleCount++;
		if(obj.IdleCount > SystemUnits::GetIdleTimeMillLimit()) break;
	}

	if(iFlag.ExtCommand)
	{
		if(iFlag.ExtCommand == IOPin::LedEnable)
		{
			LedMode(Sampling, &obj, dma, pixel, Delay);
		}
		else
		{
			obj.Duty = GetNewDuty(iFlag.ExtCommand);
			pwm.SetCH1CompareValue(obj.Duty);
		}
	}
	if(iFlag.IntCommand)
	{
		ResetIntCommand();
		obj.adcValue = adc.StartSoftConvert();

		if(obj.adcValue < SystemUnits::GetUnderVoltageLimit())
		{
			obj.VoltageFlag = false;
			SwitchOff();
		}
		else
		{
			obj.VoltageFlag = true;
			SwitchOn();
		}
	}

	if(obj.LedFlag)
	{
		SendData(dma, pixel, Delay);
	}

	obj.IdleCount = 0;

	while(iFlag.ExtCommand != 0)
	{
		if(LL_SYSTICK_IsActiveCounterFlag()) obj.IdleCount++;
		if(obj.IdleCount > SystemUnits::GetIdleTimeMillLimit()) break;
	}
}
}
#endif

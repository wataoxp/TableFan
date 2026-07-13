/*
 * mylib.cpp
 * Peripheral ConfigFunction
 *
 *  Created on: Jun 29, 2025
 *      Author: wataoxp
 */


#include "mylib.h"

#if defined(STM32G0)
void RCC_Config(CoreClock source,rccStatus ClockSource)
{
	RCC_InitTypedef rcc;
	uint32_t frequency = source * (1000*1000);

	if(ClockSource == rccStatus::InternalClock)
	{
		rcc.PLLSrc = LL_RCC_PLLSOURCE_HSI;
		rcc.SysClkSrc = LL_RCC_SYS_CLKSOURCE_HSI;
	}
	else
	{
		rcc.PLLSrc = LL_RCC_PLLSOURCE_HSE;
		rcc.SysClkSrc = LL_RCC_SYS_CLKSOURCE_HSE;
	}
	rcc.AHBdiv = LL_RCC_SYSCLK_DIV_1;
	rcc.APBdiv = LL_RCC_APB1_DIV_1;

	if(frequency <= 24000000)		//PWR_CR1,VOSが1のとき以下のレイテンシを設定
	{
		rcc.Latency = LL_FLASH_LATENCY_0;
	}
	else if(frequency <= 48000000)
	{
		rcc.Latency = LL_FLASH_LATENCY_1;
	}
	else
	{
		rcc.Latency = LL_FLASH_LATENCY_2;
	}
	ConfigHighClock(&rcc,ClockSource);

	if(source > HSICLOCK)
	{
		rcc.SysClkSrc = LL_RCC_SYS_CLKSOURCE_PLL;
		rcc.PLLM = LL_RCC_PLLM_DIV_1;
		rcc.PLLN = 8;			//HSEによる
		rcc.PLLR = LL_RCC_PLLR_DIV_2;

		ConfigPLL(&rcc);
	}
}
#elif defined(STM32C0)
void RCC_Config(uint32_t HsiDiv)
{
	RCC_InitTypedef rcc;
	rcc.HSIdiv = HsiDiv;
	rcc.AHBdiv = LL_RCC_HCLK_DIV_1;
	rcc.SysClkSrc = LL_RCC_SYS_CLKSOURCE_HSI;
	rcc.APBdiv = LL_RCC_APB1_DIV_1;

	// 24MHz以上であるならレイテンシが必要
	if(rcc.HSIdiv <= LL_RCC_HSI_DIV_2)
	{
		rcc.Latency = LL_FLASH_LATENCY_1;
	}
	else
	{
		rcc.Latency = LL_FLASH_LATENCY_0;
	}

	RCC_InitC0(&rcc);
}
#endif

uint32_t RTC_Config(RealClock& rtc)
{
	using namespace RealClockSpace;

	// すでにLSEが駆動しているならLSEは触らない
	if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE)
	{
		rtc.ClockConfig();
	}

	// BDCRレジスタはリセットの影響を受けないが、APBはリセットされる
	LL_RCC_EnableRTC();
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_RTC);

	return rtc.Config(LL_RTC_HOURFORMAT_24HOUR, AsynchDefault, SynchDefault);
}

uint32_t GPIO_Config(GPIO_TypeDef *GPIOx,uint32_t pin,uint32_t Mode)
{
	uint32_t ret;
	GPIO IOPin(GPIOx,pin);
	ret = IOPin.Begin();

	switch(Mode)
	{
	case LL_GPIO_MODE_OUTPUT:
		IOPin.SetParameter(LL_GPIO_PULL_NO, Mode, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_PUSHPULL);
		IOPin.OutputInit();
		break;
	case LL_GPIO_MODE_INPUT:
		IOPin.SetParameter(LL_GPIO_PULL_UP, Mode, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_PUSHPULL);
		IOPin.InputInit();
		break;
	default:
		ret += 1;
		break;
	}

	return ret;
}

// SysClkで計測
uint32_t MCO_Config(GPIO_TypeDef *GPIOx,uint32_t Pin,uint32_t AF,uint32_t source,uint32_t Div)
{
	uint32_t ret = 0;

	GPIO MCO(GPIOx,Pin);

	ret = MCO.Begin();
	MCO.SetParameter(LL_GPIO_PULL_NO, LL_GPIO_MODE_ALTERNATE, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_PUSHPULL);
	MCO.AlternateInit(AF);

	if(source == LL_RCC_MCO1SOURCE_SYSCLK)
	{
		LL_RCC_ConfigMCO(source, Div);
	}
#ifdef LL_RCC_MCO2SOURCE_SYSCLK
	else if(source == LL_RCC_MCO2SOURCE_SYSCLK)
	{
		LL_RCC_ConfigMCO2(source, Div);
	}
#endif
	else
	{
		ret += 1;
	}
	return ret;
}

uint32_t EXTI_Config(GPIO_TypeDef *GPIOx,uint32_t pin,uint32_t Pull,uint8_t ExtiMode,uint8_t Trigger)
{
	uint32_t ret = 0;
	EXTIR exti(GPIOx,pin);
	GPIO IOPin(GPIOx,pin);

	ret += exti.Config();
	exti.ConfigMode_Trigger(ExtiMode, Trigger);

	ret += IOPin.Begin();
	IOPin.SetParameter(Pull, LL_GPIO_MODE_INPUT, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_PUSHPULL);
	IOPin.InputInit();

	return ret;
}

uint32_t ADC_Config(ADC_TypeDef *ADCx,AnalogConverter& adc,uint32_t adc_channel,GPIO_TypeDef *GPIOx,uint32_t pin,DelayPoicy& delay)
{
	using namespace AnalogParameter;

	uint32_t ret = 0;
	ADC_ConfigTypedef Config = {0};
	GPIO Conv(GPIOx,pin);

	ret = Conv.Begin();		//GPIOは初期状態でアナログ入力なのでそのままでも一応動く
	Conv.SetParameter(LL_GPIO_PULL_NO, LL_GPIO_MODE_ANALOG, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_PUSHPULL);
	Conv.InputInit();

	Config.Clock = LL_ADC_CLOCK_SYNC_PCLK_DIV2;		// PCLK/2がもっとも汎用性が高い。DIV1はPLL必須
	Config.Resolution = LL_ADC_RESOLUTION_8B;		// 分解能
	Config.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;	// 右または左寄せ

	Config.SamplingTime1 = LL_ADC_SAMPLINGTIME_160CYCLES_5;	// サンプリングサイクル
	Config.SamplingTime2 = LL_ADC_SAMPLINGTIME_160CYCLES_5;
	Config.OverRun = LL_ADC_REG_OVR_DATA_OVERWRITTEN;		// オーバーラン時にDRの値を保持or上書き

	Config.Configurability = LL_ADC_REG_SEQ_FIXED;			// 変換モード
	Config.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;	// シーケンス変換の終了位置

	Config.ExternalTrigger = SoftTrigger;

	// 外部トリガの有効、および極性の選択&外部トリガの選択。デフォルトは0
	if(Config.ExternalTrigger == ExTrigger)
	{
		Config.TriggerSource = LL_ADC_REG_TRIG_EXT_TIM3_TRGO;
		Config.TriggerEdge = LL_ADC_REG_TRIG_EXT_RISING;
	}

	ret += adc.Config(&Config,adc_channel,delay);

	LL_ADC_StartCalibration(ADCx);
	while((ADCx->CR & ADC_CR_ADCAL) != 0U);

	return ret;
}

uint32_t DMA_Config(DMA& dma,uint32_t Channel,uint32_t ReqID,uint32_t *MemoryAddress,uint32_t *PeriphAddress)
{
	DMA_InitTypdef Config ={0};

	Config.RequestID = ReqID;
	Config.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
	Config.TransferMode = LL_DMA_MODE_NORMAL;
	Config.PeriphInc = LL_DMA_PERIPH_NOINCREMENT;
	Config.MemoryInc = LL_DMA_MEMORY_INCREMENT;
	Config.PeriphSize = LL_DMA_PDATAALIGN_WORD;
	Config.MemorySize = LL_DMA_MDATAALIGN_BYTE;
	Config.Priority = LL_DMA_PRIORITY_VERYHIGH;		// 複数のDMAチャネルが競合する時の優先度？

	dma.Config(&Config, Channel);

	return dma.AddressSet(Channel, MemoryAddress,PeriphAddress);
}

uint32_t I2C_Config(CoreClock clock,I2C& i2c,GPIO_TypeDef *PortSCL,uint32_t PinSCL,uint32_t SCL_AF,GPIO_TypeDef *PortSDA,uint32_t PinSDA,uint32_t SDA_AF)
{
	uint32_t ret = 0;
	GPIO SCL(PortSCL,PinSCL);
	GPIO SDA(PortSDA,PinSDA);

	ret += SCL.Begin();
	ret += SDA.Begin();

	SCL.SetParameter(LL_GPIO_PULL_NO, LL_GPIO_MODE_ALTERNATE, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_OPENDRAIN);
	SDA.SetParameter(LL_GPIO_PULL_NO, LL_GPIO_MODE_ALTERNATE, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_OPENDRAIN);

	SCL.AlternateInit(SCL_AF);
	SDA.AlternateInit(SDA_AF);

	ret += i2c.ConfigMaster(clock);

	return ret;
}
void SPI_Config(SPI& spi)
{
	SPI_InitTypedef Config = {0};

	Config.TransferDirection = LL_SPI_FULL_DUPLEX;
	Config.Mode = LL_SPI_MODE_MASTER;		// SSIもここでセット済み
	Config.ClockPolarity = LL_SPI_POLARITY_LOW;
	Config.ClockPhase = LL_SPI_PHASE_1EDGE;
	Config.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV32;
	Config.BitOrder = LL_SPI_MSB_FIRST;
	Config.DataWidth = LL_SPI_DATAWIDTH_8BIT;

	spi.Config(&Config);
}
uint32_t SPI_PinConfig(GPIO_TypeDef *Mosi,uint32_t mosi,uint32_t mosiAF,GPIO_TypeDef *Sck,uint32_t sck,uint32_t sckAF)
{
	uint32_t ret = 0;
	GPIO _MOSI(Mosi,mosi);
	GPIO _SCK(Sck,sck);

	ret += _MOSI.Begin();
	ret += _SCK.Begin();

	_MOSI.SetParameter(LL_GPIO_PULL_NO, LL_GPIO_MODE_ALTERNATE, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_PUSHPULL);
	_SCK.SetParameter(LL_GPIO_PULL_NO, LL_GPIO_MODE_ALTERNATE, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_PUSHPULL);

	_MOSI.AlternateInit(mosiAF);
	_SCK.AlternateInit(sckAF);

	return ret;
}
uint32_t SPI_NSS_Config(SPI& spi,GPIO_TypeDef *GPIOx,uint32_t Pin,uint32_t Mode)
{
	uint32_t ret = 0;
	GPIO NSS(GPIOx,Pin);

 	ret += NSS.Begin();

 	switch(Mode)
 	{
 	case LL_SPI_NSS_HARD_OUTPUT:
 		NSS.SetParameter(LL_GPIO_PULL_NO, LL_GPIO_MODE_ALTERNATE, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_PUSHPULL);
		NSS.AlternateInit(LL_GPIO_AF_0);
		break;
 	case LL_SPI_NSS_SOFT:
 		NSS.SetParameter(LL_GPIO_PULL_NO, LL_GPIO_MODE_OUTPUT, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_PUSHPULL);
 		NSS.OutputInit();
 		break;
 	default:
 		break;
 	}

 	spi.ConfigNSS(GPIOx, Pin, Mode);

 	return ret;
}
uint32_t SPI_MISO_Config(GPIO_TypeDef *GPIOx,uint32_t Pin)
{
	uint32_t ret = 0;
	GPIO MISO(GPIOx,Pin);

	ret += MISO.Begin();
	MISO.SetParameter(LL_GPIO_PULL_NO, LL_GPIO_MODE_ALTERNATE, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_PUSHPULL);
	MISO.AlternateInit(LL_GPIO_AF_0);

	return ret;
}

uint32_t UART_Config(UART& uart,CoreClock sysclk,GPIO_TypeDef *TxPort,uint32_t TxPin,uint32_t TxAf,GPIO_TypeDef *RxPort,uint32_t RxPin,uint32_t RxAf)
{
	uint32_t ret = 0;

	Serial::UART_InitTypedef init;

	// ワード長のセット。パリティを含んだ値なので受信・解析時は‐1ビットで確認
	init.DataWidth = LL_USART_DATAWIDTH_9B;

	// パリティの選択
	init.Parity = LL_USART_PARITY_ODD;

	// 転送方向の設定
	init.Direction = LL_USART_DIRECTION_TX_RX;

	// データのサンプリング回数の設定
	init.OverSampling = LL_USART_OVERSAMPLING_16;

	// ストップビットの数を設定
	init.StopBits = LL_USART_STOPBITS_1;

	// 対応した機器(RS232C等)のみ
	init.HardWareControl = LL_USART_HWCONTROL_NONE;

	// ボーレートの設定
	init.PreScalerDiv = LL_USART_PRESCALER_DIV1;
	init.BaudRate = 115200;
//	init.BaudRate = 19200;

	// FIFOの設定
	init.TxFifoThreshold = LL_USART_FIFOTHRESHOLD_1_8;
	init.RxFifoThreshold = LL_USART_FIFOTHRESHOLD_1_8;
	init.FifoEN = Serial::SerialParameters::Fifo_Disable;

	init.SelectMode = Serial::SerialParameters::AsyncMode;

	if(init.Direction == LL_USART_DIRECTION_TX_RX)
	{
		UART_RxConfig(RxPort, RxPin, RxAf);
		UART_TxConfig(TxPort, TxPin, TxAf);
	}
	else if(init.Direction == LL_USART_DIRECTION_TX)
	{
		UART_TxConfig(TxPort, TxPin, TxAf);
	}
	else if(init.Direction == LL_USART_DIRECTION_RX)
	{
		UART_RxConfig(TxPort, TxPin, TxAf);
	}
	else
	{
		ret = 1;
	}

	uart.Config(&init,(sysclk*1000*1000));

	return ret;
}


uint32_t UART_RxConfig(GPIO_TypeDef *RxPort,uint32_t RxPin,uint32_t RxAf)
{
	uint32_t ret = 0;
	GPIO RX(RxPort,RxPin);

	ret = RX.Begin();
	RX.SetParameter(LL_GPIO_PULL_NO, LL_GPIO_MODE_ALTERNATE, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_PUSHPULL);
	RX.AlternateInit(RxAf);

	return ret;
}

uint32_t UART_TxConfig(GPIO_TypeDef *TxPort,uint32_t TxPin,uint32_t TxAf)
{
	uint32_t ret = 0;
	GPIO TX(TxPort,TxPin);

	ret = TX.Begin();
	TX.SetParameter(LL_GPIO_PULL_NO, LL_GPIO_MODE_ALTERNATE, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_PUSHPULL);
	TX.AlternateInit(TxAf);

	return ret;
}

void TIM_Config(TIM& tim,uint32_t Prescaler,uint32_t Reload)
{
	TimerPeripheral::TIM_InitTypedef Config = {0};

	Config.CountMode = LL_TIM_CLOCKDIVISION_DIV1;
	Config.Source = LL_TIM_CLOCKSOURCE_INTERNAL;	// 内部クロックを利用
	Config.Prescale = (Prescaler)? (Prescaler-1):(Prescaler);
	Config.Reload = (Reload)? (Reload-1):(Reload);
	Config.CountMode = LL_TIM_COUNTERDIRECTION_UP;
	Config.Division = LL_TIM_CLOCKDIVISION_DIV1;	// サンプリング周波数fDTSの分周比を設定

	tim.ConfigTimer(&Config);
}

uint32_t PWM_Config(TIM& tim,GPIO_TypeDef *GPIOx,uint32_t pin,uint32_t Alternate,uint32_t Channel,uint32_t Mode)
{
	uint32_t ret = 0;
	GPIO Pulse(GPIOx,pin);

	ret = Pulse.Begin();
	Pulse.SetParameter(LL_GPIO_PULL_NO, LL_GPIO_MODE_ALTERNATE, LL_GPIO_SPEED_FREQ_LOW, LL_GPIO_OUTPUT_PUSHPULL);
	Pulse.AlternateInit(Alternate);

	tim.ConfigPWM(Channel, Mode);

	return ret;
}




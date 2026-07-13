/*
 * adc.h
 *
 *  Created on: Jun 3, 2025
 *      Author: wataoxp
 */

#ifndef ADC_H_
#define ADC_H_

#include "periph.h"
#include "delay.h"

namespace AnalogParameter{
	typedef enum{
		SoftTrigger,
		ExTrigger,
	}StatusADC;

	typedef struct{
		/* CFGR1 */
		uint32_t Resolution; 		//RES
		uint32_t DataAlignment;		//ALGIN

		uint32_t TriggerSource;		//EXTSEL
		uint32_t TriggerEdge;		//EXTEN
		uint32_t OverRun;			//OVRMOD

		uint32_t Configurability;	//CHSELRMOD

		/* CFGR2 */
		uint32_t Clock;				//CKMODE

		/* SMPR */
		uint32_t SamplingTime1;		//SMP1
		uint32_t SamplingTime2;		//SMP2

		/* CHSELR */
		uint32_t SequencerLength;


		/* Other */
		StatusADC ExternalTrigger;
	}ADC_ConfigTypedef;
}

class AnalogConverter{
private:
	ADC_TypeDef *ADCx;
	uint32_t RegConfig(AnalogParameter::ADC_ConfigTypedef *RegConfig,uint32_t Channel);
	void SingleMode(uint32_t Channel);
	uint32_t SequenceMode(uint32_t Channel,uint32_t length);
public:
	AnalogConverter(ADC_TypeDef *ADCPort);
	uint32_t Config(AnalogParameter::ADC_ConfigTypedef* Config,uint32_t Channel,DelayPoicy& delay);

	uint16_t StartSoftConvert(void);
	void DisableADC(void);

	// inline
	void StartHwTrigger(void);
	void EnableADC(void);
	void SetISR(void);
};

inline void AnalogConverter::StartHwTrigger(void)
{
	LL_ADC_REG_StartConversion(ADCx);
}
inline void AnalogConverter::EnableADC(void)
{
	LL_ADC_Enable(ADCx);
	while(LL_ADC_IsActiveFlag_ADRDY(ADCx) == 0);
}
inline void AnalogConverter::SetISR(void)
{
	LL_ADC_EnableIT_EOC(ADCx);
}




#endif /* ADC_H_ */

/*
 * mylib.h
 *
 *  Created on: Jun 15, 2025
 *      Author: wataoxp
 */

#ifndef INC_MYLIB_H_
#define INC_MYLIB_H_

#include "periph.h"

#include "gpio.h"
#include "rcc.h"
#include "delay.h"
#include "exti.h"

#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "adc.h"
#include "dma.h"
#include "rtc.h"
#include "uart.h"

#ifdef STM32G0
void RCC_Config(CoreClock source,rccStatus ClockSource);
#endif
#ifdef STM32C0
void RCC_Config(uint32_t HsiDiv);
#endif

uint32_t RTC_Config(RealClock& rtc);
void RTC_StructInit(RealClockSpace::AlarmParameters *init);

uint32_t GPIO_Config(GPIO_TypeDef *GPIOx,uint32_t pin,uint32_t Mode);
uint32_t MCO_Config(GPIO_TypeDef *GPIOx,uint32_t Pin,uint32_t AF,uint32_t source,uint32_t Div);
uint32_t EXTI_Config(GPIO_TypeDef *GPIOx,uint32_t pin,uint32_t Pull,uint8_t Mode,uint8_t Trigger);
uint32_t ADC_Config(ADC_TypeDef *ADCx,AnalogConverter& adc,uint32_t adc_channel,GPIO_TypeDef *GPIOx,uint32_t pin,DelayPoicy& delay);
uint32_t DMA_Config(DMA& dma,uint32_t Channel,uint32_t ReqID,uint32_t *MemoryAddress,uint32_t *PeriphAddress);

uint32_t I2C_Config(CoreClock clock,I2C& i2c,GPIO_TypeDef *PortSCL,uint32_t PinSCL,uint32_t SCL_AF,GPIO_TypeDef *PortSDA,uint32_t PinSDA,uint32_t SDA_AF);

void SPI_Config(SPI& spi);
uint32_t SPI_PinConfig(GPIO_TypeDef *Mosi,uint32_t mosi,uint32_t mosiAF,GPIO_TypeDef *Sck,uint32_t sck,uint32_t sckAF);
uint32_t SPI_NSS_Config(SPI& spi,GPIO_TypeDef *GPIOx,uint32_t Pin,uint32_t Mode);
uint32_t SPI_MISO_Config(GPIO_TypeDef *GPIOx,uint32_t Pin);

uint32_t UART_Config(UART& uart,CoreClock sysclk,GPIO_TypeDef *TxPort,uint32_t TxPin,uint32_t TxAf,GPIO_TypeDef *RxPort,uint32_t RxPin,uint32_t RxAf);
uint32_t UART_RxConfig(GPIO_TypeDef *RxPort,uint32_t RxPin,uint32_t RxAf);
uint32_t UART_TxConfig(GPIO_TypeDef *TxPort,uint32_t TxPin,uint32_t TxAf);

void TIM_Config(TIM& tim,uint32_t Prescaler,uint32_t Reload);
uint32_t PWM_Config(TIM& tim,GPIO_TypeDef *GPIOx,uint32_t pin,uint32_t Alternate,uint32_t Channel,uint32_t Mode);



#endif /* INC_MYLIB_H_ */

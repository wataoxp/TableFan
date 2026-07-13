/*
 * i2c.c
 *
 *  Created on: Nov 5, 2024
 *      Author: wataoxp
 */

#include "i2c.h"

using namespace Wires;

I2C::I2C(I2C_TypeDef *pI2C,DelayPoicy& pDelay) :I2Cx(pI2C),delay(pDelay)
{
	;
}
inline uint8_t I2C::GetMemAddLowByte(uint16_t address)
{
	return (uint8_t)(address & 0xFF);
}
inline uint8_t I2C::GetMemAddHighByte(uint16_t address)
{
	return (uint8_t)(address >> 8);
}

inline uint32_t I2C::GetWireBus(void)
{
	uint32_t Periphs = 0;

	if(I2Cx == I2C1)
	{
		Periphs = LL_APB1_GRP1_PERIPH_I2C1;
	}
#ifdef I2C2
	else if(I2Cx == I2C2)
	{
		Periphs = LL_APB1_GRP1_PERIPH_I2C2;
	}
#endif
#ifdef I2C3
	else if(I2Cx == I2C3)
	{
		Periphs = LL_APB1_GRP1_PERIPH_I2C2;
	}
#endif
	else
	{
		Periphs = Failed;
	}

	return Periphs;
}

/* Config */

ReturnCode I2C::ConfigMaster(CoreClock clock)
{
	uint32_t Periphs;
	uint32_t Timing;

	Periphs = GetWireBus();

	if(Periphs == Failed)
	{
		return Failed;
	}

	Timing = GetClockTiming(clock);

	LL_APB1_GRP1_EnableClock(Periphs);

	LL_I2C_Disable(I2Cx);
	LL_I2C_ConfigFilters(I2Cx, LL_I2C_ANALOGFILTER_ENABLE,0);
	LL_I2C_SetTiming(I2Cx, Timing);

	LL_I2C_EnableAutoEndMode(I2Cx);
	LL_I2C_EnableClockStretching(I2Cx);

	LL_I2C_Enable(I2Cx);

	return Complete;
}

void I2C::ConfigSlave(CoreClock source,uint8_t OwnAddr)
{
	uint32_t Timing = GetClockTiming(source);

	LL_I2C_Disable(I2Cx);
	LL_I2C_ConfigFilters(I2Cx, LL_I2C_ANALOGFILTER_ENABLE,0);
	LL_I2C_SetTiming(I2Cx, Timing);

	/* I2C Interrupt */
	LL_I2C_EnableIT_ADDR(I2Cx);
	LL_I2C_EnableIT_STOP(I2Cx);
	LL_I2C_EnableIT_RX(I2Cx);
	LL_I2C_EnableIT_TX(I2Cx);		//TXフラグはすぐに立てるべきではない？

	/* Slave Mode */
	LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_ACK);
	LL_I2C_DisableGeneralCall(I2Cx);
	LL_I2C_SetOwnAddress1(I2Cx, (OwnAddr << OAR1_BitPos), LL_I2C_OWNADDRESS1_7BIT);
	LL_I2C_EnableOwnAddress1(I2Cx);
	LL_I2C_DisableOwnAddress2(I2Cx);

	LL_I2C_Enable(I2Cx);
}

/* DeviceCheck */

// ISR & FlagがCheckPatternに一致するかチェック
uint32_t I2C::WaitFlag(uint32_t Mask,uint32_t FlagBit)
{
	uint32_t Ret = 1;
	uint32_t TimeCount = WaitMillTimeLimit();

	delay.WaitSetUp();

	while(TimeCount)
	{
		if(delay.GetWaitFlag())
		{
			TimeCount--;
		}
		if((I2Cx->ISR & Mask) == FlagBit)
		{
			Ret = 0;
			break;
		}
	}
	return Ret;
}

bool I2C::IsActiveDevice(uint8_t addr)
{
	uint32_t isNACK = 0;

	LL_I2C_HandleTransfer(I2Cx, addr, LL_I2C_ADDRSLAVE_7BIT, 0, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	// AUTOENDかつNBYTEが0なのでACKorNACKを待って終了
	WaitFlag(I2C_ISR_STOPF,I2C_ISR_STOPF);
	LL_I2C_ClearFlag_STOP(I2Cx);

	isNACK = LL_I2C_IsActiveFlag_NACK(I2Cx);
	LL_I2C_ClearFlag_NACK(I2Cx);

	return (isNACK == 0)? true:false;
}

uint8_t I2C::GetDeviceAddress()
{
	uint8_t DeviceAddress = 0;

	for(DeviceAddress = 0;DeviceAddress < UINT8_MAX;DeviceAddress++)
	{
		if(IsActiveDevice(DeviceAddress))
		{
			break;
		}
	}
	return (DeviceAddress < UINT8_MAX)? (DeviceAddress >> 1):UINT8_MAX;
}

/* Connection */

// データのみ
ReturnCode I2C::Transmit(uint8_t addr,uint8_t *TxBuf,uint8_t length)
{
	uint32_t Ret = 0;

	Ret = WaitFlag(I2C_ISR_BUSY,0);

	LL_I2C_HandleTransfer(I2Cx, addr, LL_I2C_ADDRSLAVE_7BIT, length, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	for(uint8_t i = 0;i < length;i++)
	{
		LL_I2C_TransmitData8(I2Cx, TxBuf[i]);
		Ret = WaitFlag(I2C_ISR_TXE,I2C_ISR_TXE);
	}

	Ret = WaitFlag(I2C_ISR_STOPF,I2C_ISR_STOPF);
	LL_I2C_ClearFlag_STOP(I2Cx);

	return (Ret == 0)? Complete:TimeOut;
}

// デバイス内部のアドレスを指定
ReturnCode I2C::MemWrite(uint8_t addr,uint16_t Reg,MemAdd MemAddSize,uint8_t *TxBuf,uint8_t length)
{
	uint32_t Ret = 0;

	Ret = WaitFlag(I2C_ISR_BUSY,0);

	LL_I2C_HandleTransfer(I2Cx, addr, LL_I2C_ADDRSLAVE_7BIT, (length+MemAddSize), LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	if(MemAddSize == MemAddSize8)
	{
		LL_I2C_TransmitData8(I2Cx, GetMemAddLowByte(Reg));
		Ret = WaitFlag(I2C_ISR_TXE,I2C_ISR_TXE);
	}
	else
	{
		LL_I2C_TransmitData8(I2Cx, GetMemAddHighByte(Reg));
		Ret = WaitFlag(I2C_ISR_TXE,I2C_ISR_TXE);
		LL_I2C_TransmitData8(I2Cx, GetMemAddLowByte(Reg));
		Ret = WaitFlag(I2C_ISR_TXE,I2C_ISR_TXE);
	}

	for(uint8_t i = 0;i < length;i++)
	{
		LL_I2C_TransmitData8(I2Cx, TxBuf[i]);
		Ret = WaitFlag(I2C_ISR_TXE,I2C_ISR_TXE);
	}

	Ret = WaitFlag(I2C_ISR_STOPF,I2C_ISR_STOPF);
	LL_I2C_ClearFlag_STOP(I2Cx);

	return (Ret == 0)? Complete:TimeOut;
}

ReturnCode I2C::MemRead(uint8_t addr,uint16_t Reg,MemAdd MemAddSize,uint8_t *RxBuf,uint8_t length)
{
	uint32_t Ret = 0;

	Ret = WaitFlag(I2C_ISR_BUSY,0);
	LL_I2C_HandleTransfer(I2Cx, addr, LL_I2C_ADDRSLAVE_7BIT, MemAddSize, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);

	if(MemAddSize == MemAddSize8)
	{
		LL_I2C_TransmitData8(I2Cx, GetMemAddLowByte(Reg));
		Ret = WaitFlag(I2C_ISR_TXE,I2C_ISR_TXE);
	}
	else
	{
		LL_I2C_TransmitData8(I2Cx, GetMemAddHighByte(Reg));
		Ret = WaitFlag(I2C_ISR_TXE,I2C_ISR_TXE);
		LL_I2C_TransmitData8(I2Cx, GetMemAddLowByte(Reg));
		Ret = WaitFlag(I2C_ISR_TXE,I2C_ISR_TXE);
	}

	Ret = WaitFlag(I2C_ISR_TC, I2C_ISR_TC);

	LL_I2C_HandleTransfer(I2Cx, addr, LL_I2C_ADDRSLAVE_7BIT, length, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);

	for(uint8_t i = 0;i < length;i++)
	{
		Ret = WaitFlag(I2C_ISR_RXNE,I2C_ISR_RXNE);
		RxBuf[i] = LL_I2C_ReceiveData8(I2Cx);
	}

	Ret = WaitFlag(I2C_ISR_STOPF,I2C_ISR_STOPF);
	LL_I2C_ClearFlag_STOP(I2Cx);

	return (Ret == 0)? Complete:TimeOut;
}

//Reg1個だけ
ReturnCode I2C::Write(uint8_t addr,uint8_t Reg)
{
	uint32_t Ret = 0;
	Ret = WaitFlag(I2C_ISR_BUSY,0);

	LL_I2C_HandleTransfer(I2Cx, addr, LL_I2C_ADDRSLAVE_7BIT, 1, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	LL_I2C_TransmitData8(I2Cx, Reg);
	Ret = WaitFlag(I2C_ISR_TXE,I2C_ISR_TXE);

	Ret = WaitFlag(I2C_ISR_STOPF,I2C_ISR_STOPF);
	LL_I2C_ClearFlag_STOP(I2Cx);

	return (Ret == 0)? Complete:TimeOut;
}

//RegとDataを1個ずつ
ReturnCode I2C::Write(uint8_t addr,uint8_t Reg,uint8_t Data)
{
	uint32_t Ret = 0;

	Ret = WaitFlag(I2C_ISR_BUSY,0);

	LL_I2C_HandleTransfer(I2Cx, addr, LL_I2C_ADDRSLAVE_7BIT, 2, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	LL_I2C_TransmitData8(I2Cx, Reg);
	Ret = WaitFlag(I2C_ISR_TXE,I2C_ISR_TXE);

	LL_I2C_TransmitData8(I2Cx, Data);
	Ret = WaitFlag(I2C_ISR_TXE,I2C_ISR_TXE);

	Ret = WaitFlag(I2C_ISR_STOPF,I2C_ISR_STOPF);
	LL_I2C_ClearFlag_STOP(I2Cx);

	return (Ret == 0)? Complete:TimeOut;
}

#if 0
//bool I2C::IsActiveDevice(uint8_t addr)
//{
//	uint32_t Ret = 0;
//
////	WaitFlag(I2C_ISR_BUSY);
//	LL_I2C_HandleTransfer(I2Cx, addr, LL_I2C_ADDRSLAVE_7BIT, 1, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
//
//	LL_I2C_TransmitData8(I2Cx, 0);
//	Ret = WaitFlag(I2C_ISR_TXE,I2C_ISR_TXE);
////	WaitFlag(I2C_ISR_STOPF,SET);
//	LL_I2C_ClearFlag_STOP(I2Cx);
//	LL_I2C_ClearFlag_NACK(I2Cx);
//
//	return (Ret == 0)? true:false;
//}

//uint32_t I2C::WaitFlag(uint32_t Flag)
//{
//	uint32_t Ret = 1;
//	uint32_t TimeCount = 0;
//
//	while(TimeCount < TimeOutLimit)
//	{
//		delay.uDelay(WaitTime::MicroTime);
//		if(I2Cx->ISR & Flag)
//		{
//			Ret = 0;
//			break;
//		}
//		TimeCount += WaitTime::MicroTime;
//	}
//	return Ret;
//}
#endif

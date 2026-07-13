/*
 * i2c.h
 *
 *  Created on: Nov 5, 2024
 *      Author: wataoxp
 */

#ifndef INC_I2C_H_
#define INC_I2C_H_

#include "periph.h"
#include "delay.h"

namespace Wires{
	typedef enum{
		Complete,
		TimeOut,
		Failed,
		TxOver,
		RxOver,
	}ReturnCode;

	typedef enum{
		MemAddSize8 = 1,
		MemAddSize16 = 2,
	}MemAdd;

	// I2C_OAR1レジスタ設定用。7ビットアドレスの場合最下位ビット(OA１)は無視されるので１ビットシフトする
	constexpr uint8_t OAR1_BitPos = 1;

#ifdef STM32C011xx
	// 400kHz
	constexpr uint32_t GetClockTiming(CoreClock source)
	{
		return 0x0090194B;
	}
#else

	// 400kHz
	constexpr uint32_t GetClockTiming(CoreClock source)
	{
		if(source == PLLCLOCK) return 0x00C12166;
		else return 0x00300617;
	}
#endif

	constexpr uint32_t WaitMillTimeLimit() { return 100U; }
}

class I2C{
private:
	I2C_TypeDef *I2Cx;
	DelayPoicy& delay;
	uint8_t GetMemAddLowByte(uint16_t address);
	uint8_t GetMemAddHighByte(uint16_t address);
	uint32_t GetWireBus(void);

	uint32_t WaitFlag(uint32_t Mask,uint32_t FlagBit);
public:
	I2C(I2C_TypeDef *pI2C,DelayPoicy& pDelay);
	Wires::ReturnCode ConfigMaster(CoreClock clock);
	void ConfigSlave(CoreClock source,uint8_t OwnAddr);

	bool IsActiveDevice(uint8_t addr);
	uint8_t GetDeviceAddress();

	Wires::ReturnCode Transmit(uint8_t addr,uint8_t *TxBuf,uint8_t length);
	Wires::ReturnCode MemWrite(uint8_t addr,uint16_t Reg,Wires::MemAdd MemAddSize,uint8_t *TxBuf,uint8_t length);
	Wires::ReturnCode MemRead(uint8_t addr,uint16_t Reg,Wires::MemAdd MemAddSize,uint8_t *RxBuf,uint8_t length);

	Wires::ReturnCode Write(uint8_t addr,uint8_t Reg);
	Wires::ReturnCode Write(uint8_t addr,uint8_t Reg,uint8_t Data);

	void Enable(void);
	void Disable(void);
};

inline void I2C::Enable(void)
{
	LL_I2C_Enable(I2Cx);
}

inline void I2C::Disable(void)
{
	LL_I2C_Disable(I2Cx);
	/* P935、PEクリア後は3APBクロック分0で維持する。APB分周無と仮定したウェイト */
	__NOP();
	__NOP();
	__NOP();
}



#endif /* INC_I2C_H_ */

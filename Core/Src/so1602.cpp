/*
 * so1602.c
 *
 *  Created on: Mar 1, 2025
 *      Author: wataoxp
 */
#include "so1602.h"
#include "delay.h"

SO1602::SO1602(I2C& i2c,DelayPoicy& pDelay) : wire(i2c),delay(pDelay)
{
	;
}

uint32_t SO1602::Init()
{
	uint8_t config[] = {CLEAR_DISPLAY,RETURN_HOME,DISP_CMD,CLEAR_DISPLAY};
	uint8_t contrast[] = {0x2a,0x79,0x81,0xff,0x78,0x28};

	// 秋月のマニュアルにもあるウェイト。これをしないとアドレス送信でNACKが返される
	delay.mDelay(100);

	for(uint8_t i = 0;i < sizeof(config);i++)
	{
		wire.Write(SO1602_ADDRESS, REG_CMD, config[i]);
		delay.mDelay(20);
	}
	for(uint8_t j = 0;j < sizeof(contrast);j++)
	{
		wire.Write(SO1602_ADDRESS, REG_CMD, contrast[j]);
	}

	return 0;
}

void SO1602::PointClear(uint8_t y)
{
	char Clear[16];

	y &= 1;		// y座標のマスク

	for(uint8_t i = 0;i < sizeof(Clear);i++)
	{
		Clear[i] = 0xA0;		//半角スペース
	}
	SetCusor(0, y);
	StringLCD(Clear, sizeof(Clear));
	SetCusor(0, y);
}

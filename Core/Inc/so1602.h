/*
 * so1602.h
 *
 *  Created on: Mar 1, 2025
 *      Author: wataoxp
 */

#ifndef INC_SO1602_H_
#define INC_SO1602_H_

#include "i2c.h"
#include "delay.h"

//SA0 == Low
#define SO1602_ADDRESS (0x3C << 1)

#define REG_CMD 0x00
#define REG_DATA 0x40

#define CLEAR_DISPLAY 0x01
#define RETURN_HOME 0x02

/* SO1602 DDRAM */
/****************
 * Row1|0x00~0x0F
 * Row2|0x20~0x2F
 ****************/
#define HOME_CUSOR 0x00
#define ENTER_CUSOR 0x20

#define DISP_ON 0x04
#define CUSOR_ON 0x02
#define BLINK_CUSOR 0x01
#define DISP_CMD 0x08 | DISP_ON

#define DDRAM_ACCESS 0x80

class SO1602{
private:
	I2C& wire;
	DelayPoicy& delay;
public:
	SO1602(I2C& i2c,DelayPoicy& pDelay);
	uint32_t Init();
	void PointClear(uint8_t y);

	void StringLCD(const char *str,uint8_t size);
	void ClearLCD(void);
	void SetCusor(uint8_t x,uint8_t y);
	void SendCMD(uint8_t cmd);
};
inline void SO1602::StringLCD(const char *str,uint8_t size)
{
	wire.MemWrite(SO1602_ADDRESS, REG_DATA, Wires::MemAddSize8, (uint8_t*)str, size);
}
inline void SO1602::ClearLCD(void)
{
	wire.Write(SO1602_ADDRESS, REG_CMD, CLEAR_DISPLAY);
}
inline void SO1602::SetCusor(uint8_t x,uint8_t y)
{
	wire.Write(SO1602_ADDRESS, REG_CMD, (DDRAM_ACCESS | (x + y * ENTER_CUSOR)));
}
inline void SO1602::SendCMD(uint8_t cmd)
{
	wire.Write(SO1602_ADDRESS, REG_CMD, cmd);
}

#endif /* INC_SO1602_H_ */

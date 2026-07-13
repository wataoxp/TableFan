/*
 * ws2812.cpp
 *
 *  Created on: May 24, 2026
 *      Author: wataoxp
 */

#include "ws2812.h"
#include <string.h>

using namespace LEDParts;

ws2812::ws2812(uint8_t* buffer,uint16_t* Hue,LEDParts::NeoPixelTypedef* Pixel,uint32_t num) :
	BufferAddress(buffer),HueAddress(Hue),pixel(Pixel),NumLed(num),param{}
{
	// 初期値の設定
	param.Lux = UINT8_MAX;
	param.Color = UINT8_MAX;
	param.Shift = UINT8_MAX;
}

ReturnCode ws2812::SetParameter(uint8_t Lux,uint8_t Color,uint8_t Shift)
{
	if(Lux < LuxMin || Lux > LuxMax)
	{
		return OverParameter;
	}

	param.Lux = Lux;
	param.Color = Color & ColorTypeMax;
	param.Shift = Shift;

	return Success;
}

ReturnCode ws2812::BuildBufferData()
{
	if(param.Lux == UINT8_MAX || param.Color == UINT8_MAX || param.Shift == UINT8_MAX)
	{
		return EmptyParameter;
	}

	CreateHueData();
	for(uint32_t pix = 0;pix < NumLed;pix++)
	{
		HueToRGB(HueAddress[pix],pix);
		SetLux(pix);
	}
	MakeBuffer();

	return Success;
}

void ws2812::CreateHueData()
{
	// 色相360をLED数で割り、すべての色相にLEDを分配。
	// angle*iでi番目のLEDの色相値が決まる
	uint16_t angle = (360 / NumLed) * param.Shift;
	uint32_t color = param.Color & ColorTypeMax;

	// すべてのLED(i)のHueデータを作る
	for(uint32_t i = 0; i < NumLed;i++)
	{
		HueAddress[i] = i * angle;
		SetColor(color, HueAddress, i);
	}
}

inline void ws2812::SetColor(uint32_t color,uint16_t* val,uint32_t num)
{
	switch(color)
	{
	case Rainbow:
		val[num] %= 360;
		break;
	case Red:
		val[num] %= 0x3F;	// 63を上限にする
		break;
	case Green:
		val[num] = (val[num] & 0x3F) + 90;
		break;
	case Blue:
		val[num]= (val[num] & 0x3F) + 180;
		break;
	case Neon:
		MoveNeon(val,num);
		break;
	}
}

void ws2812::MoveNeon(uint16_t *val,uint8_t num)
{
	switch(num & 0x0f)
	{
		case 1:
		case 3:
			val[num] = 330;
			break;
		case 2:
		case 5:
			val[num] = 0;
			break;
		case 4:
		case 6:
			val[num] = 60;
			break;
		default:
			val[num] = 240;
			break;
	}
}

void ws2812::HueToRGB(uint16_t Hue,uint32_t num)
{
	if(Hue <= 60)
	{
		pixel[num].bit24.r = 255;
		pixel[num].bit24.g = ((Hue * FixedPoint) >> FixedShift);
		pixel[num].bit24.b = 0;
	}
	else if(Hue <= 120)
	{
		Hue = 120 - Hue;
		pixel[num].bit24.r = ((Hue * FixedPoint) >> FixedShift);
		pixel[num].bit24.g = 255;
		pixel[num].bit24.b = 0;
	}
	else if(Hue <= 180)
	{
		Hue -= 120;
		pixel[num].bit24.r = 0;
		pixel[num].bit24.g = 255;
		pixel[num].bit24.b = ((Hue * FixedPoint) >> FixedShift);
	}
	else if(Hue <= 240)
	{
		Hue = 240 - Hue;
		pixel[num].bit24.r = 0;
		pixel[num].bit24.g = ((Hue * FixedPoint) >> FixedShift);
		pixel[num].bit24.b = 255;
	}
	else if(Hue <= 300)
	{
		Hue -= 240;
		pixel[num].bit24.r = ((Hue * FixedPoint) >> FixedShift);
		pixel[num].bit24.g = 0;
		pixel[num].bit24.b = 255;
	}
	else if(Hue <= 360)
	{
		Hue = 360 - Hue;
		pixel[num].bit24.r = 255;
		pixel[num].bit24.g = 0;
		pixel[num].bit24.b = ((Hue * FixedPoint) >> FixedShift);
	}
}

void ws2812::SetLux(uint32_t num)
{
	// 0で消灯、8で全灯
	uint8_t Lux = LuxMax - param.Lux;

	pixel[num].bit24.r >>= Lux;
	pixel[num].bit24.g >>= Lux;
	pixel[num].bit24.b >>= Lux;
}

void ws2812::MakeBuffer()
{
	uint8_t *ptr = BufferAddress;

	for(uint32_t i = 0;i < NumLed; i++)
	{
		for(int32_t mask = 23; mask >= 0; mask--)	//1LED,24byte
		{
			// LowDuty*2でだいたいHighDutyの値
			*ptr++ = LowDuty << ((pixel[i].fullcolor >> mask) & 0x01);
		}
	}
}

void ws2812::ResetAllLed()
{
	uint8_t *ptr = BufferAddress;

	for(uint32_t i = 0;i < NumLed; i++)
	{
		for(int32_t mask = 23; mask >= 0; mask--)
		{
			*ptr++ = LowDuty;
		}
	}
}

//ShiftPixel->24byte
void ws2812::ShiftPixel()
{
	uint8_t* srcBuf = BufferAddress;
	uint8_t tmpBuf[24];

	memcpy(tmpBuf,srcBuf,24);
	memmove(srcBuf,srcBuf+24,(NumLed - 1)*24);
	memcpy(srcBuf+(NumLed - 1)*24,tmpBuf,24);
}





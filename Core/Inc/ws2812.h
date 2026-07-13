/*
 * ws2812.h
 *
 *  Created on: May 24, 2026
 *      Author: wataoxp
 */

#ifndef INC_WS2812_H_
#define INC_WS2812_H_

#include "stm32c0xx.h"
#include <stdint.h>
#include <stddef.h>

namespace LEDParts{
	// HighDuty = 周期の70%.LowDuty = 周期の30%.
	enum DataPulse{
		HighDuty = 11U,
		LowDuty = 5U,

		Bit = 24U,	// 1バイトがLEDにとっての1ビットになるので1LEDにつき24バイト必要になる
		ResetPulse = 1U,
	};

	//HSV変換用固定小数
	enum HueData{
		FixedShift = 3U,
		FixedPoint = (255 * (1 << FixedShift)) / 60,
	};

	enum Parameter{
		LuxMin = 0U,
		LuxMax = 8U,
	};

	enum ColorType{
		Rainbow,
		Red,
		Green,
		Blue,
		Neon,

		ColorTypeMax = 7,
	};

	enum ReturnCode{
		Success,
		EmptyParameter,
		OverParameter,
	};

	//STMはリトルエンディアンなのでメモリ上では以下の順番でgrbの順となる
	typedef struct{
		uint8_t b;
		uint8_t r;
		uint8_t g;
	}RGBcolor;

	typedef union{
		uint32_t fullcolor;
		RGBcolor bit24;
	}NeoPixelTypedef;

	typedef struct{
		uint8_t Lux;
		uint8_t Color;
		uint8_t Shift;
	}LedStatus;
}

class ws2812{
private:
	uint8_t* BufferAddress;
	uint16_t* HueAddress;
	LEDParts::NeoPixelTypedef* pixel;
	uint32_t NumLed;
	LEDParts::LedStatus param;

	void CreateHueData();
	void SetColor(uint32_t color,uint16_t* val,uint32_t num);
	void HueToRGB(uint16_t Hue,uint32_t num);
	void SetLux(uint32_t num);
	void MakeBuffer();
	void MoveNeon(uint16_t *val,uint8_t num);
public:
	ws2812(uint8_t* buffer,uint16_t* Hue,LEDParts::NeoPixelTypedef* Pixel,uint32_t num);
	LEDParts::ReturnCode SetParameter(uint8_t Lux,uint8_t Color,uint8_t Shift);
	LEDParts::ReturnCode BuildBufferData();
	void ShiftPixel();
	void ResetAllLed();
};

// LEDの数をsize_tに格納し、各バッファを管理する
template <size_t LedCount>
class NeoPixelSystem{
private:
	uint8_t Buffer[(LedCount * LEDParts::Bit) + LEDParts::ResetPulse] = {};
	uint16_t HueBuffer[LedCount] = {};
	LEDParts::NeoPixelTypedef Pixel[LedCount] = {};
	ws2812 led;
	uint32_t Length = (LedCount * LEDParts::Bit) + LEDParts::ResetPulse;

public:
	// コンストラクタでws2812インスタンスを生成する
	NeoPixelSystem() :led(Buffer,HueBuffer,Pixel,LedCount)
	{
		;
	}

	LEDParts::ReturnCode ParameterSet(uint8_t Lux,uint8_t Color,uint8_t Shift)
	{
		return led.SetParameter(Lux, Color, Shift);
	}

	LEDParts::ReturnCode CreateBuffer()
	{
		return led.BuildBufferData();
	}

	void Shift()
	{
		led.ShiftPixel();
	}

	void Reset()
	{
		led.ResetAllLed();
	}

	uint8_t* GetBufferAddress()
	{
		return Buffer;
	}

	uint32_t GetLength()
	{
		return Length;
	}
};


/* 固定小数点
 * Hueが1変化するごとにRGBは255/60ずつ変化する。
 * (255/60)*2^3=34
 * 2^3は(255/60)との乗算の結果が「整数」になる最小の数値。
 *
 * 上記の式に演算したい乗数を掛けた後に、8で割る(右3シフト)
 * 今回の場合は(34*Hue)>>3によって小数と整数の疑似的な乗算ができる。
 * enumの定義では(1 << 3)で2＾3を作る
 */

#endif /* INC_WS2812_H_ */

/*
 * rtc.h
 *
 *  Created on: Jan 21, 2026
 *      Author: wataoxp
 */

#ifndef INC_RTC_H_
#define INC_RTC_H_

#include "periph.h"

namespace RealClockSpace{
	typedef enum{
		Success,
		Initialized,
		Error_Initialization,
		Error_NotSynchro,
		Clock_NotSet,
		Alarm_NotModule,
		Retry,
	}ConfigStatus;

	constexpr uint32_t SynchroTime = 1000;
	constexpr uint32_t AsynchDefault = 0x7F;
	constexpr uint32_t SynchDefault = 0xFF;

	enum class AlarmValue :uint32_t{
		WeekDay_Disable,
		WeekDay_Enable,
	};

	enum class Options :uint8_t{
		Alarm_Enable,
		Alarm_Disable,
		ALMA,
		ALMB,
		ALMA_ALMB,
		NoneAlarm,
		WakeUp_Enable,
		WakeUp_Disable,
	};

	typedef struct{
		AlarmValue WeekDaySel;
		uint8_t Day;

		uint32_t Mask;

		uint8_t Hours;
		uint8_t Minutes;
		uint8_t Seconds;
	}AlarmStatus;

	typedef struct{
		Options Alarm;
		Options SelectAlarm;
		Options WakeUp;

		AlarmStatus ALMA;
		AlarmStatus ALMB;

	}AlarmParameters;

	constexpr uint32_t GetAlarmSecondsMask(Options selAlarm)
	{
#ifndef STM32C0
		if(selAlarm == Options::ALMB)
		{

			return (LL_RTC_ALMB_MASK_DATEWEEKDAY | LL_RTC_ALMB_MASK_HOURS | LL_RTC_ALMB_MASK_MINUTES);
		}
#endif
		return (LL_RTC_ALMA_MASK_DATEWEEKDAY | LL_RTC_ALMA_MASK_HOURS | LL_RTC_ALMA_MASK_MINUTES);
	}


//	constexpr uint32_t ALMAMaskIsSeconds = (LL_RTC_ALMA_MASK_DATEWEEKDAY | LL_RTC_ALMA_MASK_HOURS | LL_RTC_ALMA_MASK_MINUTES);
//	constexpr uint32_t ALMBMaskIsSeconds = (LL_RTC_ALMB_MASK_DATEWEEKDAY | LL_RTC_ALMB_MASK_HOURS | LL_RTC_ALMB_MASK_MINUTES);
//	constexpr uint32_t ALMAMaskIsHoursMinutes = (LL_RTC_ALMA_MASK_DATEWEEKDAY);
//	constexpr uint32_t ALMBMaskIsHoursMinutes = (LL_RTC_ALMB_MASK_DATEWEEKDAY);

	typedef struct{
		uint8_t HourTens;		// HT
		uint8_t HourUnits;		// HU
		uint8_t MinuteTens;		// MNT
		uint8_t MinuteUnits;	// MNU
		uint8_t SecondTens;		// SNT
		uint8_t SecondUnits;	// SNU
	}TimeRegBitField;

	typedef struct{
		uint8_t YearTens;	// YT
		uint8_t YearUnits;	// YU
		uint8_t MonthTens;	// MT
		uint8_t MonthUnits;	// MU
		uint8_t DayTens;	// DT
		uint8_t DayUnits;	// DU
		uint8_t WeekDayUnits;	// WDU
	}DateRegBitFiled;
}


class RealClock{
private:
	RTC_TypeDef *RTCx;
	uint32_t CheckInitMode(void);
	void CheckDBP(void);
	uint32_t WaitForSynchro(void);
	void SetALMA(RealClockSpace::AlarmStatus *alma);
	void SetALMB(RealClockSpace::AlarmStatus *almb);
public:
	RealClock(RTC_TypeDef *rtc);
	void ClockConfig(void);
	uint32_t Config(uint32_t HourFormat,uint32_t AsynchPrescaler,uint32_t SynchPrescaler);
	uint32_t SetTime(uint32_t Format,uint32_t Hours,uint32_t Minutes,uint32_t Seconds);
	uint32_t SetDate(uint32_t WeekDay,uint32_t Month,uint32_t Day,uint32_t Year);
	uint32_t SetAlarm(RealClockSpace::AlarmParameters *init);
	uint32_t DisableAlarm(RealClockSpace::Options select);
	void SetWakeUpTimer(uint32_t Count);
	void GetTimeRegister(RealClockSpace::TimeRegBitField *tr);
	void GetDateRegister(RealClockSpace::DateRegBitFiled *dr);
};




#endif /* INC_RTC_H_ */

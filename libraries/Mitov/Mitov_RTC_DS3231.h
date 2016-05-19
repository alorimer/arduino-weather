////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_RTC_DS3231_h
#define _MITOV_RTC_DS3231_h

#include <Mitov.h>
#include <Mitov_Basic_RTC.h>

namespace Mitov
{
//---------------------------------------------------------------------------
//	enum DS3231ClockFrequency { rtc3231fDisabled, rtc3231f1Hz, rtc3231f1024Hz, rtc3231f4096Hz, rtc3231f8192Hz };
//---------------------------------------------------------------------------
	class RTC_DS3231 : public BasicHaltRTC
	{
		typedef BasicHaltRTC inherited;

		enum Register 
		{
			kSecondReg  = 0,
			kMinuteReg  = 1,
			kHourReg    = 2,
			kDayReg     = 3,
			kDateReg    = 4,
			kMonthReg   = 5,
			kYearReg    = 6,
			kControlReg = 0xE
//			kClockReg	= 7,

			// The RAM register space follows the clock register space.
//			kRamAddress0     = 8
		};

		const uint8_t	DS3231_ADDRESS  = 0x68;

	protected:
		Mitov::TDateTime FLastDateTime;

	protected:
		uint8_t readRegister(const uint8_t reg) 
		{
			Wire.beginTransmission(DS3231_ADDRESS);
			Wire.write( reg );	
			Wire.endTransmission();

			Wire.requestFrom(DS3231_ADDRESS, (uint8_t)1 );

			return Wire.read();
		}

		void writeRegister( const uint8_t reg, const uint8_t value ) 
		{
			Wire.beginTransmission(DS3231_ADDRESS);
			Wire.write( reg );	
			Wire.write( value );	
			Wire.endTransmission();
		}

		uint8_t hourFromRegisterValue(const uint8_t value) 
		{
			uint8_t adj;
			if (value & 128)  // 12-hour mode
				adj = 12 * ((value & 32) >> 5);

			else           // 24-hour mode
				adj = 10 * ((value & (32 + 16)) >> 4);

			return (value & 15) + adj;
		}

		virtual void ReadTime()
		{
			if( ! OutputPin.IsConnected() )
				return;

			Wire.beginTransmission(DS3231_ADDRESS);
			Wire.write((byte)0);	
			Wire.endTransmission();

			Wire.requestFrom(DS3231_ADDRESS, (byte)7);
			uint16_t ASecond = FromBcdToDec(Wire.read() & 0x7F);
			uint16_t AMinute = FromBcdToDec(Wire.read());
			uint16_t AHour = hourFromRegisterValue( Wire.read() );
			uint16_t ADay = Wire.read();
			uint16_t ADate = FromBcdToDec(Wire.read());
			uint16_t AMonth = FromBcdToDec(Wire.read());
			uint16_t AYear = FromBcdToDec(Wire.read()) + 2000;

			Mitov::TDateTime ADateTime;

			if( ADateTime.TryEncodeDateTime( AYear, AMonth, ADate, AHour, AMinute, ASecond, 0 ))
			{
				if( FLastDateTime != ADateTime )
				{
					FLastDateTime = ADateTime;
					OutputPin.Notify( &ADateTime );
				}
			}

		}

		virtual void UpdateHalt()
		{
			uint8_t sec = readRegister( kSecondReg );

			sec &= ~(1 << 7);
			sec |= ( ( Halt & 1 ) << 7 );
			writeRegister( kSecondReg, sec );
		}

/*
		void UpdateClockFrequency()
		{
			const uint8_t CValues [] = { 0x00, 0x80, 0x40, 0x41, 0x42, 0x43 };

			uint8_t AValue = readRegister( kControlReg );

			sec &= ~(1 << 7);
			sec |= ( ( Halt & 1 ) << 7 );
			writeRegister( kSecondReg, sec );

			writeRegister( kControlReg, CValues[ ClockFrequency ] );
		}
*/
	protected:
		virtual void DoSetReceive( void *_Data )
		{
			Mitov::TDateTime &ADateTime = *(Mitov::TDateTime *)_Data;

			uint16_t AYear;
			uint16_t AMonth;
			uint16_t ADay;
			uint16_t AWeekDay;
			uint16_t AHour;
			uint16_t AMinute;
			uint16_t ASecond;
			uint16_t AMilliSecond;
			ADateTime.DecodeDateTime( AYear, AMonth, ADay, AWeekDay, AHour, AMinute, ASecond, AMilliSecond );

			Wire.beginTransmission(DS3231_ADDRESS);
			Wire.write((byte)0); // start at location 0

			Wire.write( ( ( Halt & 1 ) << 7 ) | FromDecToBcd( ASecond ));
			Wire.write(FromDecToBcd( AMinute ));
			Wire.write(FromDecToBcd( AHour ));
			Wire.write(FromDecToBcd( AWeekDay ));
			Wire.write(FromDecToBcd( ADay ));
			Wire.write(FromDecToBcd( AMonth ));
			Wire.write(FromDecToBcd( AYear - 2000));

			Wire.endTransmission();
		}

	protected:
		virtual void SystemInit()
		{
			UpdateHalt();
//			UpdateClockFrequency();
			inherited::SystemInit();
		}

	public:
		RTC_DS3231()// :
//			ClockFrequency( rtc3231fDisabled )
		{
		}
	};
}

#endif

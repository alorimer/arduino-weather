////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Derived from https://github.com/msparks/arduino-ds1302

#ifndef _MITOV_RTC_DS1302_h
#define _MITOV_RTC_DS1302_h

#include <Mitov.h>
#include <Mitov_Basic_RTC.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	template<int T_DATA_PIN, int T_CLOCK_PIN, int T_ENABLE_PIN> class RTC_DS1302 : public BasicHaltRTC
	{
		typedef BasicHaltRTC inherited;

		enum Register 
		{
			kSecondReg       = 0,
			kMinuteReg       = 1,
			kHourReg         = 2,
			kDateReg         = 3,
			kMonthReg        = 4,
			kDayReg          = 5,
			kYearReg         = 6,
			kWriteProtectReg = 7,

			// The RAM register space follows the clock register space.
			kRamAddress0     = 32
		};

		enum Command 
		{
			kClockBurstRead  = 0xBF,
			kClockBurstWrite = 0xBE,
			kRamBurstRead    = 0xFF,
			kRamBurstWrite   = 0xFE
		};

		class SPISession
		{
		public:
			SPISession()
			{
				digitalWrite(T_CLOCK_PIN, LOW);
				digitalWrite( T_ENABLE_PIN, HIGH );
				delayMicroseconds(4);  // tCC
			}

			~SPISession() 
			{
				digitalWrite( T_ENABLE_PIN, LOW );
				delayMicroseconds(4);  // tCWH
			}
		};

	public:
		bool	WriteProtect = false;

	protected:
		Mitov::TDateTime FLastDateTime;

	public:
		void SetWriteProtect( bool AValue )
		{
			if( WriteProtect == AValue )
				return;

			WriteProtect = AValue;
			UpdateWriteProtect();
		}

	protected:
		virtual void SystemInit()
		{
			pinMode( T_ENABLE_PIN, OUTPUT );
			pinMode( T_CLOCK_PIN, OUTPUT );
//			pinMode( 10, OUTPUT );

//			Serial.println( "INIT" );
			UpdateHalt();
//			Serial.println( "INIT1" );

			inherited::SystemInit();
		}

	protected:
		void writeOut(const uint8_t value) 
		{
			pinMode( T_DATA_PIN, OUTPUT);
			// This assumes that shiftOut is 'slow' enough for the DS1302 to read the
			// bits. The datasheet specifies that SCLK must be in its high and low states
			// for at least 0.25us at 5V or 1us at 2V. Experimentally, a 16MHz Arduino
			// seems to spend ~4us high and ~12us low when shifting.
			shiftOut( T_DATA_PIN, T_CLOCK_PIN, LSBFIRST, value);
		}

		uint8_t readIn() 
		{
			uint8_t input_value = 0;
			uint8_t bit = 0;
			pinMode( T_DATA_PIN, INPUT);

			// Bits from the DS1302 are output on the falling edge of the clock
			// cycle. This method is called after a previous call to writeOut() or
			// readIn(), which will have already set the clock low.
			for (int i = 0; i < 8; ++i) 
			{
				bit = digitalRead( T_DATA_PIN );
				input_value |= (bit << i);  // Bits are read LSB first.

				// See the note in writeOut() about timing. digitalWrite() is slow enough to
				// not require extra delays for tCH and tCL.
				digitalWrite( T_CLOCK_PIN, HIGH);
				digitalWrite( T_CLOCK_PIN, LOW);
			}

			return input_value;
		}

		virtual void UpdateHalt()
		{
			uint8_t sec = readRegister( kSecondReg );

//			Serial.println( sec );

			sec &= ~(1 << 7);
			sec |= ( ( Halt & 1 ) << 7 );
			writeRegister( kSecondReg, sec );
		}

		void UpdateWriteProtect()
		{
			writeRegister(kWriteProtectReg, ( ( WriteProtect & 1 ) << 7));
		}

		uint8_t readRegister(const uint8_t reg) 
		{
			const SPISession s;

			const uint8_t cmd_byte = (0x81 | (reg << 1));
			writeOut(cmd_byte);
			return readIn();
		}

		void writeRegister( const uint8_t reg, const uint8_t value ) 
		{
			const SPISession s;

			const uint8_t cmd_byte = (0x80 | (reg << 1));
			writeOut(cmd_byte);
			writeOut(value);
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

			const SPISession s;

			writeOut(kClockBurstRead);
			uint16_t ASecond = FromBcdToDec(readIn() & 0x7F);
			uint16_t AMinute = FromBcdToDec(readIn());
			uint16_t AHour = hourFromRegisterValue(readIn());
			uint16_t ADate = FromBcdToDec(readIn());
			uint16_t AMonth = FromBcdToDec(readIn());
			uint16_t ADay = FromBcdToDec(readIn());
			uint16_t AYear = 2000 + FromBcdToDec(readIn());

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

	protected:
		virtual void DoSetReceive( void *_Data )
		{
			if( WriteProtect )
				return;

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

			const SPISession s;

			writeOut(kClockBurstWrite);
			writeOut( ( ( Halt & 1 ) << 7 ) | FromDecToBcd( ASecond ));
			writeOut(FromDecToBcd( AMinute ));
			writeOut(FromDecToBcd( AHour ));
			writeOut(FromDecToBcd( ADay ));
			writeOut(FromDecToBcd( AMonth ));
			writeOut(FromDecToBcd( AWeekDay ));
			writeOut(FromDecToBcd( AYear - 2000));
			// All clock registers *and* the WP register have to be written for the time
			// to be set.
			writeOut(0);  // Write protection register.
		}

	};
}

#endif

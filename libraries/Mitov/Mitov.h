////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_h
#define _MITOV_h

#include <OpenWire.h>

#if defined( VISUINO_TEENSY_3_0 ) || defined( VISUINO_TEENSY_3_1 ) || defined( VISUINO_TEENSY_LC )
  #define VISUINO_TEENSY
#endif

#if defined( VISUINO_ARDUINO_DUE ) || defined( VISUINO_LINKIT_ONE ) || defined( VISUINO_TEENSY ) || defined( VISUINO_TEENSY_2_0 ) || defined( VISUINO_TEENSY_2_0_PP )
  #define NO_SERIAL_FORMAT
#endif

#if defined( VISUINO_ESP8266 )
#define V_FMOD(a,b) (a - b * floor(a / b))
#else
#define V_FMOD(a,b) fmod(a,b)
#endif

#ifndef PWMRANGE
  #define PWMRANGE 255
#endif

#define MAKE_CALLBACK(A) (OpenWire::Component *)this, (OpenWire::TOnPinReceive)( void (OpenWire::Component::*) ( void *_Data ) )&A

namespace Mitov
{
//---------------------------------------------------------------------------
#ifdef VISUINO_ESP8266

template<typename T> const T& MitovMin(const T& a, const T& b)
{
    return (b < a) ? b : a;
}

template<typename T> const T& MitovMax(const T& a, const T& b)
{
    return (b > a) ? b : a;
}

#else
	#define MitovMin min
	#define MitovMax max
#endif
//---------------------------------------------------------------------------
	struct TDataBlock
	{
	public:
		uint32_t	Size;
		byte *Data;

	public:
		TDataBlock( uint32_t ASize, void *AData ) :
			Size( ASize), 
			Data( (byte *)AData )
		{
		}

	};
//---------------------------------------------------------------------------
	const bool GBooleanConst[] = { false, true };
//---------------------------------------------------------------------------
	#pragma pack(push, 1)
	struct TColor
	{
	public:
		uint8_t Red;
		uint8_t Green;
		uint8_t Blue;
		uint8_t Alpha = 0;

	public:
		void SetValue( const long AValue )
		{
			*((long *)this) = AValue & 0xFFFFFF;
//			Blue = ( AValue >> 16 ) & 0xFF;
//			Green = ( AValue >> 8 ) & 0xFF;
//			Red = AValue & 0xFF;
//			Alpha = 0;
		}

		void operator =( const long AValue )
		{
			SetValue( AValue );
		}

		bool operator ==( const TColor other )
		{
			return *((long *)this) == *((long *)&other);
		}

		operator long()
		{
			return *((long *)this);
		}

	public:
		String ToString()
		{
			return	String( "(" ) + 
					String((int)Red ) + "," +
					String((int)Green ) + "," +
					String((int)Blue ) + ")";
		}

	public:
		TColor( long AValue = 0 )
		{
			SetValue( AValue );
		}

		TColor( unsigned char ARed, unsigned char AGreen, unsigned char ABlue ) :
			Red( ARed ),
			Green( AGreen ),
			Blue( ABlue )
		{
		}

		TColor( long AValue, bool ) // For Windows BGR support
		{
			Red = ( AValue >> 16 ) & 0xFF;
			Green = ( AValue >> 8 ) & 0xFF;
			Blue = AValue & 0xFF;
		}
	};
	#pragma pack(pop)
//---------------------------------------------------------------------------
	const double HoursPerDay		= 24;
	const double MinsPerHour		= 60;
	const double SecsPerMin			= 60;
	const double MSecsPerSec		= 1000;
	const double MinsPerDay			= HoursPerDay * MinsPerHour;
	const double SecsPerDay			= MinsPerDay * SecsPerMin;
	const double SecsPerHour		= SecsPerMin * MinsPerHour;
	const double MSecsPerDay		= SecsPerDay * MSecsPerSec;
	const int32_t	IMSecsPerDay	= MSecsPerDay;
// Days between 1/1/0001 and 12/31/1899
//	const double DateDelta			= 693594;
//---------------------------------------------------------------------------
	const uint16_t MonthDays[2][12] =
	{
		{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
		{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
	};
//---------------------------------------------------------------------------
	class TDateTime
	{
	public:
		int32_t Time; // Number of milliseconds since midnight
		int32_t Date; // One plus number of days since 1/1/0001

	protected:
		inline void DivMod( int32_t Dividend, uint16_t Divisor, uint16_t &Result, uint16_t &Remainder )
		{
			Result = Dividend / Divisor;
			Remainder = Dividend % Divisor;
		}

	public:
		void DecodeTime( uint16_t &Hour, uint16_t &Min, uint16_t &Sec, uint16_t &MSec )
		{
			uint16_t	MinCount, MSecCount;

			DivMod( Time, SecsPerMin * MSecsPerSec, MinCount, MSecCount );
			DivMod( MinCount, MinsPerHour, Hour, Min );
			DivMod( MSecCount, MSecsPerSec, Sec, MSec );
		}

		bool IsLeapYear( uint16_t Year )
		{
			return (Year % 4 == 0) && ((Year % 100 != 0) || (Year % 400 == 0));
		}

		bool DecodeDateFully( uint16_t &Year, uint16_t &Month, uint16_t &Day, uint16_t &DOW )
		{
			const uint32_t D1 = 365;
			const uint32_t D4 = D1 * 4 + 1;
			const uint32_t D100 = D4 * 25 - 1;
			const uint32_t D400 = D100 * 4 + 1;

/*
		var
		  Y, M, D, I: Word;
		  T: Integer;
		  DayTable: PDayTable;
		begin
*/
			int32_t T = Date;
			if( T <= 0 )
			{
				Year = 0;
				Month = 0;
				Day = 0;
				DOW = 0;
				return( false );
			}

			else
			{
				DOW = T % 7 + 1;
				--T;
				uint16_t Y = 1;
				while( T >= D400 )
				{
					T -= D400;
					Y += 400;
				}

				uint16_t D, I;
				DivMod( T, D100, I, D );
				if( I == 4 )
				{
					-- I;
					D -= D100;
				}

				Y += I * 100;
				DivMod(D, D4, I, D);

				Y += I * 4;
				DivMod(D, D1, I, D);

				if( I == 4 )
				{
					--I;
					D += D1;
				}

				Y += I;
				bool Result = IsLeapYear(Y);

				const uint16_t *DayTable = MonthDays[Result];
				uint16_t M = 0; // The C++ Day table is zero indexed!
				for(;;)
				{
					I = DayTable[M];
					if( D < I )
						break;

					D -= I;
					++ M;
				}

				Year = Y;
				Month = M + 1;
				Day = D + 1;

                return Result;
			}
		}

		bool TryEncodeDate( uint16_t Year, uint16_t Month, uint16_t Day )
		{
			const uint16_t *DayTable = MonthDays[IsLeapYear(Year)];
			if( (Year >= 1) && (Year <= 9999) && (Month >= 1) && (Month <= 12) &&
				(Day >= 1) && (Day <= DayTable[Month]))
			{
				for( int i = 1; i < Month; i ++ )
					Day += DayTable[ i - 1 ];

				int I = Year - 1;
//				Serial.println( I );
//				Serial.println( Day );
				Date = ((uint32_t)I) * 365 + (int)( I / 4 ) - (int)( I / 100 ) + (int)( I / 400 ) + (uint32_t)Day;
//				Serial.println( Date );
				return true;
			}

			return false;
		}

		bool TryEncodeTime( uint16_t Hour, uint16_t Min, uint16_t Sec, uint16_t MSec )
		{
			if ((Hour < HoursPerDay) && (Min < MinsPerHour) && (Sec < SecsPerMin) && (MSec < MSecsPerSec))
			{
				Time =  (Hour * (MinsPerHour * SecsPerMin * MSecsPerSec))
					  + (Min * SecsPerMin * MSecsPerSec)
					  + (Sec * MSecsPerSec)
					  +  MSec;
//				Date = DateDelta; // This is the "zero" day for a TTimeStamp, days between 1/1/0001 and 12/30/1899 including the latter date
				Date = 0; // This is the "zero" day for a TTimeStamp, days between 1/1/0001 and 12/30/1899 including the latter date
				return true;
			}

			return false;
		}

		bool TryEncodeDateTime( uint16_t AYear, uint16_t AMonth, uint16_t ADay, uint16_t AHour, uint16_t AMinute, uint16_t ASecond, uint16_t AMilliSecond )
		{
			bool Result = TryEncodeDate( AYear, AMonth, ADay );
			if( Result )
			{
				TDateTime LTime;
				Result = LTime.TryEncodeTime( AHour, AMinute, ASecond, AMilliSecond );
				if( Result )
					Time = LTime.Time; 

			}

            return Result;
		}

		void DecodeDateTime( uint16_t &AYear, uint16_t &AMonth, uint16_t &ADay, uint16_t &AHour, uint16_t &AMinute, uint16_t &ASecond, uint16_t &AMilliSecond )
		{
			uint16_t AWeekDay;
			DecodeDateTime( AYear, AMonth, ADay, AWeekDay, AHour, AMinute, ASecond, AMilliSecond );
		}

		void DecodeDateTime( uint16_t &AYear, uint16_t &AMonth, uint16_t &ADay, uint16_t &AWeekDay, uint16_t &AHour, uint16_t &AMinute, uint16_t &ASecond, uint16_t &AMilliSecond )
		{
			DecodeDateFully( AYear, AMonth, ADay, AWeekDay );
			DecodeTime( AHour, AMinute, ASecond, AMilliSecond );
		}

		String ToString()
		{
			uint16_t AYear, AMonth, ADay, AHour, AMinute, ASecond, AMilliSecond;
			DecodeDateTime( AYear, AMonth, ADay, AHour, AMinute, ASecond, AMilliSecond );
			char ABuffer[ 6 + 4 + 5 * 2 + 3 + 2 + 1 ];
			sprintf( ABuffer, "(%04d.%02d.%02d %02d:%02d:%02d.%03d)", AYear, AMonth, ADay, AHour, AMinute, ASecond, AMilliSecond );
			return ABuffer;
		}

	public:
		bool operator == ( TDateTime &AOther )
		{
			if( Date != AOther.Date )
				return false;

			return ( Time == AOther.Time );
		}

		bool operator != ( TDateTime &AOther )
		{
			if( Date != AOther.Date )
				return true;

			return ( Time != AOther.Time );
		}

		bool operator <= ( TDateTime &AOther )
		{
			if( Date > AOther.Date )
				return false;

			if( Date < AOther.Date )
				return true;

			return ( Time <= AOther.Time );
		}

		bool operator >= ( TDateTime &AOther )
		{
			if( Date > AOther.Date )
				return true;

			if( Date < AOther.Date )
				return false;

			return ( Time >= AOther.Time );
		}

		bool operator < ( TDateTime &AOther )
		{
			if( Date > AOther.Date )
				return false;

			if( Date < AOther.Date )
				return true;

			return ( Time < AOther.Time );
		}

		bool operator > ( TDateTime &AOther )
		{
			if( Date > AOther.Date )
				return true;

			if( Date < AOther.Date )
				return false;

			return ( Time > AOther.Time );
		}

	public:
		TDateTime() :
			Date( 693594 ),
			Time( 0 )
		{
		}

		TDateTime( int32_t ADate, int32_t ATime ) :
			Date( ADate ),
			Time( ATime )
		{
		}

		TDateTime( uint32_t ADummy ) :
			Date( 693594 ),
			Time( 0 )
		{
		}

	};
//---------------------------------------------------------------------------
	template <typename T> void swap ( T& a, T& b )
	{
		T c(a); a=b; b=c;
	}
//---------------------------------------------------------------------------
	class Bytes
	{
	public:
		unsigned char *_Bytes;
		unsigned int  _BytesSize = 0;

	};
//---------------------------------------------------------------------------
	class CommonSource : public OpenWire::Component
	{
	public:
		OpenWire::SourcePin	OutputPin;

	};
//---------------------------------------------------------------------------
	class EnabledComponent : public OpenWire::Component
	{
	public:
		bool	Enabled = true;

	};
//---------------------------------------------------------------------------
	class ClockingSupport : public OpenWire::Object
	{
	public:
		OpenWire::ConnectSinkPin	ClockInputPin;

	protected:
		virtual void DoClockReceive( void *_Data ) = 0;

	public:
		ClockingSupport()
		{
			ClockInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ClockingSupport::DoClockReceive );
		}

	};
//---------------------------------------------------------------------------
	class CommonEnableSource : public CommonSource
	{
    public:
        bool Enabled = true;

	};
//---------------------------------------------------------------------------
	class CommonSink : public OpenWire::Component
	{
	public:
		OpenWire::SinkPin	InputPin;

	protected:
		virtual void DoReceive( void *_Data ) = 0;

	public:
		CommonSink()
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&CommonSink::DoReceive );
		}
	};
//---------------------------------------------------------------------------
	class CommonFilter : public CommonSource
	{
	public:
		OpenWire::SinkPin	InputPin;

	protected:
		virtual void DoReceive( void *_Data ) = 0;

	public:
		CommonFilter()
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&CommonFilter::DoReceive );
		}
	};
//---------------------------------------------------------------------------
	class CommonEnableFilter : public CommonFilter
	{
    public:
        bool Enabled = true;

	};
//---------------------------------------------------------------------------
	class CommonImplementedEnableFilter : public CommonEnableFilter
	{
	protected:
		virtual void ReceiveValue( void *_Data ) = 0;

		virtual void DoReceive( void *_Data ) override
        {
            if( ! Enabled )
            {
			    OutputPin.Notify( _Data );
                return;
            }

			ReceiveValue( _Data );
        }

	};
//---------------------------------------------------------------------------
	template<typename T_IN, typename T_OUT> class CommonTypedInOutFilter : public CommonImplementedEnableFilter
    {
	protected:
        virtual T_OUT FilterValue( T_IN AValue ) = 0;

	protected:
		virtual void ReceiveValue( void *_Data ) override
        {
			T_IN AInValue = *(T_IN*)_Data;

            T_OUT AOutValue = FilterValue( AInValue );

			OutputPin.Notify( &AOutValue );
        }

    };
//---------------------------------------------------------------------------
	template<typename T> class CommonTypedFilter : public CommonTypedInOutFilter<T, T>
    {
    };
//---------------------------------------------------------------------------
	class CommonGenerator : public CommonSource
	{
		typedef Mitov::CommonSource inherited;

	protected:
		unsigned long FLastTime = 0;
		unsigned long FPeriod = 0;

	public:
		bool	Enabled = true;
		float	Frequency = 1.0;

	public:
		void SetFrequency( float AValue )
		{
			if( Frequency == AValue )
				return;

			Frequency = AValue;
			CalculateFields();
		}

	protected:
		virtual void Clock() = 0;

	protected:
		virtual void CalculateFields()
		{
			FPeriod = ( (( 1 / Frequency ) * 1000000 ) + 0.5 );
		}

		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			while( currentMicros - FLastTime >= FPeriod )
			{
				if( Enabled )
					Clock();

				FLastTime += FPeriod;
			}

			inherited::SystemLoopBegin( currentMicros );
		}

		virtual void SystemStart() override
		{
			inherited::SystemStart();
			FLastTime = micros();
		}

	};
//---------------------------------------------------------------------------
	template<typename T> class ValueSource : public Mitov::CommonSource
	{
		typedef Mitov::CommonSource inherited;

	public:
		OpenWire::SinkPin	ClockInputPin;

	public:
		T Value;

	protected:
		virtual void SystemStart()
		{
			inherited::SystemStart();
			OutputPin.Notify( &Value );
		}

		void DoClockReceive( void *_Data )
		{
			OutputPin.Notify( &Value );
		}

	public:
		ValueSource( T AValue ) :
			Value( AValue )
		{
			ClockInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ValueSource::DoClockReceive );
		}

	};
//---------------------------------------------------------------------------
	template<typename T> class BindableValueSource : public Mitov::ValueSource<T>
	{
		typedef Mitov::ValueSource<T> inherited;

	protected:
		T OldValue;

	protected:
		virtual void SystemInit()
		{
			inherited::SystemInit();
			OldValue = inherited::Value;
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			if( inherited::Value == OldValue )
				return;

			OldValue = inherited::Value;
			inherited::OutputPin.Notify( &OldValue );
		}

	public:
		BindableValueSource( T AValue ) :
			inherited( AValue ),
			OldValue( AValue )
		{
		}

	};
//---------------------------------------------------------------------------
	template<typename T, typename T_OUT> class BasicMultiInput : public Mitov::CommonSource
	{
		typedef Mitov::CommonSource	inherited;

	protected:
		bool	FModified = false;
		T_OUT	FLastOutput;

	protected:
		void DoReceive( void *_Data )
		{
			FModified = true;
		}

	protected:
		virtual T_OUT CalculateOutput() = 0;

		inline void CallCalculateSendOutput( bool AFromStart ) // Work around compiler bug
		{
			CalculateSendOutput( AFromStart );
		}

		virtual void CalculateSendOutput( bool AFromStart )
		{
//			Serial.println( "CalculateSendOutput+++" );
			T_OUT AValue = CalculateOutput();
//			Serial.println( AValue );
			if( ! AFromStart )
				if( FLastOutput == AValue )
					return;

			OutputPin.SendValue( AValue ); // Use Send Value for String Compatibility!
			FLastOutput = AValue;

			FModified = false;
		}

		virtual void SystemStart()
		{
			CalculateSendOutput( true );
		}

		virtual void SystemLoopEnd()
		{
			if( FModified )
				CalculateSendOutput( false );

		}

	};
//---------------------------------------------------------------------------
	template<typename T, typename T_OUT, int C_NUM_INPUTS> class BasicCommonMultiInput : public Mitov::BasicMultiInput<T, T_OUT>
	{
		typedef Mitov::BasicMultiInput<T, T_OUT>	inherited;

	public:
		OpenWire::VlaueSinkPin<T> InputPins[ C_NUM_INPUTS ];

	protected:
		virtual void SystemInit()
		{
			inherited::SystemInit();

			for( int i = 0; i < C_NUM_INPUTS; ++i )
				InputPins[ i ].SetCallback( this, (OpenWire::TOnPinReceive)&BasicCommonMultiInput::DoReceive );

		}
	};
//---------------------------------------------------------------------------
	template<typename T, int C_NUM_INPUTS> class CommonMultiInput : public Mitov::BasicCommonMultiInput<T,T, C_NUM_INPUTS>
	{
		typedef Mitov::BasicCommonMultiInput<T,T, C_NUM_INPUTS>	inherited;

	};
//---------------------------------------------------------------------------
	template<typename T, int C_NUM_INPUTS> class CommonClockedMultiInput : public Mitov::CommonMultiInput<T, C_NUM_INPUTS>, public ClockingSupport
	{
		typedef Mitov::CommonMultiInput<T, C_NUM_INPUTS>	inherited;

	protected:
		virtual void CalculateSendOutput( bool AFromStart )
		{
			if( ClockInputPin.IsConnected() )
				return;

			inherited::CalculateSendOutput( AFromStart );
/*
			T_OUT AValue = CalculateOutput();
//			Serial.println( AValue );
			if( ! AFromStart )
				if( FLastOutput == AValue )
					return;

			OutputPin.SendValue( AValue ); // Use Send Value for String Compatibility!
			FLastOutput = AValue;

			FModified = false;
*/
		}

	protected:
		virtual T CalculateOutput() = 0;

		void DoClockReceive( void *_Data )
		{
			T AValue = CalculateOutput();
//			if( FLastOutput == AValue )
//				return;

			inherited::OutputPin.SendValue( AValue ); // Use Send Value for String Compatibility!
			inherited::FLastOutput = AValue;

			inherited::FModified = false;
		}

	};
//---------------------------------------------------------------------------
	class DigitalInput : public CommonSource
	{
		typedef Mitov::CommonSource	inherited;

	public:
		uint8_t PinNumber;


	protected:
		virtual void SystemInit()
		{
			inherited::SystemInit();
			pinMode(PinNumber, INPUT);
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			bool AValue = digitalRead( PinNumber );
			OutputPin.Notify( &AValue );
		}

	public:
		DigitalInput( int APinNumber ) :
			PinNumber( APinNumber )
		{
		}
	};
//---------------------------------------------------------------------------
	class DigitalOutput : public CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data )
		{
//			Serial.println( "Test1" );
			if( *(bool*)_Data )
				digitalWrite( PinNumber, HIGH );

			else
				digitalWrite( PinNumber, LOW );

		}

	public:
		uint8_t PinNumber;

	protected:
		virtual void SystemInit()
		{
			inherited::SystemInit();
			pinMode(PinNumber, OUTPUT);
		}

	public:
		DigitalOutput( int APinNumber ) :
			PinNumber( APinNumber )
		{
		}
	};
//---------------------------------------------------------------------------
	class ArduinoBasicAnalogInputChannel : public CommonSource
	{
		typedef Mitov::CommonSource	inherited;

	protected:
		uint8_t	FPinNumber;
		float	FLastOutput;

	public:
		ArduinoBasicAnalogInputChannel( int APinNumber ) :
			FPinNumber( APinNumber )
		{
		}
	};
//---------------------------------------------------------------------------
	class ArduinoAnalogInputChannel : public ArduinoBasicAnalogInputChannel
	{
		typedef Mitov::ArduinoBasicAnalogInputChannel	inherited;

#ifdef VISUINO_TEENSY
	protected:
		unsigned int FResolution = 0;
		float		 FDivider = 1023.0;

	public:
		void SetResolution( unsigned int AResolution )
		{
			if( AResolution == FResolution )
				return;

			FResolution = AResolution;
			analogReadRes( FResolution );
			CalculateMultiplier();
		}

	protected:
		void CalculateMultiplier()
		{
			FDivider = pow( 2, FResolution ) - 1;
		}

#endif
	protected:
		virtual void SystemStart()
		{
#ifdef VISUINO_TEENSY
			analogReadRes( FResolution );
			FLastOutput = analogRead( FPinNumber ) / FDivider;
#else
			FLastOutput = analogRead( FPinNumber ) / 1023.0;
#endif
	    	OutputPin.Notify( &FLastOutput );
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
#ifdef VISUINO_TEENSY
			float AValue = analogRead( FPinNumber ) / FDivider;
#else
			float AValue = analogRead( FPinNumber ) / 1023.0;
#endif
			if( AValue == FLastOutput )
				return;

			FLastOutput = AValue;
			OutputPin.Notify( &AValue );
		}

	public:
		ArduinoAnalogInputChannel( int APinNumber ) :
			inherited( APinNumber )
		{
		}

#ifdef VISUINO_TEENSY
		ArduinoAnalogInputChannel( int APinNumber, unsigned int AResolution ) :
			inherited( APinNumber ),
			FResolution( AResolution )
		{
			CalculateMultiplier();
		}

#endif
	};
//---------------------------------------------------------------------------
#ifdef VISUINO_TEENSY
//---------------------------------------------------------------------------
	class ArduinoTouchAnalogInputChannel : public ArduinoBasicAnalogInputChannel
	{
		typedef Mitov::ArduinoBasicAnalogInputChannel	inherited;

	protected:
		virtual void SystemStart()
		{
			FLastOutput = touchRead( FPinNumber );
	    	OutputPin.Notify( &FLastOutput );
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			float AValue = touchRead( FPinNumber );
			if( AValue == FLastOutput )
				return;

			FLastOutput = AValue;
			OutputPin.Notify( &AValue );
		}

	public:
		ArduinoTouchAnalogInputChannel( int APinNumber ) :
			inherited( APinNumber )
		{
		}
	};
//---------------------------------------------------------------------------
/*
	template <int V_PIN> void SetArduinoAnalogInputResolutionChannel( unsigned int AValue )
	{
		analogReadRes( AValue );
	}
*/
//---------------------------------------------------------------------------
#endif // VISUINO_TEENSY
//---------------------------------------------------------------------------
	class AnalogOutput : public CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data )
		{
			float AValue = *(float*)_Data;
			analogWrite( PinNumber, ( AValue * PWMRANGE ) + 0.5 );
		}

	public:
		uint8_t PinNumber;

	protected:
		virtual void SystemInit()
		{
			inherited::SystemInit();

			pinMode(PinNumber, OUTPUT);
		}

	public:
		AnalogOutput( int APinNumber ) :
			PinNumber( APinNumber )
		{
		}
	};
//---------------------------------------------------------------------------
    class ArduinoDigitalChannel : public CommonSource
    {
		typedef Mitov::CommonSource	inherited;

	protected:
		uint8_t  FPinNumber;
        bool FIsOutput : 1;
        bool FIsPullUp : 1;
        bool FIsAnalog : 1;
		bool FCombinedInOut : 1;
		bool FLastOutput : 1;
		bool FRawInput : 1;

	public:
		OpenWire::SinkPin	DigitalInputPin;
		OpenWire::SinkPin	AnalogInputPin;

	protected:
        void PinDirectionsInit()
        {
			if( FRawInput )
				return;

            if( FIsOutput )
                pinMode( FPinNumber, OUTPUT );

            else
            {
                if( FIsPullUp )
                    pinMode( FPinNumber, INPUT_PULLUP );

                else
                    pinMode( FPinNumber, INPUT );
            }
        }

	protected:
		virtual void SystemInit()
		{
			inherited::SystemInit();

            PinDirectionsInit();
		}

		virtual void SystemStart()
		{
			if( FRawInput )
				return;

			FLastOutput = ( digitalRead( FPinNumber ) == HIGH );
	    	OutputPin.SendValue( FLastOutput );
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
        {
            if( !FIsOutput )
            {
    			bool AValue = ( digitalRead( FPinNumber ) == HIGH );
				if( AValue == FLastOutput )
					return;

				FLastOutput = AValue;
//	Serial.println( AData.Value );
	    		OutputPin.Notify( &AValue );
            }
        }

	protected:
		void DoAnalogReceive( void *_Data )
        {
            if( FIsOutput && FIsAnalog )
            {
                float AValue = *(float*)_Data;
                analogWrite( FPinNumber, ( AValue * PWMRANGE ) + 0.5 );
            }
        }

/*
		void DoDigitalOutputReceive( void *_Data )
		{
			*((int*)_Data) = FPinNumber;
		}
*/
		void DoDigitalReceive( void *_Data )
		{
			if( FRawInput )
			{
				*((int*)_Data) = FPinNumber;
				return;
			}

            if( ( FCombinedInOut || FIsOutput ) && (! FIsAnalog ))
            {
				bool AValue = *(bool *)_Data;

				if( AValue )
				  if( FCombinedInOut )
					  pinMode( FPinNumber, OUTPUT );

                if( AValue )
                    digitalWrite( FPinNumber, HIGH );

                else
                    digitalWrite( FPinNumber, LOW );

				if( ! AValue )
					if( FCombinedInOut )
						if( FIsPullUp )
							pinMode( FPinNumber, INPUT_PULLUP );

						else
							pinMode( FPinNumber, INPUT );

            }
		}

	public:
        void SetIsOutput( bool AValue )
        {
            if( FIsOutput == AValue )
                return;

            FIsOutput = AValue;
            PinDirectionsInit();
        }

        void SetIsPullUp( bool AValue )
        {
            if( FIsPullUp == AValue )
                return;

            FIsPullUp = AValue;
            PinDirectionsInit();
        }

        void SetIsAnalog( bool AValue )
        {
            if( FIsAnalog == AValue )
                return;

            FIsAnalog = AValue;
            PinDirectionsInit();
        }

	public:
		ArduinoDigitalChannel( int APinNumber, bool AIsOutput, bool AIsPullUp, bool AIsAnalog, bool ACombinedInOut, bool ARawInput ) :
			FPinNumber( APinNumber ),
            FIsOutput( AIsOutput ),
            FIsPullUp( AIsPullUp ),
            FIsAnalog( AIsAnalog ),
			FRawInput( ARawInput ),
			FCombinedInOut( ACombinedInOut )

		{
			DigitalInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ArduinoDigitalChannel::DoDigitalReceive );
			AnalogInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ArduinoDigitalChannel::DoAnalogReceive );
		}
    };
//---------------------------------------------------------------------------
    class ArduinoAnalogOutputChannel : public CommonSink
    {
		typedef Mitov::CommonSink	inherited;

	protected:
		uint8_t  FPinNumber;

	protected:
		virtual void DoReceive( void *_Data )
		{
            float AValue = *(float*)_Data;
            analogWrite( FPinNumber, ( AValue * PWMRANGE ) + 0.5 );
		}

	public:
		ArduinoAnalogOutputChannel( int APinNumber ) : 
			FPinNumber( APinNumber )
		{
		}

	};
//---------------------------------------------------------------------------
#ifdef HAVE_CDCSERIAL
	#define SERIAL_TYPE Serial_
#else
	#define SERIAL_TYPE HardwareSerial
#endif
//---------------------------------------------------------------------------
	enum TArduinoSerialParity { spNone, spEven, slOdd, slMark, slSpace };
//---------------------------------------------------------------------------
	class BasicSerialPort : public OpenWire::Component
	{
		typedef OpenWire::Component	inherited;

	public:
		OpenWire::SourcePin	SendingOutputPin;

	public:
		bool	Enabled : 1;

	protected:
		bool	FSending : 1;

	public:
		uint32_t	AfterSendingDelay = 10000;

	public:
		virtual Stream &GetStream() = 0;

	protected:
		uint32_t	FStartTime;

	public:
		template<typename T> void Print( T AValue )
		{
			if( Enabled )
			{
				FSending = true;
				SendingOutputPin.SendValue( FSending );
				FStartTime = micros();
				GetStream().println( AValue );

			}
		}

		template<typename T> void PrintChar( T AValue )
		{
			if( Enabled )
			{
				FSending = true;
				SendingOutputPin.SendValue( FSending );
				FStartTime = micros();
				GetStream().print( AValue );
			}
		}

	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			if( FSending )
			{
				if( currentMicros - FStartTime >= AfterSendingDelay )
				{
					FSending = false;
					SendingOutputPin.SendValue( FSending );
				}
			}

			inherited::SystemLoopBegin( currentMicros );
		}

	public:
		BasicSerialPort() :
			Enabled( true ),
			FSending( false )
		{
		}
	};
//---------------------------------------------------------------------------
	template<typename T_SERIAL_TYPE, T_SERIAL_TYPE *T_SERIAL> class BasicTypedSerialPort : public Mitov::BasicSerialPort
	{
		typedef Mitov::BasicSerialPort	inherited;

	public:
		virtual Stream &GetStream() { return *T_SERIAL; }

	protected:
		virtual void SystemInit()
		{
			if( Enabled )
				StartPort();

			inherited::SystemInit();
		}

	protected:
		virtual void StartPort() = 0;

        void RestartPort()
		{
			if( ! Enabled )
				return;

			T_SERIAL->end();
			StartPort();
		}

	public:
		void SetEnabled( bool AValue )
		{
            if( Enabled == AValue )
                return;

            Enabled = AValue;
			if( Enabled )
				StartPort();

			else
				T_SERIAL->end();
		}

	};
//---------------------------------------------------------------------------
	template<typename T_SERIAL_TYPE, T_SERIAL_TYPE *T_SERIAL> class SpeedSerialPort : public Mitov::BasicTypedSerialPort<T_SERIAL_TYPE, T_SERIAL>
	{
		typedef Mitov::BasicTypedSerialPort<T_SERIAL_TYPE, T_SERIAL>	inherited;

	public:
		unsigned long	Speed = 9600;

	public:
		void SetSpeed( unsigned int AValue )
		{
            if( Speed == AValue )
                return;

            Speed = AValue;
            inherited::RestartPort();
		}

	protected:
		virtual void StartPort()
		{
			T_SERIAL->begin( Speed );
		}

	};
//---------------------------------------------------------------------------
	template<typename T_INSTANCE, T_INSTANCE *T_SERIAL_INSTANCE, typename T> class ArduinoSerialInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			T_SERIAL_INSTANCE->Print( *(T*)_Data );
		}
	};
//---------------------------------------------------------------------------
	template<typename T_INSTANCE, T_INSTANCE *T_SERIAL_INSTANCE> class ArduinoSerialInput<T_INSTANCE, T_SERIAL_INSTANCE, bool> : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			T_SERIAL_INSTANCE->Print( *(bool*)_Data ? "true" : "false" );
		}
	};
//---------------------------------------------------------------------------
	template<typename T_INSTANCE, T_INSTANCE *T_SERIAL_INSTANCE> class ArduinoSerialInput<T_INSTANCE, T_SERIAL_INSTANCE, char> : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			if( *(char*)_Data == '\r' )
				T_SERIAL_INSTANCE->Print( "" );

			else
				T_SERIAL_INSTANCE->PrintChar( *(char*)_Data );
		}
	};
//---------------------------------------------------------------------------
	template<typename T_INSTANCE, T_INSTANCE *T_SERIAL_INSTANCE> class ArduinoSerialBinaryInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			if( T_SERIAL_INSTANCE->Enabled )
			{
				Mitov::TDataBlock ADataBlock = *(Mitov::TDataBlock*)_Data;
				T_SERIAL_INSTANCE->GetStream().write((uint8_t *) ADataBlock.Data, ADataBlock.Size );
//				T_SERIAL_INSTANCE->GetStream().write( *(unsigned char*)_Data );
			}
		}
	};
//---------------------------------------------------------------------------
	template<typename T_INSTANCE, T_INSTANCE *T_SERIAL_INSTANCE> class ArduinoSerialStringInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			T_SERIAL_INSTANCE->Print( (char*)_Data );
		}
	};
//---------------------------------------------------------------------------
	template<typename T_INSTANCE, T_INSTANCE *T_SERIAL_INSTANCE, typename T_OBJECT> class ArduinoSerialObjectInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			T_SERIAL_INSTANCE->Print( ((T_OBJECT *)_Data)->ToString().c_str() );
		}
	};
//---------------------------------------------------------------------------
	template<typename T_INSTANCE, T_INSTANCE *T_SERIAL_INSTANCE> class ArduinoDirectSerialInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	public:
		virtual Stream &GetStream()
		{
			return T_SERIAL_INSTANCE->GetStream();
		}

	protected:
		virtual void DoReceive( void *_Data ) override
		{
//			if( T_SERIAL_INSTANCE->Enabled )
//				T_SERIAL->write( *(unsigned char*)_Data );
		}
	};
//---------------------------------------------------------------------------
	template<typename T_INSTANCE, T_INSTANCE *T_SERIAL_INSTANCE> class ArduinoSerialOutput : public Mitov::CommonSource
	{
		typedef Mitov::CommonSource	inherited;

//	protected:
//		byte FData[ 2 ];

	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			if( ! T_SERIAL_INSTANCE->Enabled )
				return;

//			if( Serial.available() )
			{
				int AData = T_SERIAL_INSTANCE->GetStream().read();
				if( AData >= 0 )
				{
//					FData[ 0 ] = AData;
//					OutputPin.Notify( FData );

					unsigned char AByte = AData;
					OutputPin.SendValue( Mitov::TDataBlock( 1, &AByte ));
				}
			}

			inherited::SystemLoopBegin( currentMicros );
		}

/*
	public:
		ArduinoSerialOutput()
		{
			FData[ 1 ] = 0;
		}
*/
	};
//---------------------------------------------------------------------------
	class ArduinoI2C_Command
	{
	public:
		virtual void Execute() = 0;

	};
//---------------------------------------------------------------------------
	class ArduinoI2CInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			((ArduinoI2C_Command*)_Data )->Execute();
//			Wire.println( *(ArduinoI2C_Command*)_Data );
		}
	};
//---------------------------------------------------------------------------
	class GPRSAccessPoint
	{
	public:
		bool	Enabled = true;

		String	AccessPoint;
		String	UserName;
		String	Password;

		int32_t	NumberRetries = 5;
		int32_t	RetryInterval = 30000;

	public:
		int32_t	FRetryCount = 0;
		unsigned long FLastTime = 0;

	public:
		bool CanRetry( unsigned long currentMicros )
		{
			if( FRetryCount > NumberRetries )
				return false;

			if(( currentMicros - FLastTime ) < RetryInterval )
				return false;

			return true;
		}

	};
//---------------------------------------------------------------------------
	class CommonConverter : public OpenWire::SinkPin
	{
	public:
		OpenWire::SourcePin	OutputPin;

	};
//---------------------------------------------------------------------------
	class Convert_BinaryBlockToChar : public Mitov::CommonConverter
	{
	protected:
		virtual void DoReceive( void *_Data )
		{
			Mitov::TDataBlock ABlock = *(Mitov::TDataBlock *)_Data;
			while( ABlock.Size-- )
			  OutputPin.Notify( ABlock.Data++ );
		}

	};
//---------------------------------------------------------------------------
	class Convert_BinaryToClock : public Mitov::CommonConverter
	{
	protected:
		bool	FLastValue = false;

	protected:
		virtual void DoReceive( void *_Data )
		{
			bool AValue = *(bool *)_Data;
			if( AValue == FLastValue )
				return;

			FLastValue = AValue;
			if( AValue )
				OutputPin.Notify( NULL );

		}

	};
//---------------------------------------------------------------------------
	inline uint8_t FromBcdToDec(const uint8_t bcd) 
	{
		return (10 * ((bcd & 0xF0) >> 4) + (bcd & 0x0F));
	}
//---------------------------------------------------------------------------
	inline uint8_t FromDecToBcd(const uint8_t dec) 
	{
		const uint8_t tens = dec / 10;
		const uint8_t ones = dec % 10;
		return (tens << 4) | ones;
	}
//---------------------------------------------------------------------------
	inline float ConvertCtoF(float c) 
	{
		return c * 1.8 + 32;
	}
//---------------------------------------------------------------------------
	inline float ConvertFtoC(float f)
	{
		return (f - 32) * 0.55555;
	}
//---------------------------------------------------------------------------
	class InterruptLock 
	{
	public:
		InterruptLock()
		{
			noInterrupts();
		}

		~InterruptLock() 
		{
			interrupts();
		}
	};
//---------------------------------------------------------------------------
} // Mitov

#endif


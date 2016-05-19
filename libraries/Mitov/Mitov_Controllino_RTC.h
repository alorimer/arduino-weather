////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_CONTROLLINO_RTC_h
#define _MITOV_CONTROLLINO_RTC_h

#include <Mitov.h>
#include <Controllino.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class ControllinoRTCModule : public OpenWire::Component, public Mitov::ClockingSupport
	{
		typedef OpenWire::Component inherited;

	public:
		OpenWire::SourcePin	OutputPin;
		OpenWire::SinkPin	SetInputPin;

	public:
		bool	Enabled = true;

	public:
		void SetEnabled( bool AValue )
		{
            if( Enabled == AValue )
                return;

            Enabled = AValue;
			if( Enabled )
				StartModule();

			else
				StopModule();

		}

	protected:
		virtual void SystemInit()
		{
			if( Enabled )
				StartModule();

			inherited::SystemInit();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			if( ! ClockInputPin.IsConnected() )
				DoClockReceive( NULL );

			inherited::SystemLoopBegin( currentMicros );
		}

		void StopModule()
		{
		}

		void StartModule()
		{
//			Serial.println( "START" );
//			SPI.begin();
//			Controllino_RTC_init( 65 );
			Controllino_RTC_init( 0 );
//			Controllino_RTCSSInit();
//			Controllino_SetRTCSS( true );
		}

		virtual void DoClockReceive( void *_Data ) override
		{
			if( ! Enabled )
				return;

//			Serial.println( "Test1" );
			unsigned char aDay, aWeekDay, aMonth, aYear, aHour, aMinute, aSecond;
//			Controllino_PrintTimeAndDate();

			Controllino_ReadTimeDate( &aDay, &aWeekDay, &aMonth, &aYear, &aHour, &aMinute, &aSecond );
//			Serial.println( "Test2" );
			uint16_t ALongYear = 2000 + aYear;
//			Serial.println( ALongYear );
//			Serial.println( aMonth );
//			Serial.println( aDay );
			TDateTime ADateTime;
//			ADateTime.TryEncodeDate( 2000 + aYear, aMonth, aDay );
//			ADateTime.TryEncodeTime( aHour, aMinute, aSecond, 0 );
			ADateTime.TryEncodeDateTime( ALongYear, aMonth, aDay, aHour, aMinute, aSecond, 0 );
			OutputPin.Notify( &ADateTime );
		}

		void DoSetTimeReceive( void *_Data )
		{
			if( ! Enabled )
				return;

			uint16_t AYear, AMonth, ADay, aWeekDay, AHour, AMinute, ASecond, AMilliSecond;

//			Serial.println( "Test2" );
			((TDateTime *)_Data)->DecodeDateTime( AYear, AMonth, ADay, aWeekDay, AHour, AMinute, ASecond, AMilliSecond );
			Controllino_SetTimeDate( ADay, aWeekDay, AMonth, AYear, AHour, AMinute, ASecond );

		}

	public:
		ControllinoRTCModule()
		{			
			SetInputPin.SetCallback( MAKE_CALLBACK( ControllinoRTCModule::DoSetTimeReceive ));
		}

	};
//---------------------------------------------------------------------------
}

#endif

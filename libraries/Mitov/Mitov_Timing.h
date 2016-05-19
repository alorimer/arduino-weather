////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_TIMING_h
#define _MITOV_TIMING_h

#include <Mitov.h>

namespace Mitov
{
	class ClockGenerator : public Mitov::CommonGenerator
	{
	protected:
		virtual void Clock()
		{
			OutputPin.Notify( NULL );
		}

	};
//---------------------------------------------------------------------------
	class Timer : public Mitov::CommonEnableSource
	{
		typedef Mitov::CommonEnableSource inherited;

	public:
		OpenWire::SinkPin	StartInputPin;
		OpenWire::SinkPin	ResetInputPin;

	public:
		uint32_t	Interval = 1000000;
		bool		AutoRepeat : 1;
		bool		CanRestart : 1;

	protected:
		bool		FValue : 1;

		uint32_t	FStartTime;

	protected:
		virtual void SystemStart()
		{
			inherited::SystemStart();
			OutputPin.SendValue( FValue );
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			if( ! AutoRepeat )
				if( ! FValue )
				{
					inherited::SystemLoopBegin( currentMicros );
					return;
				}

			if( FValue )
				if( ! Enabled )
				{
					FValue = false;
					OutputPin.SendValue( FValue );
					inherited::SystemLoopBegin( currentMicros );
					return;
				}

			currentMicros = micros(); // Use micros(); to make sure the DoReceiveStart is not from the same clock event!
			if( currentMicros - FStartTime >= Interval )
			{
				FValue = !FValue;
				OutputPin.SendValue( FValue );
				if( AutoRepeat )
					FStartTime += Interval;
			}

			inherited::SystemLoopBegin( currentMicros );
		}

	protected:
		virtual void DoReceiveStart( void *_Data )
		{
			if( ! Enabled )
				return;

			if( !CanRestart )
				if( FValue )
					return;

			FStartTime = micros();
			if( FValue )
				return;

			FValue = true;
			OutputPin.SendValue( FValue );
		}

		void DoReceiveClear( void *_Data )
		{
			if( FValue )
			{
				FValue = false;
				OutputPin.SendValue( FValue );

				if( AutoRepeat )
					FStartTime = micros();
			}
		}

	public:
		Timer() :
			AutoRepeat( false ),
			CanRestart( true ),
			FValue( false )
		{
			StartInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&Timer::DoReceiveStart );
			ResetInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&Timer::DoReceiveClear );
		}
	};
//---------------------------------------------------------------------------
	class Delay : public Mitov::CommonEnableSource
	{
		typedef Mitov::CommonEnableSource inherited;

	public:
		OpenWire::SinkPin	StartInputPin;
		OpenWire::SinkPin	ResetInputPin;

	public:
		unsigned long	Interval = 1000000;
		bool			CanRestart : 1;

	protected:
		bool			FValue : 1;
		unsigned long	FStartTime;

	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			if( FValue )
			{
				if( ! Enabled )
					FValue = false;

				else
				{
					unsigned long ACurrentMicros = micros(); // Use micros(); to make sure the DoReceiveStart is not from the same clock event!
					if( ACurrentMicros - FStartTime >= Interval )
					{
						FValue = false;
						OutputPin.Notify( NULL );
					}
				}
			}

			inherited::SystemLoopBegin( currentMicros );
		}

	protected:
		virtual void DoReceiveStart( void *_Data )
		{
			if( ! Enabled )
				return;

			if( !CanRestart )
				if( FValue )
					return;

			FStartTime = micros();
			if( FValue )
				return;

			FValue = true;
		}

		void DoReceiveClear( void *_Data )
		{
			FValue = false;
		}

	public:
		Delay() :
			CanRestart( true ),
			FValue( false )
		{
			StartInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&Delay::DoReceiveStart );
			ResetInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&Delay::DoReceiveClear );
		}
	};
//---------------------------------------------------------------------------
	class DetectEdge : public Mitov::CommonEnableFilter
	{
	public:
		bool	Rising : 1;

	protected:
		bool	FLastValue : 1;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			bool AValue = *(bool *)_Data;
			if( AValue == FLastValue )
				return;

			FLastValue = AValue;
			if( Rising == AValue )
				OutputPin.Notify( NULL );

		}

	public:
		DetectEdge() : 
			FLastValue( false ),
			Rising( true )
		{
		}

	};

//---------------------------------------------------------------------------
}

#endif

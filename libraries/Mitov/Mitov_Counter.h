////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_COUNTER_h
#define _MITOV_COUNTER_h

#include <Mitov.h>

namespace Mitov
{
#define Min Min
#define Max Max
	class CounterLimit
	{
	public:
		long Value;
		bool RollOver = true;

	public:
		CounterLimit( long AValue ) :
			Value( AValue )
		{
		}
	};
//---------------------------------------------------------------------------
	class BasicCounter : public Mitov::CommonEnableSource
	{
		typedef Mitov::CommonEnableSource inherited;

	public:
		OpenWire::SinkPin	ResetInputPin;

	public:
		long	InitialValue = 0;

	public:
		CounterLimit	Min = -2147483648;
		CounterLimit	Max = 2147483647;

	protected:
		long	FCount = 0;

	protected:
		void DoReceiveReset( void *_Data )
		{
			FCount = InitialValue;
			OutputPin.Notify( &FCount );
		}

		virtual void SystemInit()
		{
//			Serial.println( "Counter::SystemInit" );
			FCount = InitialValue;

			inherited::SystemInit();
			OutputPin.Notify( &FCount );
		}

		virtual void SystemStart()
		{
			inherited::SystemStart();
			if( FCount != InitialValue )
				OutputPin.Notify( &FCount );

		}

	protected:
		inline void	CountUp()
		{
			if( Max.RollOver || ( FCount < Max.Value ))
			{
				++FCount;

				if( FCount > Max.Value )
					FCount = Min.Value;

				OutputPin.Notify( &FCount );
			}
		}

		inline void	CountDown()
		{
			if( Min.RollOver || ( FCount > Min.Value ))
			{
				--FCount;
				if( FCount < Min.Value )
					FCount = Max.Value;

				OutputPin.Notify( &FCount );
			}
		}

	public:
		BasicCounter()			
		{
			ResetInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&BasicCounter::DoReceiveReset );
		}

	};
//---------------------------------------------------------------------------
	class Counter : public Mitov::BasicCounter
	{
		typedef Mitov::BasicCounter inherited;

	public:
		OpenWire::SinkPin	InputPin;

	public:
		bool	Reversed = false;

	public:
		virtual void DoReceive( void *_Data )
		{
            if( ! Enabled )
				return;

			if( Reversed )
				CountDown();

			else
				CountUp();

		}

	public:
		Counter()
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&Counter::DoReceive );
		}

	};
//---------------------------------------------------------------------------
	class UpDownCounter : public Mitov::BasicCounter
	{
		typedef Mitov::BasicCounter inherited;

	public:
		OpenWire::SinkPin	UpInputPin;
		OpenWire::SinkPin	DownInputPin;

	protected:
		void DoReceiveUp( void *_Data )
		{
            if( Enabled )
				CountUp();
		}

		void DoReceiveDown( void *_Data )
		{
            if( Enabled )
				CountDown();
		}

	public:
		UpDownCounter()
		{
			UpInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&UpDownCounter::DoReceiveUp );
			DownInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&UpDownCounter::DoReceiveDown );
		}

	};
//---------------------------------------------------------------------------
#undef Min
#undef Max
}

#endif

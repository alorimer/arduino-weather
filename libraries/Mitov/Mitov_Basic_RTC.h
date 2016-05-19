////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_BASIC_RTC_h
#define _MITOV_BASIC_RTC_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class BasicRTC : public Mitov::CommonSource, public Mitov::ClockingSupport
	{
		typedef Mitov::CommonSource inherited;

	public:
		OpenWire::SinkPin	SetInputPin;

	protected:
		virtual void DoSetReceive( void *_Data ) = 0;
		virtual void ReadTime() = 0;

		virtual void DoClockReceive( void *_Data ) override
		{
			ReadTime();
		}

	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			if( ! ClockInputPin.IsConnected() )
				ReadTime();

			inherited::SystemLoopBegin( currentMicros );
		}

	public:
		BasicRTC()
		{
			SetInputPin.SetCallback( MAKE_CALLBACK( BasicRTC::DoSetReceive ));
		}

	};
//---------------------------------------------------------------------------
	class BasicHaltRTC : public BasicRTC
	{
		typedef BasicRTC inherited;

	public:
		bool	Halt = false;

	public:
		void SetHalt( bool AValue )
		{
			if( Halt == AValue )
				return;

			Halt = AValue;
			UpdateHalt();
		}

	protected:
		virtual void UpdateHalt() = 0;

	};
//---------------------------------------------------------------------------
}

#endif

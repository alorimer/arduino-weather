////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_CONTROLLINO_RS485_h
#define _MITOV_CONTROLLINO_RS485_h

#include <Mitov.h>
#include <Controllino.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class ControllinoRS485Module : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		OpenWire::SinkPin	TransmitInputPin;
		OpenWire::SinkPin	ReceiveInputPin;

	public:
		bool	FTransmit = false;
		bool	FReceive = false;

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

		void StopModule()
		{
		}

		void StartModule()
		{
			Controllino_RS485Init();
			Controllino_SwitchRS485DE( FTransmit );
			Controllino_SwitchRS485RE( FReceive );
		}

	protected:
		void DoTransmitInputPinReceive( void *_Data )
		{
			FTransmit = *(bool *)_Data;
			if( ! Enabled )
				return;

			Controllino_SwitchRS485DE( FTransmit );
		}

		void DoReceiveInputPinReceive( void *_Data )
		{
			FReceive = *(bool *)_Data;
			if( ! Enabled )
				return;

			Controllino_SwitchRS485RE( FReceive );
		}

	public:
		ControllinoRS485Module()
		{
			TransmitInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ControllinoRS485Module::DoTransmitInputPinReceive );
			ReceiveInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ControllinoRS485Module::DoReceiveInputPinReceive );
		}

	};
}

#endif

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_COMMON_MUX_h
#define _MITOV_COMMON_MUX_h

#include <Mitov.h>

namespace Mitov
{
	template<typename T, typename T_OUT> class ToggleSwitch : public Mitov::BasicMultiInput<T, T_OUT>
	{
		typedef Mitov::BasicMultiInput<T, T_OUT> inherited;

	public:
		OpenWire::VlaueSinkPin<T>	TrueInputPin;
		OpenWire::VlaueSinkPin<T>	FalseInputPin;

		OpenWire::SinkPin	SelectInputPin;

	public:
		bool	InitialSelectValue = false;

	protected:
		bool	FSelectValue = false;

	protected:
		virtual void SystemStart()
		{
			FSelectValue = InitialSelectValue;
			inherited::SystemStart();
		}

	protected:
		virtual void DoReceiveSelect( void *_Data )
		{
			bool AValue = *(bool *)_Data;
			if( FSelectValue == AValue )
				return;

			FSelectValue = AValue;
			inherited::CallCalculateSendOutput( false );
		}

	protected:
		virtual T_OUT CalculateOutput() override
		{
			if( FSelectValue )
				return (T_OUT)TrueInputPin.Value;

			else
				return (T_OUT)FalseInputPin.Value;
		}

	public:
		ToggleSwitch()
		{
			SelectInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ToggleSwitch::DoReceiveSelect );
			TrueInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ToggleSwitch::DoReceive );
			FalseInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ToggleSwitch::DoReceive );
		}

	};
//---------------------------------------------------------------------------
	template<typename T, int C_NUM_INPUTS> class CommonMux : public Mitov::CommonMultiInput<T, C_NUM_INPUTS>
	{
		typedef Mitov::CommonMultiInput<T, C_NUM_INPUTS> inherited;

	protected:
		unsigned int	FChannel = 0;

	public:
		unsigned int	InitialChannel = 0;

	public:
		OpenWire::SinkPin	SelectInputPin;

	protected:
		virtual T CalculateOutput() override
		{
			return inherited::InputPins[ FChannel ].Value;
		}

	protected:
		virtual void DoReceiveSelect( void *_Data )
		{
			uint32_t AChannel = *(uint32_t *)_Data;
			if( AChannel >= C_NUM_INPUTS )
				AChannel = C_NUM_INPUTS - 1;

			if( FChannel == AChannel )
				return;

//			Serial.println( AChannel );

			FChannel = AChannel;
			inherited::CallCalculateSendOutput( false );
		}

	protected:
		virtual void SystemStart()
		{
			FChannel = InitialChannel;
			inherited::SystemStart();
		}

	public:
		CommonMux()
		{
			SelectInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&CommonMux::DoReceiveSelect );
		}

	};
//---------------------------------------------------------------------------
}

#endif

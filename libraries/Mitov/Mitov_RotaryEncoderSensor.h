////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_ROTARY_ENCODER_SENSOR_h
#define _MITOV_ROTARY_ENCODER_SENSOR_h

#include <Mitov.h>

namespace Mitov
{
	class BasicRotaryEncoderSensor : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		OpenWire::SinkPin	AInputPin;
		OpenWire::SinkPin	BInputPin;

		OpenWire::SourcePin	UpOutputPin;
		OpenWire::SourcePin	DownOutputPin;

	public:
		bool	Enabled = true;

	protected:
		bool	FAValue = false;
		bool	FBValue = false;
		bool    FAPopulated = false;
		bool    FBPopulated = false;

	protected:
		void DoValueReceiveA( bool AValue )
		{
			FAPopulated = true;
			if( FAValue == AValue )
				return;

			FAValue = AValue;
			if( Enabled && AValue && FBPopulated )
			{
				if( FBValue )
					DownOutputPin.Notify( NULL );

				else
					UpOutputPin.Notify( NULL );
			}
		}

		void DoValueReceiveB( bool AValue )
		{
			FBPopulated = true;
			if( FBValue == AValue )
				return;

			FBValue = AValue;
		}

	protected:
		virtual void DoReceiveA( void *_Data ) = 0;
		virtual void DoReceiveB( void *_Data ) = 0;

	public:
		BasicRotaryEncoderSensor()
		{
			AInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&BasicRotaryEncoderSensor::DoReceiveA );
			BInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&BasicRotaryEncoderSensor::DoReceiveB );
		}

	};
//---------------------------------------------------------------------------
	class RotaryEncoderSensor : public BasicRotaryEncoderSensor
	{
		typedef BasicRotaryEncoderSensor inherited;

	protected:
		virtual void DoReceiveA( void *_Data ) override
		{
			DoValueReceiveA( *(bool*)_Data );
		}

		virtual void DoReceiveB( void *_Data ) override
		{
			DoValueReceiveB( *(bool*)_Data );
		}

	};
//---------------------------------------------------------------------------
	class RotaryEncoderSensor_Debounce : public BasicRotaryEncoderSensor
	{
		typedef BasicRotaryEncoderSensor inherited;

	public:
		uint32_t	DebounceInterval = 0;

	protected:
		bool	FALastValue : 1;
		bool	FBLastValue : 1;

		unsigned long	FALastTime = 0;
		unsigned long	FBLastTime = 0;

	protected:
		virtual void DoReceiveA( void *_Data )
		{
			bool AValue = *( bool *)_Data;
			if( AValue != FALastValue )
				FALastTime = millis();

			FALastValue = AValue;
		}

		virtual void DoReceiveB( void *_Data )
		{
			bool AValue = *( bool *)_Data;
			if( AValue != FBLastValue )
				FBLastTime = millis();

			FBLastValue = AValue;
		}

	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			if( FAValue != FALastValue )
				if( millis() - FALastTime > DebounceInterval )
					DoValueReceiveA( FALastValue );

			if( FBValue != FBLastValue )
				if( millis() - FBLastTime > DebounceInterval )
					DoValueReceiveB( FBLastValue );

			inherited::SystemLoopBegin( currentMicros );
		}

	public:
		RotaryEncoderSensor_Debounce() :
			FALastValue( false ),
			FBLastValue( false )
		{
		}
	};
//---------------------------------------------------------------------------
}

#endif

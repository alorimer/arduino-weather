////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_HX711_WEIGHT_h
#define _MITOV_HX711_WEIGHT_h

#include <Mitov.h>

namespace Mitov
{
	template<int T_CLOCK_PIN, int T_DATA_PIN> class HX711_Weight : public Mitov::CommonSource, public Mitov::ClockingSupport
	{
		typedef Mitov::CommonSource inherited;

	public:
		OpenWire::SinkPin	ZeroInputPin;
		OpenWire::SinkPin	CalibrateInputPin;

	public:
		float	ScaleDivider = 1.0f;
		float	Offset = 0.0f;

		bool	UseChannelB : 1;
		bool	Gain128 : 1;
		bool	Powered : 1;

		int32_t	Average = 1;

	public:
		void SetScaleDivider( float AValue )
		{
			if( ScaleDivider == AValue )
				return;

			ScaleDivider = AValue;
			SendOutput();
		}

		void SetOffset( float AValue )
		{
			if( Offset == AValue )
				return;

			Offset = AValue;
			SendOutput();
		}

		void SetUseChannelB( bool AValue )
		{
			if( UseChannelB == AValue )
				return;

			UseChannelB = AValue;
			UpdateGain();
		}

		void SetGain128( bool AValue )
		{
			if( Gain128 == AValue )
				return;

			Gain128 = AValue;
			UpdateGain();
		}

		void SetAverage( int32_t AValue )
		{
			if( AValue < 1 )
				Average = 1;

			else
				Average = AValue;

		}

		void SetPowered( bool AValue )
		{
			if( Powered == AValue )
				return;

			Powered = AValue;
			UpdatePower();
		}

	protected:
		bool	FNeedsRead = false;
		float	FCurrentValue = 0.0f;

	protected:
		virtual void SystemInit() override
		{
			pinMode( T_CLOCK_PIN, OUTPUT );
			UpdatePower();
			inherited::SystemInit();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			if( FNeedsRead || ( ! ClockInputPin.IsConnected() ))
				ReadSensor();

			inherited::SystemLoopBegin( currentMicros );
		}

	protected:
		void ReadSensor()
		{
			if( OutputPin.IsConnected() )
			{
				if( Powered ) 
				{
//					if( SensorDataInputPin.Value )
					if( digitalRead( T_DATA_PIN ) == HIGH )
					{
						FNeedsRead = true;
						return;
					}

					uint32_t AAccumulator = 0;
					for( int i = 0; i < Average; ++i )
						AAccumulator += ReadSensorOnes();

					FCurrentValue = AAccumulator / Average;
					FNeedsRead = false;
				}

				SendOutput();
			}
		}

		uint32_t ReadSensorOnes()
		{
			while( digitalRead( T_DATA_PIN ) == HIGH )
				;

			uint32_t AValue = 0;
			for( int i = 0; i < 24; ++i )
			{
				AValue <<= 1;
				digitalWrite( T_CLOCK_PIN, HIGH );
//				SensorClockOutputPin.SendValue( true );

				if( digitalRead( T_DATA_PIN ) == HIGH )
					AValue |= 1;

				digitalWrite( T_CLOCK_PIN, LOW );
//				SensorClockOutputPin.SendValue( false );
			}

//			Serial.println( AValue );
			int ACount;
			if( UseChannelB )
				ACount = 2;

			else if( Gain128 )
				ACount = 1;

			else
				ACount = 3;

			// set the channel and the gain factor for the next reading using the clock pin
			for (int i = 0; i < ACount; i++) 
			{
				digitalWrite( T_CLOCK_PIN, HIGH );
				digitalWrite( T_CLOCK_PIN, LOW );
//				SensorClockOutputPin.SendValue( true );
//				SensorClockOutputPin.SendValue( false );
			}				
				
			return AValue;
		}

		void SendOutput()
		{
			float AValue = ( FCurrentValue + Offset ) / ScaleDivider;
			OutputPin.Notify( &AValue );
		}

		void UpdateGain()
		{
			if( ! Powered )
				return;

			digitalWrite( T_CLOCK_PIN, LOW );
//			SensorClockOutputPin.SendValue( false );
			ReadSensorOnes();
		}

		void UpdatePower()
		{
			digitalWrite( T_CLOCK_PIN, LOW );
//			SensorClockOutputPin.SendValue( false );
			if( Powered )
				UpdateGain();

			else
				digitalWrite( T_CLOCK_PIN, HIGH );
//				SensorClockOutputPin.SendValue( true );
		}

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			ReadSensor();
		}

		void DoZeroReceive( void *_Data )
		{
//			Serial.print( "TEST---------------: " );
			Offset = -FCurrentValue;
//			Serial.println( Offset );
		}

		void DoCalibrateReceive( void *_Data )
		{
			ScaleDivider = ( FCurrentValue + Offset );
			if( ! ScaleDivider )
				ScaleDivider = 1.0;
		}

	public:
		HX711_Weight() :
			UseChannelB( false ),
			Gain128( false ),
			FNeedsRead( false ),
			Powered( true )
		{
			ZeroInputPin.SetCallback( MAKE_CALLBACK( HX711_Weight::DoZeroReceive ));
			CalibrateInputPin.SetCallback( MAKE_CALLBACK( HX711_Weight::DoCalibrateReceive ));
		}

	};
}

#endif

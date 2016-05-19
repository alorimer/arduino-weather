////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_MAXIM_ONEWIRE_THERMOMETER_h
#define _MITOV_MAXIM_ONEWIRE_THERMOMETER_h

#include <Mitov.h>

namespace Mitov
{
	class OneWireThermometer : public Mitov::CommonSource, public Mitov::ClockingSupport
	{
		typedef Mitov::CommonSource inherited;

	public:
		OpenWire::SinkPin	OneWireInputPin;
		OpenWire::SourcePin	OneWireOutputPin;

		OpenWire::SourcePin	AddressOutputPin;

		OpenWire::SourcePin	AlarmOutputPin;

	public:
		bool	AutoDiscover : 1;
		bool	InFahrenheit : 1;
		uint8_t Resolution = 9;

		float   AlarmHighTemperature = 100.0f;
		float   AlarmLowTemperature = 0.0f;

	public:
		void SetResolution( uint8_t AValue )
		{
			if( Resolution == AValue )
				return;

			Resolution = AValue;
			if( FThermometer )
				FThermometer->setResolution( FAddress, Resolution );
		}

	protected:
//		bool	FIsChained;
		int		FDigitalPin;
		int     FIndex;
//		float   FCurrentValue;

	protected:
		OneWire				*FOneWire = nullptr;
		// Migrate to direct OpenWire with DallasTemperature rewrite!
		DallasTemperature	*FThermometer = nullptr;
		DeviceAddress		FAddress;

	protected:
		virtual void SystemInit()
		{
			if( FIndex )
			{
//				OneWireOutputPin.Notify( &FOneWire );
				// Migrate to OpenWire with DallasTemperature rewrite!
				OneWireOutputPin.Notify( &FThermometer );
			}

			else
			{
				GetOneWire();

				FThermometer = new DallasTemperature( FOneWire );
				FThermometer->begin();
			}

			if( AutoDiscover )
				FThermometer->getAddress( FAddress, FIndex );

			FThermometer->setResolution( FAddress, Resolution );


/*
			if( FIndex )
			{
				Serial.println( FThermometer->getDeviceCount() );
				Serial.println( FIndex );
				for (uint8_t i = 0; i < 8; i++)
					Serial.println( FAddress[ i ] );
			}
*/
			inherited::SystemInit();
		}

		virtual void SystemStart()
		{
			if( FIndex == 0 )
			{
				FThermometer->setCheckForConversion( true );
				FThermometer->requestTemperatures();
				FThermometer->setCheckForConversion( false );
			}

			if( AddressOutputPin.IsConnected() )
			{
/*
				String AAddress = String( FAddress[ 0 ], HEX );
				for( int i = 1; i < 8; ++i )
					AAddress += '-' + String( FAddress[ i ], HEX );

				AddressOutputPin.Notify( (void *)AAddress.c_str() );
*/
				char format[ 24 ];
				sprintf( format, "%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X", FAddress[ 0 ], FAddress[ 1 ], FAddress[ 2 ], FAddress[ 3 ], FAddress[ 4 ], FAddress[ 5 ], FAddress[ 6 ], FAddress[ 7 ] );
				AddressOutputPin.Notify( format );
			}

			if( AlarmOutputPin.IsConnected() )
			{
				FThermometer->setHighAlarmTemp( FAddress, (char)( AlarmHighTemperature + 0.5 ));
				FThermometer->setLowAlarmTemp( FAddress, (char)( AlarmLowTemperature + 0.5 ));

//				float AValue = ( AlarmHighTemperature - (-55) ) / ( 125 - -55 ) * 255;
//				FThermometer->setHighAlarmTemp( FAddress, (char)( AValue + 0.5 ));

//				AValue = ( AlarmLowTemperature - (-55) ) / ( 125 - -55 ) * 255;
//				FThermometer->setLowAlarmTemp( FAddress, (char)( AValue + 0.5 ));
			}

			inherited::SystemStart();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			if( ! ClockInputPin.IsConnected() )
				ReadTemperature();

			// Needs to be changed to handle when the clock is different for the thermometers!
			if( FIndex == 0 )
				FThermometer->requestTemperatures();

			inherited::SystemLoopBegin( currentMicros );
		}

	protected:
		void GetOneWire()
		{
			if( ! FOneWire )
				FOneWire = new OneWire( FDigitalPin );
		}

		void ReadTemperature()
		{
			if( AlarmOutputPin.IsConnected() )
			{
				bool AValue = FThermometer->hasAlarm( FAddress );
				AlarmOutputPin.Notify( &AValue );
			}

			float   AValue = (InFahrenheit) 
				? FThermometer->getTempF( FAddress )
				: FThermometer->getTempC( FAddress );

			OutputPin.Notify( &AValue );
		}

	protected:
		void DoOneWireReceive( void *_Data )
		{
			if( FIndex )
				OneWireOutputPin.Notify( _Data );

			else
			{
				GetOneWire();
//				*(OneWire**)_Data = FOneWire;
				// Migrate to OpenWire with DallasTemperature rewrite!
				*(DallasTemperature**)_Data = FThermometer;
			}
		}

		virtual void DoClockReceive( void *_Data ) override
		{
			ReadTemperature();
		}

	public:
		OneWireThermometer( int ADigitalPin, int AIndex ) :
			FDigitalPin( ADigitalPin ),
			FIndex( AIndex ),
			AutoDiscover( true ),
			InFahrenheit( false )
		{
			memset( FAddress, 0, 8 );

			OneWireInputPin.SetCallback( MAKE_CALLBACK( OneWireThermometer::DoOneWireReceive ));
		}

		OneWireThermometer( int ADigitalPin, int AIndex, uint8_t AByte0, uint8_t AByte1, uint8_t AByte2, uint8_t AByte3, uint8_t AByte4, uint8_t AByte5, uint8_t AByte6, uint8_t AByte7 ) :
			FDigitalPin( ADigitalPin ),
			FIndex( AIndex ),
			AutoDiscover( true ),
			InFahrenheit( false )
		{
			FAddress[ 0 ] = AByte0;
			FAddress[ 1 ] = AByte1;
			FAddress[ 2 ] = AByte2;
			FAddress[ 3 ] = AByte3;
			FAddress[ 4 ] = AByte4;
			FAddress[ 5 ] = AByte5;
			FAddress[ 6 ] = AByte6;
			FAddress[ 7 ] = AByte7;

			OneWireInputPin.SetCallback( MAKE_CALLBACK( OneWireThermometer::DoOneWireReceive ));
		}

/*
		virtual ~OneWireThermometer()
		{
			if( FIndex == 0 )
			{
				if( FThermometer )
					delete FThermometer;

				if( FOneWire )
					delete FOneWire;
			}
		}
*/
	};
}

#endif

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_SOFTWARE_SERIAL_h
#define _MITOV_SOFTWARE_SERIAL_h

#include <Mitov.h>
#include <SoftwareSerial.h>

namespace Mitov
{
	template<int RX_PIN_NUMBER, int TX_PIN_NUMBER> class VisuinoSoftwareSerial : public Mitov::BasicSerialPort
	{
		typedef Mitov::BasicSerialPort inherited;

	public:
		OpenWire::SourcePin	OutputPin;

	public:
		unsigned	Speed = 9600;

	protected:
		SoftwareSerial *FSerial = nullptr;

	public:
		void SetEnabled( bool AValue )
		{
			if( Enabled == AValue )
				return;

			Enabled = AValue;
			UpdateSerial();
		}

	public:
		virtual Stream &GetStream() { return *FSerial; }

	protected:
		void UpdateSerial()
		{
			if( Enabled )
			{
				if( ! FSerial )
				{
					FSerial = new SoftwareSerial( RX_PIN_NUMBER, TX_PIN_NUMBER );
					FSerial->begin( Speed );
				}
			}
			else
			{
				if( FSerial )
				{
					delete FSerial;
					FSerial = nullptr;
				}
			}
		}

	protected:
		virtual void SystemInit() 
		{
			UpdateSerial();

			inherited::SystemInit();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			if( Enabled )
			{
				int AData = FSerial->read();
				if( AData >= 0 )
				{
					unsigned char AByte = AData;
					OutputPin.SendValue( Mitov::TDataBlock( 1, &AByte ));
//					OutputPin.Notify( &AByte );
				}
			}

			inherited::SystemLoopBegin( currentMicros );
		}

/*
		virtual ~VisuinoSoftwareSerial()
		{
			if( FSerial )
				delete FSerial;
		}
*/
	};
}

#endif

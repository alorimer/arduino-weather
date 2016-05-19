////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_WII_CONTROLLER_h
#define _MITOV_WII_CONTROLLER_h

#include <Mitov.h>

// http://wiibrew.org/wiki/Wiimote/Extension_Controllers/Nunchuck

namespace Mitov
{
//---------------------------------------------------------------------------
	class WiiNunchuck : public Mitov::EnabledComponent, public Mitov::ClockingSupport
	{
		typedef Mitov::EnabledComponent inherited;

	private:
		const byte WII_NUNCHUK_I2C_ADDRESS = 0x52;

	public:
		OpenWire::TypedSourcePin<float>	AngleOutputPins[ 3 ];
		OpenWire::TypedSourcePin<float>	StickOutputPins[ 2 ];
		OpenWire::TypedSourcePin<bool>	ButtonOutputPins[ 2 ];

	public:
		uint32_t	ReadDelay = 100;

	protected:
		bool	FStarted = false;
		unsigned long	FLastTime = 0;
		int		FClockedCount = 0;

	protected:
		void DoClockReceive( void *_Data ) override
		{
			++ FClockedCount;
			StartRead();
		}

		void ReadData( bool AChangeOnly )
		{
			if( ! Enabled )
				return;

			ReadController( AChangeOnly );
		}

		void StartRead()
		{
			Wire.beginTransmission(WII_NUNCHUK_I2C_ADDRESS);
			Wire.write(0x00);
			Wire.endTransmission();

			delayMicroseconds( 10 );

			Wire.beginTransmission(WII_NUNCHUK_I2C_ADDRESS);
			Wire.write(0x00);
			Wire.endTransmission();
			FLastTime = micros();
		}

		void ReadController( bool AChangeOnly )
		{
			uint8_t AData[6];
			Wire.requestFrom(WII_NUNCHUK_I2C_ADDRESS, (byte)6);

			for( int i = 0; i < 6; ++i )
				AData[ i ] = Wire.read();

			float AStickX = (float)AData[0] / 255;
			float AStickY = (float)AData[1] / 255;
			StickOutputPins[ 0 ].SetValue( AStickX, AChangeOnly );
			StickOutputPins[ 1 ].SetValue( AStickY, AChangeOnly );

			float AAngleX = ((float)(((uint16_t)AData[2] ) << 2) + ((AData[5] >> 2) & 0x03)) / 1023;
			float AAngleY = ((float)(((uint16_t)AData[3] ) << 2) + ((AData[5] >> 4) & 0x03)) / 1023;
			float AAngleZ = ((float)(((uint16_t)AData[4] ) << 2) + ((AData[5] >> 6) & 0x03)) / 1023;

			AngleOutputPins[ 0 ].SetValue( AAngleX, AChangeOnly );
			AngleOutputPins[ 1 ].SetValue( AAngleY, AChangeOnly );
			AngleOutputPins[ 2 ].SetValue( AAngleZ, AChangeOnly );

			bool AButtonValue = !( AData[5] & 0b10 );
			ButtonOutputPins[ 0 ].SetValue( AButtonValue, AChangeOnly );

			AButtonValue = !( AData[5] & 1 );
			ButtonOutputPins[ 1 ].SetValue( AButtonValue, AChangeOnly );

			if( ( ! ClockInputPin.IsConnected() ) || FClockedCount )
				StartRead();
		}

	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			if( ! FClockedCount )
				if( ! ClockInputPin.IsConnected() )
					++ FClockedCount;

			if( FClockedCount )
				if( currentMicros - FLastTime > ReadDelay )
				{
					-- FClockedCount;
					ReadData( FStarted );
					FStarted = true;
				}

			inherited::SystemLoopBegin( currentMicros );
		}

		virtual void SystemStart() override
		{
			// Init the controller
			Wire.beginTransmission(WII_NUNCHUK_I2C_ADDRESS);
			Wire.write(0xF0);
			Wire.write(0x55);
			Wire.endTransmission();

			Wire.beginTransmission(WII_NUNCHUK_I2C_ADDRESS);
			Wire.write(0xFB);
			Wire.write(0x00);
			Wire.endTransmission();

			StartRead();
			inherited::SystemStart();
		}

	};
//---------------------------------------------------------------------------
}

#endif

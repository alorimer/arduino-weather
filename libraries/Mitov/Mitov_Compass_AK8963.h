////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_AK8963_h
#define _MITOV_COMPASS_AK8963_h

#include <Mitov.h>
#include <Wire.h> //I2C Arduino Library
#include <Mitov_Basic_I2C.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	enum CompassAK8963Mode { cmSingleMeasurement, cm8Hz, cm100Hz, cmExternalTrigger };
//---------------------------------------------------------------------------
	namespace CompassAK8963Const
	{
		const byte CompassModes[] =
		{
			0b00000001,
			0b00000010,
			0b00000110,
			0b00000100
		};
	}
//---------------------------------------------------------------------------
	class CompassAK8963 : public Mitov::EnabledComponent, public Mitov::ClockingSupport
	{
		typedef	Mitov::EnabledComponent	inherited;

	protected:
		static const byte MPU9150_RA_MAG_ST1		= 0x02;
		static const byte MPU9150_RA_MAG_XOUT_L		= 0x03;
		static const byte MPU9150_RA_MAG_XOUT_H		= 0x04;
		static const byte MPU9150_RA_MAG_YOUT_L		= 0x05;
		static const byte MPU9150_RA_MAG_YOUT_H		= 0x06;
		static const byte MPU9150_RA_MAG_ZOUT_L		= 0x07;
		static const byte MPU9150_RA_MAG_ZOUT_H		= 0x08;
		static const byte MPU9150_RA_MAG_ST2		= 0x09;
		static const byte MPU9150_RA_MAG_CNTL1		= 0x0A;
		static const byte MPU9150_RA_MAG_CNTL2		= 0x0B;

		static const byte MPU9150_RA_MAG_ASAX		= 0x10;
		static const byte MPU9150_RA_MAG_ASAY		= 0x11;
		static const byte MPU9150_RA_MAG_ASAZ		= 0x12;

	public:
		OpenWire::SourcePin	OutputPins[ 3 ];

		OpenWire::SinkPin	ResetInputPin;
		OpenWire::SourcePin OverflowOutputPin;

	public:
		CompassAK8963Mode	Mode : 2;
		bool Enabled : 1;
		bool SelfTest : 1; // Added to save space as bitfield
		byte Address : 2;
		bool HighResolution : 1;

	protected:
		float CompassAdjustmentValues[ 3 ];

	protected:
		void PerformRead()
		{
			const float CompassCoefficients[] =
			{
				10.0f *4219.0f / 8190.0f,
				10.0f *4219.0f / 32760.0f
			};

			uint8_t AIntValues[ 7 ];
			if( ReadBytes( MPU9150_RA_MAG_XOUT_L, 7, AIntValues ))
				for( int i = 0; i < 3; ++i )
				{
					float AValue = (( ((int16_t)AIntValues[ i * 2 + 1 ] ) << 8 ) | AIntValues[ i * 2 ] ) * CompassCoefficients[ HighResolution & 1 ] * CompassAdjustmentValues[ i ];
					OutputPins[ i ].Notify( &AValue );
				}

		}

		void ReadSensors()
		{
			uint8_t AIntValue;

			if( Mode == cmSingleMeasurement )
			{
				UpdateCompassControlReg();
				delay(10);
				PerformRead();
			}

			if( ReadBytes( MPU9150_RA_MAG_ST1, 1, &AIntValue ))
			{
				OverflowOutputPin.SendValue( AIntValue & 0b00000010 );
				if( AIntValue & 0b00000001 )
					PerformRead();
			}

		}

		void UpdateCompassControlReg()
		{
			byte AValue;
			if( Enabled )
				AValue = CompassAK8963Const::CompassModes[ Mode ];

			else
				AValue = 0;

			AValue |=	( HighResolution	? 0b00010000 : 0 );

			WriteByte( MPU9150_RA_MAG_CNTL1, AValue );
		}

		void ReagCompassAdjustmentValues()
		{
			uint8_t AValues[ 3 ];

			ReadBytes( MPU9150_RA_MAG_ASAX, sizeof( AValues ), AValues );
			for( int i = 0; i < 3; ++i )
				CompassAdjustmentValues[ i ] = (((float) AValues[ i ] ) - 128.0f) / 256.0f + 1.0f;

		}

	protected:
		inline void WriteByte( byte ARegister, byte AData )
		{
			I2C::WriteByte( Address + 0x0C, ARegister, AData );
		}

		bool ReadBytes( uint8_t regAddr, uint8_t length, void *data )
		{
			return I2C::ReadBytes( Address + 0x0C, regAddr, length, data );
		}

	protected:
		virtual void SystemStart() override
		{
			UpdateCompassControlReg();
			ReagCompassAdjustmentValues();

			inherited::SystemStart();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{

			if( Enabled )
				if( ! ClockInputPin.IsConnected() )
					ReadSensors();

			inherited::SystemLoopBegin( currentMicros );
		}

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			ReadSensors();
		}

		void DoResetReceive( void *_Data )
		{
			WriteByte( MPU9150_RA_MAG_CNTL2, 0b00000001 );
		}

	public:
		CompassAK8963() :
			Mode( cm100Hz ),
			Enabled( true ),
			SelfTest( false ),
			HighResolution( true ),
			Address( 0 )
		{
			ResetInputPin.SetCallback( MAKE_CALLBACK( CompassAK8963::DoResetReceive ));
		}
	};
//---------------------------------------------------------------------------
	class CompassAK8963I2C : public Mitov::CompassAK8963
	{
		typedef	Mitov::CompassAK8963	inherited;
	};
//---------------------------------------------------------------------------
}

#endif

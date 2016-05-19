////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_HMC5883L_h
#define _MITOV_HMC5883L_h

#include <Mitov.h>
#include <Wire.h> //I2C Arduino Library

namespace Mitov
{

	const byte HMC5883L_Address = 0x1E;
	const byte HMC5883L_StatusRegister = 9;
	const byte HMC5883L_ModeRegister = 2;
	const byte HMC5883_REGISTER_MAG_CRA_REG_M = 0x00;
	const byte HMC5883_REGISTER_MAG_CRB_REG_M = 0x01;
	const byte HMC5883_REGISTER_MAG_OUT_X_H_M = 0x03;
//---------------------------------------------------------------------------
	enum HMC5883L_AveragedSamples { cas1, cas2, cas4, cas8 };
	enum HMC5883L_Bias { cbNone, cbPositive, cbNegative };
	enum HMC5883L_Gain { cg1370, cg1090, cg820, cg660, cg440, cg390, cg330, cg230 };
//---------------------------------------------------------------------------
	class HMC5883L_Compass : public Mitov::EnabledComponent, public Mitov::ClockingSupport
	{
		typedef	Mitov::EnabledComponent	inherited;

	public:
		OpenWire::TypedSourcePin<float>	XOutputPin;
		OpenWire::TypedSourcePin<float>	YOutputPin;
		OpenWire::TypedSourcePin<float>	ZOutputPin;

	public:
		HMC5883L_AveragedSamples	AveragedSamples : 2;
		HMC5883L_Bias				Bias : 2;
		HMC5883L_Gain				Gain : 3;
		bool						Idle : 1;

	protected:
		bool	FClocked : 1;
		bool	FChangeOnly : 1;

	public:
		void SetIdle( bool AValue )
		{
			if( Idle == AValue )
				return;

			Idle = AValue;
			SetIntIdle();
		}

	protected:
		float	FScale;

	protected:
		void SetIntIdle()
		{
			if( Idle )
				WriteTo( HMC5883L_Address, 0b11 ); // Idle

			else
				WriteTo( HMC5883L_Address, 0b00 ); // Single measurement mode				
//				WriteTo( HMC5883L_Address, 0b01 ); // Single measurement mode				
		}

		bool DataAvailable()
		{
			StartReadFrom( HMC5883L_StatusRegister, 1 );

			uint8_t	AStatus = Wire.read();
			return( AStatus & 1 );
		}

		void ReadCompass()
		{
			if( ! Enabled )
				return;

			if( ! DataAvailable() )
				return;

			FClocked = false;

			StartReadFrom( HMC5883_REGISTER_MAG_OUT_X_H_M, 6 );

			union T16BitData
			{
				uint8_t	Bytes[ 2 ];
				int16_t	Value;
			};

			T16BitData A16BitValue;
			float AValue;

			A16BitValue.Bytes[ 1 ] = Wire.read();
			A16BitValue.Bytes[ 0 ] = Wire.read();

/*
			float _magData_x = ((int16_t)A16BitValue.Bytes[ 0 ] ) | (((int16_t)A16BitValue.Bytes[ 1 ] ) << 8);

			if( A16BitValue.Value != _magData_x )
			{
				Serial.print( A16BitValue.Value ); Serial.print( " " ); Serial.println( _magData_x );
			}
*/
//			A16BitValue.Bytes[ 0 ] = 0;
//			A16BitValue.Bytes[ 1 ] = 0;

//			Serial.print( "Scale: " ); Serial.println( FScale );
//			Serial.print( "Value: " ); Serial.print( A16BitValue.Bytes[ 0 ] ); Serial.print( " " ); Serial.println( A16BitValue.Bytes[ 1 ] );
//			Serial.print( "Value: " ); Serial.println( A16BitValue.Value );
			AValue = A16BitValue.Value * FScale;

			XOutputPin.SetValue( AValue, FChangeOnly );

			A16BitValue.Bytes[ 1 ] = Wire.read();
			A16BitValue.Bytes[ 0 ] = Wire.read();

			AValue = A16BitValue.Value * FScale;

			ZOutputPin.SetValue( AValue, FChangeOnly );

			A16BitValue.Bytes[ 1 ] = Wire.read();
			A16BitValue.Bytes[ 0 ] = Wire.read();

			AValue = A16BitValue.Value * FScale;

			YOutputPin.SetValue( AValue, FChangeOnly );

			FChangeOnly = true;
		}

		virtual void DoClockReceive( void *_Data ) override
		{
			FClocked = true;
			ReadCompass();
		}

	protected:
		void WriteTo( byte ARegister, byte AData )
		{
//			Serial.print( "WriteTo :" ); Serial.print( ARegister ); Serial.print( " " ); Serial.println( AData );
			Wire.beginTransmission( HMC5883L_Address );
			Wire.write( ARegister );
			Wire.write( AData );
			Wire.endTransmission();
		}

		void StartReadFrom( byte ARegister, byte ALength )
		{
//			Serial.print( "StartReadFrom :" ); Serial.print( ARegister ); Serial.print( " " ); Serial.println( ALength );
			Wire.beginTransmission( HMC5883L_Address );
			Wire.write( ARegister );
			Wire.endTransmission();

			Wire.requestFrom( HMC5883L_Address, ALength );
			while (Wire.available() < ALength);
		}

		virtual void SystemStart() override
		{
			uint8_t	AValue;

			WriteTo( HMC5883L_ModeRegister, 0 );

			AValue = ( Bias & 0b11 ) + (( AveragedSamples & 0b11 ) << 5 );
			WriteTo( HMC5883_REGISTER_MAG_CRA_REG_M, AValue );

			AValue = ( Gain & 0b111 ) << 5;
			WriteTo( HMC5883_REGISTER_MAG_CRB_REG_M, AValue );
//			Serial.println( AValue );
//			delay( 10000 );

			switch( Gain )
			{
				case cg1370: FScale = 0.73; break;
				case cg1090: FScale = 0.92; break;
				case cg820: FScale = 1.22; break;
				case cg660: FScale = 1.52; break;
				case cg440: FScale = 2.27; break;
				case cg390: FScale = 2.56; break;
				case cg330: FScale = 3.03; break;
				case cg230: FScale = 4.35; break;
			}

			SetIntIdle();
			inherited::SystemStart();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			if( Enabled )
				if( FClocked || ( ! ClockInputPin.IsConnected() ))
					ReadCompass();

			inherited::SystemLoopBegin( currentMicros );
		}

	public:
		HMC5883L_Compass() :
			FClocked( false ),
			FChangeOnly( false ),
			AveragedSamples( cas1 ),
			Bias( cbNone ),
			Gain( cg1090 ),
			Idle( false )
		{
		}
	};
}

#endif

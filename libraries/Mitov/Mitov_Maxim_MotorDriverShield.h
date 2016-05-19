////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_MAXIM_MOTOR_DRIVER_SHIELD_h
#define _MITOV_MAXIM_MOTOR_DRIVER_SHIELD_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	enum TArduinoMaximMotorCurrentRegulationMode { crmRipple25, crmFastDecay, crmSlowDecay };
//---------------------------------------------------------------------------
	class Maxim_MotorDriverBasicChannel;
//---------------------------------------------------------------------------
	class Maxim_MotorDriverBasicShield : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		Mitov::SimpleList<Maxim_MotorDriverBasicChannel *>	FChannels;
		byte FChanged = 0xFF;
		bool FModified = true;

	protected:
		byte F_MAX7300;
		byte F_MAX5387_1;
		byte F_MAX5387_2;

	protected:
		void DoPotentiometerTransmission( byte Address, byte ARegister, byte AData )
		{
			byte AAddress = 0b0101000 | (( Address & 0b111 ) << 1 );

//			Serial.print( AAddress );
//			Serial.print( " " );
//			Serial.print( ARegister );
//			Serial.print( " " );
//			Serial.println( AData );

			Wire.beginTransmission( AAddress );
			Wire.write( ARegister );
			Wire.write( AData );
			Wire.endTransmission();
		}

		void DoGPIOTransmissionAt( byte ARegister, byte AData )
		{
			byte AAddress = 0b01000000 | ( F_MAX7300 & 0b1111 );

			Wire.beginTransmission( AAddress );
			Wire.write( ARegister );
			Wire.write( AData );
			Wire.endTransmission();
		}

		byte DoGPIOReadAt( byte ARegister )
		{
			byte AAddress = 0b01000000 | ( F_MAX7300 & 0b1111 );

			Wire.beginTransmission( AAddress );
			Wire.write( ARegister );
			Wire.endTransmission();
			Wire.requestFrom( AAddress, (byte)1 );
			return Wire.read();
		}

	protected:
		void UpdateSettings();

		virtual void SystemStart();

		virtual void SystemLoopUpdateHardware() override
		{
			if( FModified )
				UpdateSettings();

			inherited::SystemLoopUpdateHardware();
		}

		void UpdateFailures();

	public:
		Maxim_MotorDriverBasicShield( byte A_MAX7300, byte A_MAX5387_1, byte A_MAX5387_2 ) :
			F_MAX7300( A_MAX7300 ),
			F_MAX5387_1( A_MAX5387_1 ),
			F_MAX5387_2( A_MAX5387_2 )
		{
		}

	};
//---------------------------------------------------------------------------
	class Maxim_MotorDriverShieldComponent : public Maxim_MotorDriverBasicShield
	{
		typedef Maxim_MotorDriverBasicShield inherited;

	public:
		OpenWire::SinkPin	FaultInputPin;

	protected:
		bool	FFaultReceived = false;

	protected:
		void DoFaultReceive( void * _Data )
		{
			if( *(bool *)_Data )
				UpdateFailures();
		}

	public:
		Maxim_MotorDriverShieldComponent( byte A_MAX7300, byte A_MAX5387_1, byte A_MAX5387_2 ) :
			inherited( A_MAX7300, A_MAX5387_1, A_MAX5387_2 )
		{
			FaultInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&Maxim_MotorDriverShieldComponent::DoFaultReceive );
		}
	};
//---------------------------------------------------------------------------
	class Maxim_MotorDriverShield : public Maxim_MotorDriverBasicShield
	{
		typedef Maxim_MotorDriverBasicShield inherited;

	protected:
		int FFailureChangePin;

	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			inherited::SystemLoopBegin( currentMicros );
			if( digitalRead( FFailureChangePin ) == HIGH )
				UpdateFailures();
		}

	public:
		Maxim_MotorDriverShield( byte A_MAX7300, byte A_MAX5387_1, byte A_MAX5387_2, int AFailureChangePin ) :
			inherited( A_MAX7300, A_MAX5387_1, A_MAX5387_2 ),
			FFailureChangePin( AFailureChangePin )
		{
		}
	};
//---------------------------------------------------------------------------
	class Maxim_MotorDriverBasicChannel : public OpenWire::Object
	{
	public:
		OpenWire::SinkPin	SpeedInputPin;
		OpenWire::SourcePin	FaultOutputPin;

	public:
		bool Enabled = true;
		float MaxCurrent = 0.0f;
		TArduinoMaximMotorCurrentRegulationMode	CurrentRegulationMode = crmRipple25;
		bool FFailure = false;

	public:
        void SetEnabled( bool AValue )
		{
            if( Enabled == AValue )
                return;

            Enabled = AValue;
			FOwner.FChanged |= 0b00010000 << FIndex; 
			FOwner.FModified = true;
		}

        void SetMaxCurrent( float AValue )
        {
			AValue = constrain( AValue, 0, 2.6 );
            if( MaxCurrent == AValue )
                return;

            MaxCurrent = AValue;
			FOwner.FChanged |= 0b1 << FIndex; 
			FOwner.FModified = true;
        }

        void SetCurrentRegulationMode( TArduinoMaximMotorCurrentRegulationMode AValue )
        {
            if( CurrentRegulationMode == AValue )
                return;

            CurrentRegulationMode = AValue;
			FOwner.FChanged |= 0b00010000 << FIndex; 
			FOwner.FModified = true;
        }

	protected:
		Maxim_MotorDriverBasicShield &FOwner;
		int FIndex;

		float FSpeed;
		float FSenseResistor;

	protected:
		void DoSpeedReceive( void * _Data )
		{
			float AValue = constrain( *(float *)_Data, 0, 1.0 );
//			Serial.println( AValue );
			if( FSpeed == AValue )
				return;

			if(( AValue > 0.5 && FSpeed <= 0.5 ) || ( AValue <= 0.5 && FSpeed > 0.5 ))
				FOwner.FChanged |= 0b00010000 << FIndex; 

			FSpeed = AValue;

			FOwner.FModified = true;

//			FOwner.SetChannelValue( FIndex, AValue );
		}

	public:
		byte	GetRegisterPins()
		{
			byte	Result;
			if( Enabled )
				Result = 0;

			else
				Result = 1;

			if( FSpeed > 0.5 )
				Result |= 0b10;

			switch( CurrentRegulationMode )
			{
				case crmRipple25:
					Result |= 0b1000;

				case crmFastDecay:
					Result |= 0b1100;

				case crmSlowDecay:
					Result |= 0b0100;

			}

			return Result;
		}

		byte GetPotentiometer()
		{
//			byte ATest1 = Reference * 255 + 0.5;
//			Serial.println( ATest1 );
			return ( MaxCurrent * FSenseResistor / 2.6 ) * 255 + 0.5;
//			return Reference * 255 + 0.5;
//			return abs( FSpeed - 0.5 ) * 2 * 255 + 0.5;
		}

		virtual void UpdateSpeed() = 0;

		void UpdateFailure( bool AValue )
		{
			if( FFailure == AValue )
				return;

			FFailure = AValue;
			FaultOutputPin.Notify( &FFailure );
		}

		void InitFailure()
		{
			FaultOutputPin.Notify( &FFailure );
		}

	public:
		Maxim_MotorDriverBasicChannel( Maxim_MotorDriverBasicShield &AOwner, int AIndex, float AInitialSpeed, float ASenseResistor ) :
			FOwner( AOwner ),
			FIndex( AIndex ),
			FSpeed( AInitialSpeed ),
			FSenseResistor( ASenseResistor * 10 )
		{
//			Serial.println( "---" );
			FOwner.FChannels.push_back( this );

//			Serial.println( "---" );
			SpeedInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&Maxim_MotorDriverBasicChannel::DoSpeedReceive );
		}

	};
//---------------------------------------------------------------------------
	class Maxim_MotorDriverChannel : public Maxim_MotorDriverBasicChannel
	{
		typedef Maxim_MotorDriverBasicChannel inherited;

	protected:
		int FPWMPin;

	protected:
		virtual void UpdateSpeed()
		{
			byte AValue = abs( FSpeed - 0.5 ) * 2 * 255 + 0.5;
//			Serial.println( AValue );
//			Serial.println( FPWMPin );
			analogWrite( FPWMPin, AValue );
//			Serial.println( "---" );
		}

	public:
		Maxim_MotorDriverChannel( Maxim_MotorDriverBasicShield &AOwner, int AIndex, int APWMPin, float AInitialSpeed, float ASenseResistor ) :
			inherited( AOwner, AIndex, AInitialSpeed, ASenseResistor ),
			FPWMPin( APWMPin )
		{
		}
	};
//---------------------------------------------------------------------------
	class Maxim_MotorDriverComponentChannel : public Maxim_MotorDriverBasicChannel
	{
		typedef Maxim_MotorDriverBasicChannel inherited;

	public:
		OpenWire::SourcePin	MotorControlOutputPin;

	protected:
		virtual void UpdateSpeed()
		{
			float AValue = abs( FSpeed - 0.5 ) * 2;
//			Serial.println( FSpeed );
			MotorControlOutputPin.Notify( &AValue );
		}

	public:
		using inherited::inherited;
	};
//---------------------------------------------------------------------------
	void Maxim_MotorDriverBasicShield::SystemStart()
	{
		inherited::SystemStart();
		DoGPIOTransmissionAt( 0x4, 0b10000001 ); // Enable and enable the interrupts
		for( int i = 0x9; i < 0x9 + 4; ++i )
			DoGPIOTransmissionAt( i, 0b01010101 ); // All Output

		DoGPIOTransmissionAt( 0x0E, 0xFF ); // All inputs
		DoGPIOTransmissionAt( 0x0E, 0b01111111 ); // P31 Poutput, the rest inputs

		UpdateSettings();

		for( int i = 0; i < FChannels.size(); ++i )
			FChannels[ i ]->InitFailure();

	}
//---------------------------------------------------------------------------
	void Maxim_MotorDriverBasicShield::UpdateFailures()
	{
		DoGPIOReadAt( 6 ); // Clear the interrupt bit

		byte AFailureFlags = DoGPIOReadAt( 0x5B ); // Read pins 27 and up
		for( int i = 0; i < FChannels.size(); ++ i )
			FChannels[ i ]->UpdateFailure( AFailureFlags & ( 1 << i ) == 0 );

	}
//---------------------------------------------------------------------------
	void Maxim_MotorDriverBasicShield::UpdateSettings()
	{
		if( FChanged & 0b00110000 )
		{
			byte	MAX7300Reg = FChannels[ 0 ]->GetRegisterPins() | ( FChannels[ 1 ]->GetRegisterPins() << 4 );
//			Serial.println( MAX7300Reg );
			DoGPIOTransmissionAt( 0x44, MAX7300Reg );
		}

		if( FChanged & 0b11000000 )
		{
			byte	MAX7300Reg = FChannels[ 2 ]->GetRegisterPins() | ( FChannels[ 3 ]->GetRegisterPins() << 4 );
			DoGPIOTransmissionAt( 0x4C, MAX7300Reg );
		}

		if( FChanged & 0b00000011 )
		{
			if( FChanged & 0b00000001 )
				DoPotentiometerTransmission( F_MAX5387_1, 0b00010001, FChannels[ 0 ]->GetPotentiometer() );

			if( FChanged & 0b00000010 )
				DoPotentiometerTransmission( F_MAX5387_1, 0b00010010, FChannels[ 1 ]->GetPotentiometer() );

		}

		if( FChanged & 0b00001100 )
		{
			if( FChanged & 0b00000100 )
				DoPotentiometerTransmission( F_MAX5387_2, 0b00010001, FChannels[ 2 ]->GetPotentiometer() );

			if( FChanged & 0b00001000 )
				DoPotentiometerTransmission( F_MAX5387_2, 0b00010010, FChannels[ 3 ]->GetPotentiometer() );
		}

		for( int i = 0; i < FChannels.size(); ++i )
			FChannels[ i ]->UpdateSpeed();

		FChanged = 0;
		FModified = false;
	}
//---------------------------------------------------------------------------
}

#endif

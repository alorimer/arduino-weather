////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_SABERTOOTH_DUALMOTOR_h
#define _MITOV_SABERTOOTH_DUALMOTOR_h

#include <Mitov.h>

namespace Mitov
{
	class SabertoothDualMotorBasic : public Mitov::EnabledComponent
	{
		typedef Mitov::EnabledComponent inherited;

	public:
		OpenWire::SinkPin	MotorsInputPins[ 2 ];

	public:
		void SetEnabled( bool AValue )
		{
			if( AValue == Enabled )
				return;

			Enabled = AValue;
			if( Enabled )
				SendStart();

			else
				SendStop();
		}

	protected:
		Mitov::BasicSerialPort &FSerial;

	protected:
		float FSpeeds[2] = { -1.0, -1.0 };

	protected:
		virtual void SendStart() = 0;
		virtual void SendStop() = 0;
		virtual void SetSpeed( int AIndex, float ASpeed ) = 0;

	protected:
		void DoMotor1Receive( void *_Data )
		{
			SetSpeed( 0, *(float *)_Data );
		}

		void DoMotor2Receive( void *_Data )
		{
			SetSpeed( 1, *(float *)_Data );
		}

	public:
		SabertoothDualMotorBasic( Mitov::BasicSerialPort &ASerial ) :
			FSerial( ASerial )
		{
			MotorsInputPins[ 0 ].SetCallback( this, (OpenWire::TOnPinReceive)&SabertoothDualMotorBasic::DoMotor1Receive );
			MotorsInputPins[ 1 ].SetCallback( this, (OpenWire::TOnPinReceive)&SabertoothDualMotorBasic::DoMotor2Receive );
		}

	};
//---------------------------------------------------------------------------
	class SabertoothDualMotorSimple : public SabertoothDualMotorBasic
	{
		typedef SabertoothDualMotorBasic inherited;

	public:
		OpenWire::SourcePin	SlaveSelectOutputPin;

	protected:
		virtual void SendStart()
		{
			float AValue = FSpeeds[ 0 ];
			FSpeeds[ 0 ] = -1;
			SetSpeed( 0, AValue );

			AValue = FSpeeds[ 1 ];
			FSpeeds[ 1 ] = -1;
			SetSpeed( 1, AValue );
		}

		virtual void SendStop()
		{
			SendValue( 0 );
		}

		virtual void SetSpeed( int AIndex, float ASpeed )
		{
			ASpeed = constrain( ASpeed, 0, 1 );
			if( FSpeeds[ AIndex ] == ASpeed )
				return;

			FSpeeds[ AIndex ] = ASpeed;
			if( ! Enabled )
				return;

			byte AValue = ASpeed * 126;

			if( AIndex )
				AValue += 128;

			else
				++ AValue;

			SendValue( AValue );
		}

		void SendValue( byte AValue )
		{
			SlaveSelectOutputPin.SendValue( true );

			FSerial.GetStream().write( AValue );

			if( SlaveSelectOutputPin.IsConnected() )
			{
				delayMicroseconds( 50 );
				SlaveSelectOutputPin.SendValue( false );
			}
		}

	protected:
		virtual void SystemStart() override
		{
			SlaveSelectOutputPin.SendValue( false );
			inherited::SystemStart();
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class SabertoothDualMotorPacket : public SabertoothDualMotorBasic
	{
		typedef SabertoothDualMotorBasic inherited;

/*
	public:
		class Motor : public OpenWire::Object
		{
		public:
			OpenWire::SinkPin	InputPin;
		protected:
			bool FEnabled;

		protected:
			SabertoothDualMotorPacket &FOwner;
			int	FIndex;
			float FSpeed;

		public:
			void SetEnabled( bool AValue )
			{
				if( AValue == FEnabled )
					return;

				FEnabled = AValue;
				if( FEnabled )
					SendStart();

				else
					SendStop();
			}

		protected:
			virtual void SendStart()
			{
				byte  AValue = FSpeed * 255 + 0.5;
				byte  ACommand = FIndex << 2; // 0 or 4
				if( ( AValue & B10000000 ) == 0 )
					ACommand |= 1;

				FOwner.SendCommand( ACommand, AValue & B01111111 );
			}

			virtual void SendStop()
			{
			}

		protected:
			void DoMotorReceive( void *_Data )
			{
				float ASpeed = constrain( *(float *)_Data, 0, 1 );
				if( ASpeed == FSpeed )
					return;

				FSpeed = ASpeed;
				if( Enabled )
					SendStart();
			}

		public:
			Motor( SabertoothDualMotorPacket &AOwner, int AIndex ) : 
				FOwner( AOwner ),
				FIndex( AIndex ),
				FEnabled( true )
			{
				InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&Motor::DoMotorReceive );
			}
		};
*/
	public:
		byte	Address = 128;

		float	MinVoltage = 6.0f;
		float	MaxVoltage = 25.0;

		byte	Ramping = 0;

		bool	DirectionalMode = false;
		uint16_t	SerialTimeout = 0;
		byte	Deadband = 0;

	public:
		void SetMinVoltage( float AValue )
		{
			AValue = constrain( AValue, 6.0, 30.0 );
			if( MinVoltage == AValue )
				return;

			MinVoltage = AValue;
			SendCommand( 2, ( MinVoltage - 6 ) * 5 );
		}

		void SetMaxVoltage( float AValue )
		{
			AValue = constrain( AValue, 0.0, 25.0 );
			if( MaxVoltage == AValue )
				return;

			MaxVoltage = AValue;
			SendCommand( 3, MinVoltage * 5.12 );
		}

		void SetRamping( byte AValue )
		{
			AValue = constrain( AValue, 0, 80 );
			if( Ramping == AValue )
				return;

			Ramping = AValue;
			SendCommand( 16, Ramping );
		}

		void SetSerialTimeout( uint16_t AValue )
		{
			AValue = constrain( AValue, 0, 12700 );
			if( SerialTimeout == AValue )
				return;

			SerialTimeout = AValue;
			SendSerialTimeout();
		}

		void SetDeadband( byte AValue )
		{
			AValue = constrain( AValue, 0, 127 );
			if( Deadband == AValue )
				return;

			Deadband = AValue;
			SendCommand( 17, Deadband );
		}

	protected:
		void SendSerialTimeout()
		{
			SendCommand( 14, (SerialTimeout + 99) / 100 );
		}

		virtual void SendStart()
		{
			SendStartOne( 0 );
			SendStartOne( 1 );
		}

		void SendStartOne( int AIndex )
		{
			int  AValue = ( FSpeeds[ AIndex ] * 255 + 0.5 ) - 128;
			byte  ACommand;
			if( DirectionalMode )
			{
				if( AIndex == 0 )
					ACommand = 10;

				else
					ACommand = 8;
			}

			else
				ACommand = AIndex << 2; // 0 or 4

			if( AValue < 0 )
			{
				AValue = -AValue;
				ACommand |= 1;
			}

			SendCommand( ACommand, AValue & B01111111 );
		}

		virtual void SendStop()
		{
			// Shut down both motors
			SendCommand( 0, 0 );
			SendCommand( 4, 0 );
		}

		virtual void SetSpeed( int AIndex, float ASpeed )
		{
			ASpeed = constrain( ASpeed, 0, 1 );
			if( ASpeed == FSpeeds[ AIndex ] )
				return;

			FSpeeds[ AIndex ] = ASpeed;
			if( Enabled )
				SendStart();
		}

	protected:
		void SendCommand(byte ACommand, byte AValue)
		{
			Stream &AStream = FSerial.GetStream();

			byte AAddress = Address | B10000000;
			AStream.write( AAddress );
			AStream.write( ACommand );
			AStream.write( AValue );
			AStream.write(( AAddress + ACommand + AValue ) & B01111111);
		}

	protected:
		virtual void SystemStart() 
		{
			if( MinVoltage != 6.0 )
				SendCommand( 2, ( MinVoltage - 6 ) * 5 );

			if( MaxVoltage != 25.0 )
				SendCommand( 3, MinVoltage * 5.12 );

			if( Ramping )
				SendCommand( 16, Ramping );

			if( SerialTimeout )
				SendSerialTimeout();

			if( Deadband )
				SendCommand( 17, Deadband );

			inherited::SystemStart();
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
}

#endif

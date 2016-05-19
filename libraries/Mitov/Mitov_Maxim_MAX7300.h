////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_MAXIM_MAX7300_h
#define _MITOV_MAXIM_MAX7300_h

#include <Mitov.h>
#include <Mitov_Basic_GPIO.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class Maxim_MAX7300_Channel;
//---------------------------------------------------------------------------
	class Maxim_MAX7300 : public Mitov::EnableBasicGPIO<Mitov::EnabledComponent>
	{
		typedef Mitov::EnableBasicGPIO<Mitov::EnabledComponent> inherited;

	public:
		byte	Address = 0;

	protected:
		bool	FModifiedMap = false;
		bool	FModifiedPorts = false;
		byte	FConfigMap[ 7 ][ 2 ];
		uint32_t    FPorts[ 2 ];
		uint32_t    FReadMap = 0;
		uint32_t    FReadRegistersMap = 0;
		uint32_t    FReadBits;

	protected:
		void BeginTransmissionAt( byte ARegister )
		{
			byte AAddress = 0b01000000 | ( Address & 0b1111 );

			Wire.beginTransmission( AAddress );

			Wire.write( ARegister );
		}

		void WriteRegister( byte ARegister, byte AValue )
		{
//			Serial.print( "WriteRegister: " ); Serial.print( ARegister ); Serial.print( " " ); Serial.println( AValue );

			BeginTransmissionAt( ARegister );
			Wire.write( AValue );
			Wire.endTransmission();
		}

		virtual void UpdateEnable()
		{
			if( Enabled )
				WriteRegister( 4, 1 );

			else
				WriteRegister( 4, 0 );

		}

		void UpdateConfigMap()
		{
//			Serial.println( "UpdateConfigMap" );
			bool AInUpdate = false;
			for( int i = 0; i < 7; ++i )
			{
				if( FConfigMap[ i ][ 0 ] != FConfigMap[ i ][ 1 ] )
				{
					if( ! AInUpdate )
					{
//						Serial.print( "BeginTransmissionAt: " ); Serial.println( 9 + i );
						BeginTransmissionAt( 9 + i );
						AInUpdate = true;
					}

//					Serial.println( i );
//					Serial.println( FConfigMap[ i ][ 0 ] );

					Wire.write( FConfigMap[ i ][ 0 ] );
					FConfigMap[ i ][ 1 ] = FConfigMap[ i ][ 0 ];
				}

				else if( AInUpdate )
				{
					Wire.endTransmission();
					AInUpdate = false;
				}
			}

			if( AInUpdate )
				Wire.endTransmission();

			FReadRegistersMap = 0;
			for( int i = 0; i <= 0x58 - 0x44; ++i )
				if( FReadMap & ((uint32_t)0xFF) << i )
				{
					FReadRegistersMap |= ((uint32_t)1 ) << i;
					i += 7;
				}

//			Serial.print( "FReadRegistersMap: " ); Serial.println( FReadRegistersMap );

			FModifiedMap = false;
		}

		void UpdatePorts()
		{
//			Serial.println( "UpdatePorts" );
//			Serial.println( FPorts[ 0 ] );
//			Serial.println( FPorts[ 1 ] );
			for( int i = 0; i < 28; ++i )
			{
				uint32_t AMask = ((uint32_t)1) << i;
//				Serial.println( AMask );
				if( ( FPorts[ 0 ] & AMask ) != ( FPorts[ 1 ] & AMask ) )
				{
//   					Serial.println( i );
//					Serial.println( AMask );
					WriteRegister( 0x44 + i, FPorts[ 0 ] >> i );
					i += 8;
//					for( int j = i / 8; j < 4; j ++ )
//						WriteRegister( 0x44 + j * 8, FPorts[ 0 ] >> j * 8 );

//					break;
				}
			}
			
			FPorts[ 1 ] = FPorts[ 0 ];
			FModifiedPorts = false;
		}

	protected:
		virtual void SystemStart()
		{
			inherited::SystemStart();
			UpdateEnable();

			for( int i = 0; i < 7; ++i )
				FConfigMap[ i ][ 1 ] = ~FConfigMap[ i ][ 0 ];

			FPorts[ 1 ] = ~FPorts[ 0 ];

			UpdateConfigMap();
			UpdatePorts();

//			UpdateOutput();
		}

		virtual void SystemLoopUpdateHardware()
		{
			if( FModifiedMap )
//				if( ! ClockInputPin.IsConnected() )
				UpdateConfigMap();

			if( FModifiedPorts )
				UpdatePorts();

			inherited::SystemLoopUpdateHardware();
		}

	public:
		bool GetBitValue( uint32_t AIndex )
		{
			return( ( FReadBits & ( ((uint32_t)1 ) << AIndex )) != 0 );
		}

		void SetChannelValue( int AIndex, bool AValue )
		{
//			AIndex += 4;

//			Serial.println( "SetChannelValue" );
//			Serial.println( AIndex );
//			Serial.println( AValue );

			FPorts[ 0 ] &= ~( ((uint32_t)0b1 ) << AIndex );
			FPorts[ 0 ] |= ( ((uint32_t)AValue ) & 1 ) << AIndex;
			FModifiedPorts = true;
		}

		void SetChannelMode( int AIndex, int AMode )
		{
//			AIndex += 4;

//			Serial.println( "SetChannelMode" );
//			Serial.println( AIndex );
//			Serial.println( AMode );

			int AConfigIndex = AIndex / 4;
			int AOffset = ( AIndex % 4 ) * 2;

//			Serial.println( AConfigIndex );
//			Serial.println( AOffset );

			FConfigMap[ AConfigIndex ][ 0 ] &= ~( ((uint32_t)0b11) << AOffset );
			FConfigMap[ AConfigIndex ][ 0 ] |= ((uint32_t)AMode) << AOffset;

			if( AMode & 0b10 ) // Check if it is input
				FReadMap |= ((uint32_t)1) << AIndex;

			else
				FReadMap &= ~( ((uint32_t)1) << AIndex );

			FModifiedMap = true;
		}

	protected:
		virtual void PerformRead()
		{
			if( ! FReadRegistersMap)
				return;

			FReadBits = 0;
			for( int i = 0; i <= 0x58 - 0x44; ++i )
				if( (((uint32_t)1 ) << i ) & FReadRegistersMap )
				{
					BeginTransmissionAt( 0x44 + i );
					Wire.endTransmission();
					Wire.requestFrom( 0b01000000 | ( Address & 0b1111 ), 1 );
					while (Wire.available() < 1 );

					uint32_t	ARegister = Wire.read();

					FReadBits |= ARegister << i;

//					Serial.print( i ); Serial.print( ": " ); Serial.println( ARegister );

					i += 7;
				}

			for( int i = 0; i < inherited::FChannels.size(); ++i )
				inherited::FChannels[ i ]->UpdateInput();

		}

	public:
		Maxim_MAX7300()
		{
			for( int i = 0; i < 7; ++i )
				FConfigMap[ i ][ 0 ] = 0b10101010;
		}

	};
//---------------------------------------------------------------------------
	class Maxim_MAX7300_Channel : public OwnedBasicGPIOChannel<Maxim_MAX7300>
	{
		typedef OwnedBasicGPIOChannel<Maxim_MAX7300> inherited;
		

	public:
		virtual void UpdateInput()
		{			
			UpdateOutput( FOwner.GetBitValue( FIndex ));
		}

	protected:
        virtual void PinDirectionsInit()
        {
            if( FIsOutput )
			{
                FOwner.SetChannelMode( FIndex, 1 ); //pinMode( FPinNumber, OUTPUT );
				FOwner.SetChannelValue( FIndex, FValue );
			}

            else
            {
                if( FIsPullUp )
                    FOwner.SetChannelMode( FIndex, 3 ); //pinMode( FPinNumber, INPUT_PULLUP );

                else
                    FOwner.SetChannelMode( FIndex, 2 ); //pinMode( FPinNumber, INPUT );
            }
        }

	public:
		Maxim_MAX7300_Channel( Maxim_MAX7300 &AOwner, int AIndex, bool AInitialValue, bool AIsOutput, bool AIsPullUp, bool AIsCombinedInOut ) :
			inherited( AOwner, AIndex, AInitialValue, AIsOutput, AIsPullUp, AIsCombinedInOut )
		{
			PinDirectionsInit();
		}

	};
//---------------------------------------------------------------------------
}

#endif

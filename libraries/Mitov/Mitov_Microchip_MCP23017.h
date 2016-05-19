////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_MICROCHIP_MCP23017_h
#define _MITOV_MICROCHIP_MCP23017_h

#include <Mitov.h>
#include <Mitov_Basic_GPIO.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class Microchip_MCP23017 : public Mitov::BasicGPIO<OpenWire::Component>
	{
		typedef Mitov::BasicGPIO<OpenWire::Component> inherited;

	public:
		byte	Address = 0;

	public:
		enum TRegisterIndex { riDirection, riPullUp, riInvert, riValue };

	protected:
		uint16_t	FRegValues[ 4 ][ 2 ];
		uint16_t	FReadRegisterMap = 0;
		uint16_t	FReadBits = 0;


	protected:
		void BeginTransmissionAt( byte ARegister )
		{
			byte AAddress = 0b0100000 | ( Address & 0b111 );

			Wire.beginTransmission( AAddress );

			Wire.write( ARegister );
		}

		void WriteAt( byte ARegister, uint16_t AData )
		{
//			Serial.print( "WriteAt: " ); Serial.print( ARegister ); Serial.print( " " ); Serial.println( AData ); 
			BeginTransmissionAt( ARegister );

			Wire.write( (byte *)&AData, 2 );

			Wire.endTransmission();
		}

		void StartReadFrom( byte ARegister, int ALength )
		{
			BeginTransmissionAt( ARegister );
			Wire.endTransmission();
			Wire.requestFrom( 0b0100000 | ( Address & 0b111 ), ALength );
			while( Wire.available() < ALength );
		}

		void UpdateAll()
		{
			const byte CPorts[] = 
			{ 
				0x00,	// IODIR register
				0x02,	// IPOL register
				0x0C,	// GPPU register
				0x14	// OLAT register
			};

			for( int i = 0; i < 4; ++i )
				if( FRegValues[ i ][ 0 ] != FRegValues[ i ][ 1 ] )
				{
					WriteAt( CPorts[ i ], FRegValues[ i ][ 0 ] );
					FRegValues[ i ][ 1 ] = FRegValues[ i ][ 0 ];
				}

/*
			// Read the port 
			BeginTransmissionAt( 0x12 ); // GPIO register
			Wire.endTransmission();

			Wire.requestFrom( 0b0100000 | ( Address & 0b111 ), 2 );
			uint16_t	AValue;
			((byte *)&AValue )[ 0 ] = Wire.read();
			((byte *)&AValue )[ 1 ] = Wire.read();

			for( int i = 0; i < FChannels.size(); ++i )
				FChannels[ i ]->UpdateOutput( AValue & ( 1 << i ) != 0 );
*/
		}

		virtual void PerformRead()
		{
			if( ! FReadRegisterMap )
				return;

//			Serial.println( FReadRegisterMap );

			if( FReadRegisterMap & 0x0F )
			{
				if( FReadRegisterMap & 0xF0 )
				{
					StartReadFrom( 0x12, 2 ); // GPIO register
					((byte *)&FReadBits )[ 0 ] = Wire.read();
					((byte *)&FReadBits )[ 1 ] = Wire.read();
				}

				else
				{
					StartReadFrom( 0x12, 1 ); // GPIO register
					((byte *)&FReadBits )[ 0 ] = Wire.read();
					((byte *)&FReadBits )[ 1 ] = 0;

//					Serial.print( "FReadBits: " ); Serial.println( FReadBits );
				}
			}
			else
			{
				// Read the port 
				StartReadFrom( 0x13, 1 ); // GPIO register
				((byte *)&FReadBits )[ 0 ] = 0;
				((byte *)&FReadBits )[ 1 ] = Wire.read();
			}

			for( int i = 0; i < FChannels.size(); ++i )
				FChannels[ i ]->UpdateInput();

		}

	public:
		bool GetBitValue( uint32_t AIndex )
		{
			return( ( FReadBits & ( ((uint32_t)1 ) << AIndex )) != 0 );
		}

	public:
		void SetChannelValue( int AIndex, bool AValue )
		{
			SetChannelRegisterValue( AIndex, Microchip_MCP23017::riValue, AValue );
		}

	protected:
		virtual void SystemStart()
		{
			inherited::SystemStart();

			for( int i = 0; i < 4; ++i )
				FRegValues[ i ][ 1 ] = ~FRegValues[ i ][ 0 ];

			UpdateAll();

			for( int i = 0; i < FChannels.size(); ++i )
				FChannels[ i ]->SendOutput();
		}

		virtual void SystemLoopUpdateHardware()
		{
			UpdateAll();
			inherited::SystemLoopUpdateHardware();
		}

	public:
		void SetChannelRegisterValue( int AIndex, int ARegisterIndex, bool AValue )
		{
			FRegValues[ ARegisterIndex ][ 0 ] &= ~( ((uint16_t) 0b1 ) << AIndex );
			FRegValues[ ARegisterIndex ][ 0 ] |= ( (uint16_t)( AValue & 1 ) ) << AIndex;

			if( ARegisterIndex == Microchip_MCP23017::riDirection )
			{
				if( AValue )
					FReadRegisterMap |= ((uint16_t) 1) << AIndex;

				else
					FReadRegisterMap &= ~( ((uint16_t) 1) << AIndex );
			}
		}

	public:
		Microchip_MCP23017()
		{
			for( int i = 0; i < 4; ++i )
				FRegValues[ i ][ 0 ] = 0;
		}

	};
//---------------------------------------------------------------------------
	class Microchip_MCP23017_Channel : public OwnedBasicGPIOChannel<Microchip_MCP23017>
	{
		typedef OwnedBasicGPIOChannel<Microchip_MCP23017> inherited;
		
	public:
        void SetIsInverted( bool AValue )
        {
            if( FIsInverted == AValue )
                return;

            FIsInverted = AValue;
            PinDirectionsInit();
        }

	protected:
        bool FIsInverted;

	public:
		virtual void UpdateInput()
		{			
			UpdateOutput( FOwner.GetBitValue( FIndex ));
		}

	protected:
        virtual void PinDirectionsInit()
        {
			FOwner.SetChannelRegisterValue( FIndex, Microchip_MCP23017::riDirection, ! FIsOutput );
			FOwner.SetChannelRegisterValue( FIndex, Microchip_MCP23017::riPullUp, FIsPullUp );
			FOwner.SetChannelRegisterValue( FIndex, Microchip_MCP23017::riInvert, FIsInverted );
        }

	public:
		Microchip_MCP23017_Channel( Microchip_MCP23017 &AOwner, int AIndex, bool AInitialValue, bool AIsOutput, bool AIsPullUp, bool AIsCombinedInOut, bool AIsInverted ) :
			inherited( AOwner, AIndex, AInitialValue, AIsOutput, AIsPullUp, AIsCombinedInOut ),
			FIsInverted( AIsInverted )
		{
			PinDirectionsInit();
		}

	};
//---------------------------------------------------------------------------
}

#endif

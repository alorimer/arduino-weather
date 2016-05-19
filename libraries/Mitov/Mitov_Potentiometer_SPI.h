////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_POTENTIOMETER_SPI_h
#define _MITOV_POTENTIOMETER_SPI_h

#include <Mitov.h>
#include <Mitov_Basic_SPI.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class PotentiometerSPIChannel;
//---------------------------------------------------------------------------
	class Potentiometer_SPI : public Mitov::Basic_MultiChannel_SourceSPI
	{
		typedef Mitov::Basic_MultiChannel_SourceSPI inherited;

	protected:
		static const uint8_t kADR_WIPER0       = B00000000;
		static const uint8_t kADR_WIPER1       = B00010000;

		static const uint8_t kCMD_READ         = B00001100;
	    static const uint8_t kCMD_WRITE        = B00000000;

	    static const uint8_t kADR_VOLATILE     = B00000000;
		static const uint8_t kADR_NON_VOLATILE = B00100000;

		static const uint8_t kTCON_REGISTER    = B01000000;
		static const uint8_t kSTATUS_REGISTER  = B01010000;

	protected:
		uint16_t byte2uint16(byte high_byte, byte low_byte)
		{
			return (uint16_t)high_byte<<8 | (uint16_t)low_byte;
		}

	public:
		void	SetEnabled( bool AValue )
		{
			if( Enabled == AValue )
				return;

			Enabled = AValue;
			for( int i =0; i < FChannels.size(); ++i )
				FChannels[ i ]->InitChannel();
		}

	public:
		void SPI_write(byte cmd_byte, byte data_byte)
		{
			cmd_byte |= kCMD_WRITE;
			ChipSelectOutputPin.SendValue( false );
			FSPI.transfer(cmd_byte);
			FSPI.transfer(data_byte);
			ChipSelectOutputPin.SendValue( true );
		}

		uint16_t SPI_read(byte cmd_byte)
		{
			cmd_byte |= kCMD_READ;
			ChipSelectOutputPin.SendValue( false );
			byte high_byte = FSPI.transfer(cmd_byte);
			byte low_byte  = FSPI.transfer(0xFF);
			ChipSelectOutputPin.SendValue( true );
			return byte2uint16(high_byte, low_byte);		
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class PotentiometerSPIChannel : public Mitov::Basic_Typed_SPIChannel<Potentiometer_SPI>
	{
		typedef Mitov::Basic_Typed_SPIChannel<Potentiometer_SPI>	inherited;

	public:
		bool	Enabled = true;

		bool	ConnectTerminalA = true;
		bool	ConnectWiper = true;
		bool	ConnectTerminalB = true;
		bool	NonVolatile = false;

	protected:
		int		FMultiplier;

	public:
		virtual void InitChannel()
		{
			uint16_t ATerminalControlValue = FOwner.SPI_read( 4 );
			uint16_t ANewValue;
			if( ConnectTerminalB )
				ANewValue = 1;

			else
				ANewValue = 0;

			if( ConnectWiper )
				ANewValue |= B10;

			if( ConnectTerminalA )
				ANewValue |= B100;

			if( ! Enabled )
				ANewValue |= B1000;

			if( FIndex == 1 )
			{
				ATerminalControlValue &= 0xFF0F;
				ATerminalControlValue |= ( ANewValue << 4 );
			}
			else
			{
				ATerminalControlValue &= 0xFFF0;
				ATerminalControlValue |= ANewValue;
			}

			FOwner.SPI_write( 4, ATerminalControlValue );
		}

		virtual void SendOutput()
		{
			if( FNewValue == FValue )
				return;

			int wiper_pos = ( FValue * FMultiplier );

			wiper_pos = constrain( wiper_pos, 0, FMultiplier );

			FValue = FNewValue;
			byte data_byte;
			byte cmd_byte = FIndex;

			// Calculate the 9-bit data value to send
			data_byte = (byte)(wiper_pos & 0x00FF);
			if(wiper_pos > FMultiplier - 1)
				if( FMultiplier == 256 )
					cmd_byte |= B00000001; // Table 5-1 (page 36)

			FOwner.SPI_write( cmd_byte, data_byte );

			if( NonVolatile )
			{
				// EEPROM write cycles take 4ms each. So we block with delay(5); after any NV Writes
				FOwner.SPI_write( cmd_byte | B10, data_byte );
				delay(5);
			}
		}

	public:
		void	SetEnabled( bool AValue )
		{
			if( Enabled == AValue )
				return;

			Enabled = AValue;
			InitChannel();
		}

		void	SetConnectTerminalA( bool AValue )
		{
			if( ConnectTerminalA == AValue )
				return;

			ConnectTerminalA = AValue;
			InitChannel();
		}

		void	SetConnectWiper( bool AValue )
		{
			if( ConnectWiper == AValue )
				return;

			ConnectWiper = AValue;
			InitChannel();
		}

		void	SetConnectTerminalB( bool AValue )
		{
			if( ConnectTerminalB == AValue )
				return;

			ConnectTerminalB = AValue;
			InitChannel();
		}

		void	SetNonVolatile( bool AValue )
		{
			if( NonVolatile == AValue )
				return;

			NonVolatile = AValue;
			InitChannel();
		}

	public:
		PotentiometerSPIChannel( Potentiometer_SPI &AOwner, int AIndex, int AMultiplier ) :
			inherited( AOwner, AIndex ),
			FMultiplier( AMultiplier )
		{
		}

	};
//---------------------------------------------------------------------------
}

#endif

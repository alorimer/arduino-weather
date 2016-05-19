////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_MICROCHIP_DAC_SPI_h
#define _MITOV_MICROCHIP_DAC_SPI_h

#include <Mitov.h>
#include <Mitov_Basic_SPI.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class Microchip_DAC_SPI : public Mitov::Basic_MultiChannel_SourceSPI
	{
		typedef Mitov::Basic_MultiChannel_SourceSPI inherited;

	public:
		OpenWire::SourcePin	LatchOutputPin;

	public:
		void	SetEnabled( bool AValue )
		{
			if( Enabled == AValue )
				return;

			Enabled = AValue;
			for( int i =0; i < FChannels.size(); ++i )
				FChannels[ i ]->InitChannel();
		}

	protected:
		void UpdateLatch()
		{
			LatchOutputPin.SendValue( false );
			LatchOutputPin.SendValue( true );
		}

	protected:
		virtual void SystemLoopEnd()
		{
			inherited::SystemLoopEnd();

			if( ! ClockInputPin.IsConnected() )
				UpdateLatch();
		}

		virtual void DoClockReceive( void *_Data )
		{
			inherited::DoClockReceive( _Data );

			UpdateLatch();
		}

	public:
		void SPI_write( word APacket )
		{
			ChipSelectOutputPin.SendValue( false );
			FSPI.transfer( APacket >> 8 );
			FSPI.transfer( APacket & 0xFF );
			ChipSelectOutputPin.SendValue( true );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class Microchip_DACSPIChannel : public Mitov::Basic_Typed_SPIChannel<Microchip_DAC_SPI>
	{
		typedef Mitov::Basic_Typed_SPIChannel<Microchip_DAC_SPI>	inherited;

	public:
		bool	Enabled : 1;
		bool	Gain : 1;

	public:
		void SendOutput()
		{
			if( FNewValue == FValue )
				return;

			word volt_digits = FNewValue * 4095 + 0.5;

			word packet = volt_digits & 0b111111111111;  //shift voltage setting digits
			if( Enabled )
				packet |= 1 << 12;

			if( Gain )
				packet |= 1 << 13;   //add gain setting

			if( FIndex )
				packet |= 1 << 15;   //add gain setting

			FOwner.SPI_write( packet );
		}

	public:
		void	SetEnabled( bool AValue )
		{
			if( Enabled == AValue )
				return;

			Enabled = AValue;
			InitChannel();
		}

		void	SetGain( bool AValue )
		{
			if( Gain == AValue )
				return;

			Gain = AValue;
			InitChannel();
		}

	public:
		Microchip_DACSPIChannel( Microchip_DAC_SPI &AOwner, int AIndex, int ABits ) :
			inherited( AOwner, AIndex ),
			Enabled( true ),
			Gain( false )
		{
		}

	};
//---------------------------------------------------------------------------
}

#endif

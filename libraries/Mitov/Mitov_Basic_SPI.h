////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_BASIC_SPI_h
#define _MITOV_BASIC_SPI_h

#include <Mitov.h>
#include <SPI.h>

namespace Mitov
{
	class BasicSPI : public OpenWire::Component
	{
	public:
		virtual uint16_t transfer16(uint16_t data) = 0;
		virtual uint8_t transfer(uint8_t data) = 0;
		virtual void transfer(void *buf, size_t count) = 0;
		virtual void beginTransaction(SPISettings settings) = 0;
		virtual void endTransaction() = 0;
	};
//---------------------------------------------------------------------------
	class Basic_SPIChannel;
//---------------------------------------------------------------------------
	class Basic_SPI : public Mitov::EnabledComponent
	{
		typedef Mitov::EnabledComponent inherited;

	public:
		OpenWire::SourcePin	ChipSelectOutputPin;

	protected:
		BasicSPI	&FSPI;

	public:
		Basic_SPI( BasicSPI &ASPI ) :
			FSPI( ASPI )
		{
		}

	};
//---------------------------------------------------------------------------
	class Basic_MultiChannel_SourceSPI : public Mitov::Basic_SPI, public ClockingSupport
	{
		typedef Mitov::Basic_SPI inherited;

	public:
		bool	FModified = false;

	public:
		Mitov::SimpleList<Basic_SPIChannel *>	FChannels;

	protected:
		virtual void SystemInit();

	protected:
		virtual void DoClockReceive( void * ) override;

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class Basic_SPIChannel : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	public:
		virtual void InitChannel() {}
		virtual void SendOutput() = 0;

	};
//---------------------------------------------------------------------------
	template<typename T_OWNER> class Basic_Typed_SPIChannel : public Mitov::Basic_SPIChannel
	{
		typedef Mitov::Basic_SPIChannel	inherited;

	public:
		float	InitialValue = 0.0f;

	protected:
		float	FValue = 0.0f;
		float	FNewValue = 0.0f;

		int		FIndex;

	protected:
		T_OWNER	&FOwner;

	protected:
		virtual void DoReceive( void *_Data )
		{
			FNewValue = constrain( *((float *)_Data), 0.0f, 1.0f );
			if( FNewValue == FValue )
				return;

			if( FOwner.ClockInputPin.IsConnected() )
				FOwner.FModified = true;

			else
				SendOutput();

		}

	public:
		Basic_Typed_SPIChannel( T_OWNER &AOwner, int AIndex ) :
			FOwner( AOwner ),
			FIndex( AIndex )
		{
			AOwner.FChannels.push_back( this );
		}

	};
//---------------------------------------------------------------------------
	void Basic_MultiChannel_SourceSPI::DoClockReceive( void * )
	{
		if( ! FModified )
			return;

		for( int i =0; i < FChannels.size(); ++i )
			FChannels[ i ]->SendOutput();
	}
//---------------------------------------------------------------------------
	void Basic_MultiChannel_SourceSPI::SystemInit()
	{
		inherited::SystemInit();
		ChipSelectOutputPin.SendValue( true );

		for( int i =0; i < FChannels.size(); ++i )
			FChannels[ i ]->InitChannel();
	}
//---------------------------------------------------------------------------
}
#endif

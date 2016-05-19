////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_MAXIM_MAX521X_h
#define _MITOV_MAXIM_MAX521X_h

#include <Mitov.h>
#include <Mitov_Basic_SPI.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	enum Maxim_MAX521X_ShutDownMode { msdmHighImpedance, msdm100K, msdm1K };
//---------------------------------------------------------------------------
	class Maxim_MAX521X : public Mitov::Basic_SPI, public ClockingSupport
	{
		typedef Mitov::Basic_SPI inherited;

	public:
		OpenWire::SinkPin	InputPin;

	public:
		Maxim_MAX521X_ShutDownMode	ShutDownMode = msdmHighImpedance;

	public:
		void SetEnabled( bool AValue )
		{
			if( Enabled == AValue )
				return;

			Enabled = AValue;
			if( Enabled )
				SPI_write( 2, 0 );

			else
				SPI_write( 2, ( ((word)ShutDownMode) + 1 ) << 10 );

		}

	protected:
		word FValue = 0;

	protected:
		void UpdateValue()
		{
			SPI_write( 1, FValue );
		}

		virtual void DoClockReceive( void *_Data ) override
		{
			UpdateValue();
		}

	protected:
		virtual void SPI_transfer( byte AAddress, word AData ) = 0;

		void SPI_write( byte AAddress, word AData )
		{
			ChipSelectOutputPin.SendValue( false );
			SPI_transfer( AAddress, AData );
			ChipSelectOutputPin.SendValue( true );
		}

		void DoReceive( void *_Data )
        {
			FValue = constrain( *(float*)_Data, 0, 1 ) * 65536 + 0.5;
			if( ! ClockInputPin.IsConnected() )
				UpdateValue();
        }

	public:
		Maxim_MAX521X( BasicSPI &ASPI ) :
			inherited( ASPI )
		{
			InputPin.SetCallback( MAKE_CALLBACK( Maxim_MAX521X::DoReceive ));
		}
	};
//---------------------------------------------------------------------------
	class Maxim_MAX5214 : public Mitov::Maxim_MAX521X
	{
		typedef Mitov::Maxim_MAX521X inherited;

	protected:
		virtual void SPI_transfer( byte AAddress, word AData )
		{
			AData >>= 2;
			AData |= ((word)AAddress) << 14;
			FSPI.transfer16( AData );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class Maxim_MAX5216 : public Mitov::Maxim_MAX521X
	{
		typedef Mitov::Maxim_MAX521X inherited;

	protected:
		virtual void SPI_transfer( byte AAddress, word AData )
		{
			byte AHighByte = AData >> 10;
			AHighByte |= AAddress << 6;

			AData <<= 6;

			FSPI.transfer( AHighByte );
			FSPI.transfer16( AData );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
}

#endif

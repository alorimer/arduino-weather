////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_TEENSY_SERIAL_h
#define _MITOV_TEENSY_SERIAL_h

#include <Mitov.h>

namespace Mitov
{
	const	long CTeensySerialInits[] = 
	{
		SERIAL_7E1,
		SERIAL_7E1,
		SERIAL_7O1,
		SERIAL_8N1,
		SERIAL_8E1,
		SERIAL_8O1,

		SERIAL_8N2,
		SERIAL_8N2,
		SERIAL_8N2,
		SERIAL_8N2,
		SERIAL_8N2,
		SERIAL_8N2
/*
		SERIAL_7E1_RXINV,
		SERIAL_7E1_RXINV,
		SERIAL_7O1_RXINV,
		SERIAL_8N1_RXINV,
		SERIAL_8E1_RXINV,
		SERIAL_8O1_RXINV,

		SERIAL_8N2_RXINV,
		SERIAL_8N2_RXINV,
		SERIAL_8N2_RXINV,
		SERIAL_8N2_RXINV,
		SERIAL_8N2_RXINV,
		SERIAL_8N2_RXINV,

		SERIAL_7E1_TXINV,
		SERIAL_7E1_TXINV,
		SERIAL_7O1_TXINV,
		SERIAL_8N1_TXINV,
		SERIAL_8E1_TXINV,
		SERIAL_8O1_TXINV,

		SERIAL_8N2_TXINV,
		SERIAL_8N2_TXINV,
		SERIAL_8N2_TXINV,
		SERIAL_8N2_TXINV,
		SERIAL_8N2_TXINV,
		SERIAL_8N2_TXINV,

		SERIAL_7E1_RXINV_TXINV,
		SERIAL_7E1_RXINV_TXINV,
		SERIAL_7O1_RXINV_TXINV,
		SERIAL_8N1_RXINV_TXINV,
		SERIAL_8E1_RXINV_TXINV,
		SERIAL_8O1_RXINV_TXINV,

		SERIAL_8N2_RXINV_TXINV,
		SERIAL_8N2_RXINV_TXINV,
		SERIAL_8N2_RXINV_TXINV,
		SERIAL_8N2_RXINV_TXINV,
		SERIAL_8N2_RXINV_TXINV,
		SERIAL_8N2_RXINV_TXINV
*/
/*
#ifdef SERIAL_9BIT_SUPPORT
#define SERIAL_9N1 0x84
#define SERIAL_9E1 0x8E
#define SERIAL_9O1 0x8F
#define SERIAL_9N1_RXINV 0x94
#define SERIAL_9E1_RXINV 0x9E
#define SERIAL_9O1_RXINV 0x9F
#define SERIAL_9N1_TXINV 0xA4
#define SERIAL_9E1_TXINV 0xAE
#define SERIAL_9O1_TXINV 0xAF
#define SERIAL_9N1_RXINV_TXINV 0xB4
#define SERIAL_9E1_RXINV_TXINV 0xBE
#define SERIAL_9O1_RXINV_TXINV 0xBF
#endif
*/
	};
//---------------------------------------------------------------------------
	template<typename T_SERIAL_TYPE, T_SERIAL_TYPE *T_SERIAL> class TeensySerialPort : public Mitov::SpeedSerialPort<T_SERIAL_TYPE, T_SERIAL>
	{
		typedef	Mitov::SpeedSerialPort<T_SERIAL_TYPE, T_SERIAL> inherited;

	public:
		TArduinoSerialParity	Parity : 3;
		unsigned int			StopBits : 2;
		unsigned int			DataBits : 3;
		bool					InvertedRX : 1;
		bool					InvertedTX : 1;

	public:
		void SetParity( TArduinoSerialParity AValue )
		{
            if( Parity == AValue )
                return;

            Parity = AValue;
            inherited::RestartPort();
		}

		void SetStopBits( unsigned int AValue )
		{
            if( StopBits == AValue )
                return;

            StopBits = AValue;
            inherited::RestartPort();
		}

		void SetDataBits( unsigned int AValue )
		{
            if( DataBits == AValue )
                return;

            DataBits = AValue;
            inherited::RestartPort();
		}

	protected:
		virtual void StartPort()
		{
//			int AIndex = ((int)Parity) * 8 + ( StopBits - 1 ) * 4 + ( DataBits - 5);
			int AIndex = ( StopBits - 1 ) * 6 + ( DataBits - 5) * 3 + ((int)Parity);
			long AValue = CTeensySerialInits[ AIndex ];
			if( InvertedRX )
				AValue |= 0x10;

			if( InvertedTX )
				AValue |= 0x20;

			T_SERIAL->begin( inherited::Speed, AValue );
		}

	public:
		TeensySerialPort() :
			Parity( spNone ),
			StopBits( 1 ),
			DataBits( 8 ),
			InvertedRX( false ),
			InvertedTX( false )
		{
		}
	};
//---------------------------------------------------------------------------
} // Mitov

#endif


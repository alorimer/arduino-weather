////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_FORMATTED_SERIAL_h
#define _MITOV_FORMATTED_SERIAL_h

#include <Mitov.h>

namespace Mitov
{
#ifdef VISUINO_ESP8266
	const	SerialConfig CSerialInits[] = 
#else
	const	long CSerialInits[] = 
#endif
	{
		SERIAL_5N1,
		SERIAL_6N1,
		SERIAL_7N1,
		SERIAL_8N1,
		SERIAL_5N2,
		SERIAL_6N2,
		SERIAL_7N2,
		SERIAL_8N2,
		SERIAL_5E1,
		SERIAL_6E1,
		SERIAL_7E1,
		SERIAL_8E1,
		SERIAL_5E2,
		SERIAL_6E2,
		SERIAL_7E2,
		SERIAL_8E2,
		SERIAL_5O1,
		SERIAL_6O1,
		SERIAL_7O1,
		SERIAL_8O1,
		SERIAL_5O2,
		SERIAL_6O2,
		SERIAL_7O2,
		SERIAL_8O2
	};
//---------------------------------------------------------------------------
	template<typename T_SERIAL_TYPE, T_SERIAL_TYPE *T_SERIAL> class SerialPort : public Mitov::SpeedSerialPort<T_SERIAL_TYPE, T_SERIAL>
	{
		typedef	Mitov::SpeedSerialPort<T_SERIAL_TYPE, T_SERIAL> inherited;

	public:
		TArduinoSerialParity	Parity	 : 4;
		unsigned int			StopBits : 4;
		unsigned int			DataBits : 4;

#ifdef VISUINO_ESP8266
		bool	ReceiveEnabled : 1;
		bool	TransmitEnabled : 1;

		void SetReceiveEnabled( bool AValue )
		{
			if( ReceiveEnabled == AValue )
				return;

			ReceiveEnabled = AValue;
			ResetSerial();
		}

		void SetTransmitEnabled( bool AValue )
		{
			if( TransmitEnabled == AValue )
				return;

			TransmitEnabled = AValue;
			ResetSerial();
		}

	protected:
		void ResetSerial()
		{
			T_SERIAL->end();
			StartPort();
		}


#endif // VISUINO_ESP8266
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
			int AIndex = ((int)Parity) * 8 + ( StopBits - 1 ) * 4 + ( DataBits - 5);
#ifdef VISUINO_ESP8266
			if( ReceiveEnabled && TransmitEnabled )
				T_SERIAL->begin( inherited::Speed, CSerialInits[ AIndex ], SERIAL_FULL );

			else if( ReceiveEnabled )
				T_SERIAL->begin( inherited::Speed, CSerialInits[ AIndex ], SERIAL_RX_ONLY );

			else if( TransmitEnabled )
				T_SERIAL->begin( inherited::Speed, CSerialInits[ AIndex ], SERIAL_TX_ONLY );

#else // VISUINO_ESP8266
			T_SERIAL->begin( inherited::Speed, CSerialInits[ AIndex ] );
#endif // VISUINO_ESP8266
		}

	public:
		SerialPort() :
#ifdef VISUINO_ESP8266
			ReceiveEnabled( true ),
			TransmitEnabled( true ),
#endif // VISUINO_ESP8266
			Parity( spNone ),
			StopBits( 1 ),
			DataBits( 8 )
		{
		}
	};
//---------------------------------------------------------------------------
} // Mitov

#endif


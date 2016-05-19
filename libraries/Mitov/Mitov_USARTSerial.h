////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_USART_SERIAL_h
#define _MITOV_USART_SERIAL_h

#include <Mitov.h>

namespace Mitov
{
	const	USARTClass::USARTModes CUSARTSerialInits[] = 
	{
		SERIAL_5N1,
		SERIAL_6N1,
		SERIAL_7N1,
		SERIAL_8N2,

		SERIAL_5N2,
		SERIAL_6N2,
		SERIAL_7N2,
		SERIAL_8N2,

		SERIAL_5E1,
		SERIAL_6E1,
		SERIAL_7E1,
		SERIAL_8E2,

		SERIAL_5E2,
		SERIAL_6E2,
		SERIAL_7E2,
		SERIAL_8E2,

		SERIAL_5O1,
		SERIAL_6O1,
		SERIAL_7O1,
		SERIAL_8O2,

		SERIAL_5O2,
		SERIAL_6O2,
		SERIAL_7O2,
		SERIAL_8O2,

		SERIAL_5M1,
		SERIAL_6M1,
		SERIAL_7M1,
		SERIAL_8M2,

		SERIAL_5M2,
		SERIAL_6M2,
		SERIAL_7M2,
		SERIAL_8M2,

		SERIAL_5S1,
		SERIAL_6S1,
		SERIAL_7S1,
		SERIAL_8S2,

		SERIAL_5S2,
		SERIAL_6S2,
		SERIAL_7S2,
		SERIAL_8S2
	};
//---------------------------------------------------------------------------
	template<typename T_SERIAL_TYPE, T_SERIAL_TYPE *T_SERIAL> class USARTSerialPort : public Mitov::SpeedSerialPort<T_SERIAL_TYPE, T_SERIAL>
	{
		typedef	Mitov::SpeedSerialPort<T_SERIAL_TYPE, T_SERIAL> inherited;

	public:
		TArduinoSerialParity	Parity : 3;
		unsigned int			StopBits : 2;
		unsigned int			DataBits : 3;

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
		virtual void StartPort() override
		{
			int AIndex = ((int)Parity) * 8 + ( StopBits - 1 ) * 4 + ( DataBits - 5);
			T_SERIAL->begin( inherited::Speed, CUSARTSerialInits[ AIndex ] );
		}

	public:
		USARTSerialPort() :
			Parity( spNone ),
			StopBits( 2 ),
			DataBits( 8 )
		{
		}
	};
//---------------------------------------------------------------------------
} // Mitov

#endif


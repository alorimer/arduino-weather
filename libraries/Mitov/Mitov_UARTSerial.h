////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_UART_SERIAL_h
#define _MITOV_UART_SERIAL_h

#include <Mitov.h>

namespace Mitov
{
	const	UARTClass::UARTModes CUARTSerialInits[] = 
	{
		SERIAL_8N1,
		SERIAL_8E1,
		SERIAL_8O1,
		SERIAL_8M1,
		SERIAL_8S1
	};
//---------------------------------------------------------------------------
	template<typename T_SERIAL_TYPE, T_SERIAL_TYPE *T_SERIAL> class UARTSerialPort : public Mitov::SpeedSerialPort<T_SERIAL_TYPE, T_SERIAL>
	{
		typedef	Mitov::SpeedSerialPort<T_SERIAL_TYPE, T_SERIAL> inherited;

	public:
		TArduinoSerialParity	Parity = spNone;

	public:
		void SetParity( TArduinoSerialParity AValue )
		{
            if( Parity == AValue )
                return;

            Parity = AValue;
            inherited::RestartPort();
		}

	protected:
		virtual void StartPort() override
		{
			int AIndex = ((int)Parity);
			T_SERIAL->begin( inherited::Speed, CUARTSerialInits[ AIndex ] );
		}

	};
//---------------------------------------------------------------------------
} // Mitov

#endif


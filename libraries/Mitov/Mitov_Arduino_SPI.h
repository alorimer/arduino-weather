////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_ARDUINO_SPI_h
#define _MITOV_ARDUINO_SPI_h

#include <Mitov.h>
#include <Mitov_Basic_SPI.h>

namespace Mitov
{
	class ArduinoSPI : public Mitov::BasicSPI
	{
		typedef Mitov::BasicSPI inherited;

	public:
		virtual uint16_t transfer16(uint16_t data)
		{
			return SPI.transfer16( data );
		}

		virtual uint8_t transfer(uint8_t data)
		{
			return SPI.transfer( data );
		}

		virtual void transfer(void *buf, size_t count)
		{
			SPI.transfer( buf, count );
		}

		virtual void beginTransaction(SPISettings settings)
		{
			SPI.beginTransaction( settings );
		}

		virtual void endTransaction()
		{
			SPI.endTransaction();
		}

	protected:
		virtual void SystemInit()
		{
			inherited::SystemInit();
			SPI.begin();
		}
	};
//---------------------------------------------------------------------------
}
#endif

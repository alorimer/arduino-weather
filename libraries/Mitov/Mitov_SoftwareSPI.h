////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_SOFTWARE_SPI_h
#define _MITOV_SOFTWARE_SPI_h

#include <Mitov.h>
#include <Mitov_Basic_SPI.h>

namespace Mitov
{
	template<int MOSI_PIN_NUMBER, int CLOCK_PIN_NUMBER> class VisuinoSoftwareSPI : public Mitov::BasicSPI
	{
		typedef Mitov::BasicSPI inherited;

	public:
		virtual uint16_t transfer16(uint16_t data)
		{
			shiftOut( MOSI_PIN_NUMBER, CLOCK_PIN_NUMBER, MSBFIRST, data >> 8 );
			shiftOut( MOSI_PIN_NUMBER, CLOCK_PIN_NUMBER, MSBFIRST, data );

			return 0;
		}

		virtual uint8_t transfer(uint8_t data)
		{
			shiftOut( MOSI_PIN_NUMBER, CLOCK_PIN_NUMBER, MSBFIRST, data );

			return 0;
		}

		virtual void transfer(void *buf, size_t count)
		{
			for( int i = 0; i < count; ++i )
				shiftOut( MOSI_PIN_NUMBER, CLOCK_PIN_NUMBER, MSBFIRST, ((byte *)buf)[ i ] );
		}

		virtual void beginTransaction(SPISettings settings)
		{
		}

		virtual void endTransaction()
		{
		}


	protected:
		virtual void SystemInit() 
		{
			pinMode( MOSI_PIN_NUMBER, OUTPUT );
			pinMode( CLOCK_PIN_NUMBER, OUTPUT );

			inherited::SystemInit();
		}

	};
}

#endif

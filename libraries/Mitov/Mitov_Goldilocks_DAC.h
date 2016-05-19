////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_GOLDILOCKS_DAC_h
#define _MITOV_GOLDILOCKS_DAC_h

#include <Mitov.h>
#include <DAC.h>

namespace Mitov
{
	class GoldilocksDAC : public OpenWire::Component, public Mitov::ClockingSupport
	{
		typedef OpenWire::Component inherited;

	public:
		OpenWire::VlaueSinkPin<float>	InputPins[ 2 ];

	protected:
		virtual void SystemStart() 
		{
			inherited::SystemStart();
			DAC_init(TRUE);
		}

	protected:
		void DoReceive( void *_Data )
		{
			if( ClockInputPin.IsConnected() )
				return;

			DoClockReceive( _Data );
		}

		void DoClockReceive( void *_Data )
		{
			uint16_t aValue = InputPins[ 0 ].Value * 1023 + 0.5;
			uint16_t bValue = InputPins[ 1 ].Value * 1023 + 0.5;
			DAC_out( &aValue, &bValue );
		}

	public:
		GoldilocksDAC()
		{
			InputPins[ 0 ].SetCallback( MAKE_CALLBACK( GoldilocksDAC::DoReceive ));
			InputPins[ 1 ].SetCallback( MAKE_CALLBACK( GoldilocksDAC::DoReceive ));
		}

	};
}

#endif

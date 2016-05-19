////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_BINARY_GENERATORS_h
#define _MITOV_BINARY_GENERATORS_h

#include <Mitov.h>

namespace Mitov
{
	class PulseGenerator : public Mitov::CommonGenerator
	{
		typedef Mitov::CommonGenerator inherited;

	public:
		bool	InitialValue = false;

	protected:
		virtual void CalculateFields() override
		{
			FPeriod = ( (( 1 / ( Frequency * 2 )) * 1000000 ) + 0.5 ); // 2 times one for each switch
		}

	protected:
		virtual void Clock() override
		{
			InitialValue = !InitialValue;
			OutputPin.Notify( &InitialValue );
		}
		
		virtual void SystemInit() override
		{
			inherited::SystemInit();

			OutputPin.Notify( &InitialValue );
		}

	};
//---------------------------------------------------------------------------
}

#endif

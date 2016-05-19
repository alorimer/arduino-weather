////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_DIGITALS_TO_UNSIGNED_h
#define _MITOV_DIGITALS_TO_UNSIGNED_h

#include <Mitov.h>

namespace Mitov
{
	template<int C_NUM_INPUTS> class DigitalsToUnsigned : public Mitov::BasicCommonMultiInput<bool, uint32_t, C_NUM_INPUTS>
	{
		typedef Mitov::BasicCommonMultiInput<bool, uint32_t, C_NUM_INPUTS> inherited;

	protected:
		virtual uint32_t CalculateOutput() override
		{
			uint32_t AValue = 0;
			for( uint32_t i = C_NUM_INPUTS; i--; )
				if( inherited::InputPins[ i ].Value )
					AValue |= ((uint32_t)1) << i;

			return AValue;
		}

	};
//---------------------------------------------------------------------------
}

#endif

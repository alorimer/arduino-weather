////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_PRIORITY_ENCODER_h
#define _MITOV_PRIORITY_ENCODER_h

#include <Mitov.h>

namespace Mitov
{
	template<int C_NUM_INPUTS> class PriorityEncoder : public Mitov::BasicCommonMultiInput<bool, uint32_t, C_NUM_INPUTS>
	{
		typedef Mitov::BasicCommonMultiInput<bool, uint32_t, C_NUM_INPUTS> inherited;

	protected:
		virtual uint32_t CalculateOutput() override
		{
			for( uint32_t i = C_NUM_INPUTS; i--; )
				if( inherited::InputPins[ i ].Value )
					return i;

			return 0;
		}

	};
//---------------------------------------------------------------------------
}

#endif

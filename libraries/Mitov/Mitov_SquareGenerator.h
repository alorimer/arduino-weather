////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_SQUARE_GENERATOR_h
#define _MITOV_SQUARE_GENERATOR_h

#include <Mitov.h>
#include "Mitov_BasicGenerator.h"

namespace Mitov
{
	template<typename T> class TypedSquareGenerator : public Mitov::AsymmetricGenerator<T>
	{
		typedef Mitov::AsymmetricGenerator<T> inherited;

	protected:
		virtual void CalculateValue() override
		{
			if( inherited::FPhase < 0.5 + inherited::Asymmetry / 2 )
				inherited::FValue = inherited::Offset - inherited::Amplitude;

			else
				inherited::FValue = inherited::Offset + inherited::Amplitude;

		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
}

#endif

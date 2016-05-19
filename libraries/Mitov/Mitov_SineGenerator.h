////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_SINE_GENERATOR_h
#define _MITOV_SINE_GENERATOR_h

#include <Mitov.h>
#include "Mitov_BasicGenerator.h"

namespace Mitov
{
	class SineAnalogGenerator : public Mitov::BasicFrequencyGenerator<float>
	{
		typedef Mitov::BasicFrequencyGenerator<float> inherited;

	protected:
		virtual void CalculateValue() override
		{
			FValue = sin( FPhase * 2 * PI ) * Amplitude + Offset;
		}

	public:
		SineAnalogGenerator() :
			inherited( 0.5, 0.5 )
		{
		}
	};
//---------------------------------------------------------------------------
	class SineIntegerGenerator : public Mitov::BasicFrequencyGenerator<long>
	{
		typedef Mitov::BasicFrequencyGenerator<long> inherited;

	protected:
		virtual void CalculateValue() override
		{
			FValue = sin( FPhase * 2 * PI ) * Amplitude + Offset + 0.5;
		}

	public:
		SineIntegerGenerator() :
			inherited( 1000, 0 )
		{
		}
	};
//---------------------------------------------------------------------------
	class SineUnsignedGenerator : public Mitov::BasicFrequencyGenerator<unsigned long>
	{
		typedef Mitov::BasicFrequencyGenerator<unsigned long> inherited;

	protected:
		virtual void CalculateValue() override
		{
			FValue = sin( FPhase * 2 * PI ) * Amplitude + Offset + 0.5;
		}

	public:
		SineUnsignedGenerator() :
			inherited( 1000, 1000 )
		{
		}
	};
}

#endif

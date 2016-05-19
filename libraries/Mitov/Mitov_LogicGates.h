////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_LOGIC_GATES_h
#define _MITOV_LOGIC_GATES_h

#include <Mitov.h>

namespace Mitov
{
	template<int C_NUM_INPUTS> class BooleanOr : public CommonMultiInput<bool, C_NUM_INPUTS>
	{
		typedef CommonMultiInput<bool, C_NUM_INPUTS> inherited;

	protected:
		virtual bool CalculateOutput() override
		{
			bool AValue = false;
			for ( int i = 0; i < C_NUM_INPUTS; ++i )
				AValue |= inherited::InputPins[ i ].Value;

			return AValue;
		}

	};
//---------------------------------------------------------------------------
	template<int C_NUM_INPUTS> class BooleanAnd : public CommonMultiInput<bool, C_NUM_INPUTS>
	{
		typedef CommonMultiInput<bool, C_NUM_INPUTS> inherited;

	protected:
		virtual bool CalculateOutput() override
		{
			bool AValue = (C_NUM_INPUTS > 0);
			for ( int i = 0; i < C_NUM_INPUTS; ++i )
				AValue &= inherited::InputPins[ i ].Value;

			return AValue;
		}

	};
//---------------------------------------------------------------------------
	template<int C_NUM_INPUTS> class BooleanXor : public CommonMultiInput<bool, C_NUM_INPUTS>
	{
		typedef CommonMultiInput<bool, C_NUM_INPUTS> inherited;

	protected:
		virtual bool CalculateOutput() override
		{
			bool AValue = false;
			for ( int i = 0; i < C_NUM_INPUTS; ++i )
				AValue ^= inherited::InputPins[ i ].Value;

			return AValue;
		}

	};
//---------------------------------------------------------------------------
	class BooleanInverter : public CommonTypedFilter<bool>
	{
	protected:
		virtual bool FilterValue(bool AValue) override
		{
			return ! AValue;
		}
	};
//---------------------------------------------------------------------------

}

#endif

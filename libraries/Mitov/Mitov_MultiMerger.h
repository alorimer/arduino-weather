////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_MULTI_MERGER_h
#define _MITOV_MULTI_MERGER_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	template<int C_NUM_INPUTS> class MultiMerger : public Mitov::CommonSource
	{
		typedef Mitov::CommonSource inherited;

	public:
		OpenWire::SinkPin InputPins[ C_NUM_INPUTS ];

	protected:
		void DoReceive( void *_Data )
		{
			OutputPin.Notify( _Data );
		}

	protected:
		virtual void SystemInit()
		{
			inherited::SystemInit();

			for( int i = 0; i < C_NUM_INPUTS; ++i )
				InputPins[ i ].SetCallback( this, (OpenWire::TOnPinReceive)&MultiMerger::DoReceive );
		}

	};
}

#endif

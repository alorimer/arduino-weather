////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_BASIC_DISPLAY_h
#define _MITOV_BASIC_DISPLAY_h

#include <Mitov.h>

namespace Mitov
{
	template<typename T> class BasicDisplay : public Mitov::CommonSink
	{
		typedef	OpenWire::Component	inherited;

	protected:
		T		FValue;

	public:
		bool	Enabled = true;
		T		InitialValue = 0;

	protected:
		virtual void UpdateDisplay() = 0;

	protected:
		virtual void DoReceive( void *_Data )
		{
			FValue = *(T*)_Data;
		}

	public:
		void SetEnabled( bool AValue )
		{
			if( Enabled == AValue )
				return;

			Enabled = AValue;
			UpdateDisplay();
		}

		void SetInitialValue( T AValue )
		{
			if( InitialValue == AValue )
				return;

			InitialValue = AValue;
			UpdateDisplay();
		}

	protected:
		virtual void SystemInit()
		{
			FValue = InitialValue;
			inherited::SystemInit();
			UpdateDisplay();
		}

	};
//---------------------------------------------------------------------------
}

#endif

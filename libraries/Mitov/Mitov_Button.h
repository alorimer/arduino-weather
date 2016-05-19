////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_BUTTON_h
#define _MITOV_BUTTON_h

#include <Mitov.h>

namespace Mitov
{
	class Button : public Mitov::CommonFilter
	{
		typedef Mitov::CommonFilter inherited;

	protected:
		unsigned long	FLastTime = 0;
		bool	FLastValue = false;
		bool	FValue = false;

	public:
		uint32_t	DebounceInterval = 50;

	protected:
		virtual void DoReceive( void *_Data )
		{
			bool AValue = *( bool *)_Data;
			if( AValue != FLastValue )
				FLastTime = millis();

			FLastValue = AValue;
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			if( FValue != FLastValue )
				if( millis() - FLastTime > DebounceInterval )
				{
					FValue = FLastValue;
					OutputPin.Notify( &FValue );
				}

			inherited::SystemLoopBegin( currentMicros );
		}

	};
}

#endif

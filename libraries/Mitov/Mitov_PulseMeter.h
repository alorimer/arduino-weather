////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_PULSE_METER_h
#define _MITOV_PULSE_METER_h

#include <Mitov.h>

namespace Mitov
{
	class PulseMeter : public Mitov::CommonEnableFilter
	{
		typedef Mitov::CommonEnableFilter inherited;

	protected:
		unsigned long	FStartTime = 0;
		bool			FOldValue = false;

	protected:
		virtual void DoReceive( void *_Data )
		{
            if( ! Enabled )
				return;

			bool AValue = *(bool *)_Data;
			if( FOldValue == AValue )
				return;

			unsigned long ANow = micros();
			FOldValue = AValue;
			if( AValue )
			{
				FStartTime = ANow;
				return;
			}

			int APeriod = ANow - FStartTime;

			OutputPin.Notify( &APeriod );
		}

	};
}

#endif

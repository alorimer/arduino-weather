////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_RC_SERVO_METER_h
#define _MITOV_RC_SERVO_METER_h

#include <Mitov.h>

namespace Mitov
{
	class RCServoMeter : public Mitov::CommonEnableFilter
	{
		typedef Mitov::CommonEnableFilter inherited;

	protected:
		unsigned long	FStartTime = 0;
		bool			FOldValue = false;

	protected:
		virtual void DoReceive( void *_Data ) override
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

			float APeriod = ANow - FStartTime;
			float AFloatValue = MitovMin( ( APeriod - 1000 ) / 1000, 1 );

			OutputPin.Notify( &AFloatValue );
		}

	};
}

#endif

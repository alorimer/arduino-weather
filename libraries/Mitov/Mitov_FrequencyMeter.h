////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_FREQUENCY_METER_h
#define _MITOV_FREQUENCY_METER_h

#include <Mitov.h>

namespace Mitov
{
	class FrequencyMeter : public Mitov::CommonEnableFilter
	{
		typedef Mitov::CommonFilter inherited;

	protected:
		unsigned long	FStartTime = 0;
		bool			FFirstTime = true;

	protected:
		virtual void DoReceive( void *_Data )
		{
			if( FFirstTime )
			{
				FFirstTime = false;
				FStartTime = micros();
				return;
			}

            if( ! Enabled )
				return;

			unsigned long ANow = micros();
			unsigned long APeriod = ANow - FStartTime;
			FStartTime = ANow;

			if( APeriod == 0 )
				APeriod = 1;

			float AFrequency =  1000000.0f / APeriod;
			OutputPin.Notify( &AFrequency );
		}

	};
}

#endif

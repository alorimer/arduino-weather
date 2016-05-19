////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_AMPLITUDE_METER_h
#define _MITOV_AMPLITUDE_METER_h

#include <Mitov.h>

namespace Mitov
{
	const float FLT_MAX = 3.4028235E+38;

	class AmplitudeMeter : public Mitov::CommonEnableFilter
	{
		typedef Mitov::CommonEnableFilter inherited;

	public:
		OpenWire::SourcePin	MinOutputPin;
		OpenWire::SourcePin	MaxOutputPin;

	protected:
		unsigned long	FStartTime = 0;
		double			FAccumulator = 0.0f;
		float			FMin = 0;
		float			FMax = 0;
		unsigned long	FCount = 0;

	public:
		long	Period = 100;

	protected:
		virtual void DoReceive( void *_Data )
		{
            if( ! Enabled )
				return;

			float AValue = *(float *)_Data;
			FAccumulator += AValue;
			if( FCount == 0 )
			{
				FMin = AValue;
				FMax = AValue;
			}

			else
			{
				if( AValue < FMin )
					FMin = AValue;

				if( AValue > FMax )
					FMax = AValue;

			}

			++FCount;

			unsigned long ANow = micros();
			unsigned long APeriod = ANow - FStartTime;

			if( APeriod < ((unsigned long)Period) * 1000 )
				return;

			FStartTime = ANow;
			if( FCount == 0 )
				--FCount;

			float AAverage = FAccumulator / FCount;
			FAccumulator = 0;
			FCount = 0;

			OutputPin.Notify( &AAverage );
			MinOutputPin.Notify( &FMin );
			MaxOutputPin.Notify( &FMax );
		}

		virtual void SystemInit()
		{
			FAccumulator = 0;
			FCount = 0;

			inherited::SystemInit();
		}

	};
}

#endif

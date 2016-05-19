////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_RAMP_TO_VALUE_h
#define _MITOV_RAMP_TO_VALUE_h

#include <Mitov.h>

namespace Mitov
{
	class RampToValue : public Mitov::CommonEnableFilter, public Mitov::ClockingSupport
	{
		typedef Mitov::CommonEnableFilter inherited;

	public:
		float	InitialValue = 0.0f;
		float	Slope = 1.0f;

	protected:
		unsigned long	FLastTime = 0;
		float			FCurrentValue = 0.0f;
		float			FTargetValue = 0.0f;

	protected:
		virtual void DoReceive( void *_Data )
		{
			bool AValue = *(float *)_Data;
			if( FTargetValue == AValue )
				return;

			FTargetValue = AValue;
//			FStartTime = micros();
		}

		virtual void DoClockReceive( void *_Data ) override
		{
			Generate( micros(), true );
		}

		void Generate( unsigned long currentMicros, bool FromClock )
		{
			if( FCurrentValue != FTargetValue )
			{
				if( inherited::Enabled )
				{
					float ARamp = abs( ( currentMicros - FLastTime ) * Slope / 1000000 );
					if( FCurrentValue < FTargetValue )
					{
						FCurrentValue += ARamp;
						if( FCurrentValue > FTargetValue )
							FCurrentValue = FTargetValue;

					}
					else
					{
						FCurrentValue -= ARamp;
						if( FCurrentValue < FTargetValue )
							FCurrentValue = FTargetValue;

					}

					inherited::OutputPin.Notify( &FCurrentValue );
				}
			}

			else if( FromClock )
				inherited::OutputPin.Notify( &FCurrentValue );

			FLastTime = currentMicros;
//			inherited::SendOutput();
		}

	protected:
		virtual void SystemStart() override
		{
			FCurrentValue = InitialValue;
			FTargetValue = InitialValue;

			inherited::OutputPin.Notify( &FCurrentValue );
			inherited::SystemStart();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			if( ! ClockInputPin.IsConnected() )
				Generate( currentMicros, false );

			inherited::SystemLoopBegin( currentMicros );
		}

	};
}

#endif

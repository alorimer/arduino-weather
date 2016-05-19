////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_SPEED_TO_CLOCK_h
#define _MITOV_SPEED_TO_CLOCK_h

#include <Mitov.h>

namespace Mitov
{
	class SpeedToClock : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink inherited;

	public:
		OpenWire::SourcePin	DirectionOutputPin;
		OpenWire::SourcePin	ClockOutputPin;

	public:
		bool	Enabled = true;
		float	MaxFrequency = 100.0f;

	protected:
		unsigned long FLastTime = 0;
		unsigned long FPeriod = 0;
//		bool		  FDirection;
		bool          FSendOutput = false;

	protected:
/*
		virtual void CalculateFields()
		{
			FPeriod = ( (( 1 / Frequency ) * 1000000 ) + 0.5 );
		}
*/
		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			if( ! FSendOutput )
				FLastTime = currentMicros;

			else
				while( currentMicros - FLastTime >= FPeriod )
				{
					if( Enabled )
						ClockOutputPin.Notify( NULL );

					FLastTime += FPeriod;
				}

			inherited::SystemLoopBegin( currentMicros );
		}

		virtual void SystemStart() override
		{
			inherited::SystemStart();
			FLastTime = micros();
		}

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			float ASpeed = constrain( *(float *)_Data, 0.0, 1.0 );
			float AFrequency = abs( ASpeed - 0.5 ) * 2;
			FSendOutput = ( AFrequency != 0 );
			if( FSendOutput )
			{
				AFrequency *= MaxFrequency;
				FPeriod = ( (( 1 / AFrequency ) * 1000000 ) + 0.5 );
			}

			else
				FPeriod = 0;

			bool ADirection = ASpeed > 0.5;

			DirectionOutputPin.Notify( &ADirection );
		}

	};
}

#endif

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_GAUSS_GENERATOR_h
#define _MITOV_GAUSS_GENERATOR_h

#include <Mitov.h>
#include "Mitov_BasicGenerator.h"

namespace Mitov
{
	template<typename T> class CommonGaussGenerator : public Mitov::BasicGenerator<T>
	{
		typedef Mitov::BasicGenerator<T> inherited;

	public:
		T	Mean;
		T	StandardDeviation;
		long	Seed = 0;

	protected:
		uint8_t	FIndex = 0;
		T FRandoms[ 2 ];

	protected:
		virtual void SystemStart()
		{
			randomSeed( Seed );

			inherited::SystemStart();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			if( ! inherited::ClockInputPin.IsConnected() )
				Generate();

			inherited::SystemLoopBegin( currentMicros );
		}

	protected:
		void Generate()
		{
            if( inherited::Enabled )
            {
				if( ! FIndex )
				{
					float W;
					float X1;
					float X2;
					do
					{
						X1 = random( -1147483648, 1147483647 ) / 1147483647.0;
						X2 = random( -1147483648, 1147483647 ) / 1147483647.0;
						W = X1 * X1 + X2 * X2;
					}
					while(  W >= 1.0 );

					W = StandardDeviation * sqrt( (-2.0 * log( W ) ) / W );
					Serial.println( W );
					Serial.println( Mean );
					Serial.println( X1 );
					Serial.print( "RESULT1: " ); Serial.println( Mean + X1 * W );
					Serial.println( X2 );
					Serial.print( "RESULT2: " ); Serial.println( Mean + X2 * W );
					FRandoms[ 0 ] = Mean + X1 * W;
					FRandoms[ 1 ] = Mean + X2 * W;
				}

				inherited::FValue = FRandoms[ FIndex ];
				FIndex = FIndex ^ 1;
			}

			inherited::SendOutput();
		}

		virtual void DoClockReceive( void *_Data )
		{
			Generate();
		}

	public:
		CommonGaussGenerator( T AMean, T AStandardDeviation ) :
			Mean( AMean ),
			StandardDeviation( AStandardDeviation )
		{
		}

	};
//---------------------------------------------------------------------------
}

#endif

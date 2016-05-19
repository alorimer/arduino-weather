////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_RANDOM_GENERATOR_h
#define _MITOV_RANDOM_GENERATOR_h

#include <Mitov.h>
#include "Mitov_BasicGenerator.h"

namespace Mitov
{
#define Min Min
#define Max Max
	template<typename T> class CommonRandomGenerator : public Mitov::BasicGenerator<T>
	{
		typedef Mitov::BasicGenerator<T> inherited;

	public:
		T	Min;
		T	Max;
		long	Seed = 0;

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
		virtual void GenerateValue() = 0;

		void Generate()
		{
            if( inherited::Enabled )
            {
				if( Min == Max )
					inherited::FValue = Min;

				else
					GenerateValue();
			}

			inherited::SendOutput();
		}

		virtual void DoClockReceive( void *_Data )
		{
			Generate();
		}

	public:
		CommonRandomGenerator( T AMin, T AMax ) :
			Min( AMin ),
			Max( AMax )
		{
		}

	};
//---------------------------------------------------------------------------
	class RandomAnalogGenerator : public Mitov::CommonRandomGenerator<float>
	{
		typedef Mitov::CommonRandomGenerator<float> inherited;

	protected:
		virtual void GenerateValue()
		{
			float AMin = MitovMin( Min, Max );
			float AMax = MitovMax( Min, Max );
//			double ARandom = random( -2147483648, 2147483647 );
			double ARandom = random( -1147483648, 1147483647 );
//			FValue = ARandom;
			ARandom += 1147483648;
			FValue = AMin + ( ARandom / ( (double)1147483647 + (double)1147483648 )) * (AMax - AMin);
		}

	public:
		RandomAnalogGenerator() :
			inherited( 0, 1 )
		{
		}

	};
//---------------------------------------------------------------------------
	class RandomIntegerGenerator : public Mitov::CommonRandomGenerator<long>
	{
		typedef Mitov::CommonRandomGenerator<long> inherited;

	protected:
		virtual void GenerateValue()
		{
			long AMin = MitovMin( Min, Max );
			long AMax = MitovMax( Min, Max );
			FValue = random( AMin, AMax + 1 );
		}

	public:
		RandomIntegerGenerator() :
			inherited( -1000, 1000 )
		{
		}

	};
//---------------------------------------------------------------------------
	class RandomUnsignedGenerator : public Mitov::CommonRandomGenerator<unsigned long>
	{
		typedef Mitov::CommonRandomGenerator<unsigned long> inherited;

	protected:
		virtual void GenerateValue()
		{
			unsigned long AMin = MitovMin( Min, Max );
			unsigned long AMax = MitovMax( Min, Max );
			FValue = random( AMin, AMax + 1 );
		}

	public:
		RandomUnsignedGenerator() :
			inherited( 0, 1000 )
		{
		}

	};
//---------------------------------------------------------------------------
	class RandomDateTimeGenerator : public Mitov::CommonRandomGenerator<Mitov::TDateTime>
	{
		typedef Mitov::CommonRandomGenerator<Mitov::TDateTime> inherited;

	protected:
		virtual void GenerateValue()
		{
			uint32_t ARandomDate = random( Min.Date, Max.Date );
			uint32_t ARandomTime;
			if( ARandomDate == Min.Date )
				ARandomTime = random( Min.Time, MSecsPerDay );

			else if( ARandomDate == Max.Date )
				ARandomTime = random( 0, Max.Time );

			else
				ARandomTime = random( 0, MSecsPerDay );

			FValue.Date = ARandomDate;
			FValue.Time = ARandomTime;
/*
			float AMin = MitovMin( Min.Value, Max.Value );
			float AMax = MitovMax( Min.Value, Max.Value );
//			double ARandom = random( -2147483648, 2147483647 );
			double ARandom = random( -1147483648, 1147483647 );
//			FValue = ARandom;
			ARandom += 1147483648;
			FValue.Value = AMin + ( ARandom / ( (double)1147483647 + (double)1147483648 )) * (AMax - AMin);
*/
		}

	public:
		RandomDateTimeGenerator() :
			inherited( Mitov::TDateTime( 693594, 0 ), Mitov::TDateTime( 693694, 0 ) )
		{
//			Min(  )
		}

	};
//---------------------------------------------------------------------------
#undef Min
#undef Max
}

#endif

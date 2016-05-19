////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_BASIC_GENERATOR_h
#define _MITOV_BASIC_GENERATOR_h

#include <Mitov.h>

namespace Mitov
{
	template<typename T> class BasicGenerator : public Mitov::CommonEnableSource, public Mitov::ClockingSupport
	{
		typedef Mitov::CommonSource inherited;

	public: // Needs to be public due to compiler bug :-(
		T	FValue = T( 0 );

	protected:
		inline void SendOutput() // Needed to be due to compiler bug :-(
		{
			 OutputPin.Notify( &FValue );
		}

	};
//---------------------------------------------------------------------------
	template<typename T> class BasicFrequencyGenerator : public Mitov::BasicGenerator<T>
	{
		typedef Mitov::BasicGenerator<T> inherited;

	public:
		float	Frequency = 1.0f;
		T	Amplitude;
		T	Offset;

		// 0 - 1
		float	Phase = 0.0f;

	public:
		void SetFrequency( float AValue )
		{
			if( Frequency == AValue )
				return;

			Frequency = AValue;
			inherited::CalculateFields();
		}

		void SetPhase( float AValue )
		{
			if( Phase == AValue )
				return;

			Phase = AValue;
			FPhase = Phase;
		}

	protected:
		float	FPhase = 0.0f;
		unsigned long FLastTime = 0;
		
	protected:
		virtual void SystemStart()
		{
			FPhase = Phase;

			inherited::SystemStart();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			if( ! inherited::ClockInputPin.IsConnected() )
				Generate( currentMicros );

			inherited::SystemLoopBegin( currentMicros );
		}

		void Generate( unsigned long currentMicros )
		{
			if( inherited::Enabled )
			{
				float APeriod = 1000000 / Frequency;

				float ATime = ( currentMicros - FLastTime );
				ATime /= APeriod;
				FPhase += ATime;
				FPhase = V_FMOD( FPhase, 1 );

				CalculateValue();
			}

			FLastTime = currentMicros;
			inherited::SendOutput();
		}

	protected:
		virtual void CalculateValue() = 0;

		virtual void DoClockReceive( void *_Data )
		{
			Generate( micros() );
		}

	public:
		BasicFrequencyGenerator( T AAmplitude, T AOffset ) :
			Amplitude( AAmplitude ),
			Offset( AOffset )
		{
		}

	};
//---------------------------------------------------------------------------
	template<typename T> class AsymmetricGenerator : public Mitov::BasicFrequencyGenerator<T>
	{
		typedef Mitov::BasicFrequencyGenerator<T> inherited;

	public:
		// -1 - 1
		float	Asymmetry = 0.0f;

	public:
		using inherited::inherited;

	};
}

#endif

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_PID_CONTROLLER_h
#define _MITOV_PID_CONTROLLER_h

#include <Mitov.h>

namespace Mitov
{
	class PIDController : public Mitov::CommonEnableFilter, public Mitov::ClockingSupport
	{
		typedef Mitov::CommonEnableFilter inherited;

	public:
		OpenWire::SinkPin	ManualControlInputPin;

	public:
		float ProportionalGain = 0.1f;
		float IntegralGain = 0.1f;
		float DerivativeGain = 0.1f;

		float SetPoint = 0.0f;

		float InitialValue = 0.0f;

	protected:
		unsigned long	FLastTime;
		float	FOutput = 0.0f;
		float	FInput;
		float	FLastInput;
		float	FITerm;

	public:
		void SetEnabled( bool AValue )
		{
            if( Enabled == AValue )
                return;

            Enabled = AValue;
			if( Enabled )
				Initialize();

		}

	protected:
		virtual void DoReceive( void *_Data )
		{
			FInput = *(float *)_Data;
		}

		void DoManualControlReceive( void *_Data )
		{
			if( Enabled )
				return;

			FOutput = *(float *)_Data;
		}

		virtual void DoClockReceive( void *_Data ) override
		{
			OutputPin.Notify( &FOutput );
		}

	protected:
		void Initialize()
		{
			FITerm = FOutput;
			FLastInput = FInput;
			FLastTime = micros();
		}

	protected:
		virtual void SystemStart()
		{
			FInput = InitialValue;
			Initialize();
			inherited::SystemStart();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			inherited::SystemLoopBegin( currentMicros );
			if( ! Enabled ) 
				return;

			unsigned long timeChange = ( currentMicros - FLastTime );
			float ANormalizedTime = timeChange / 1000000;

			// Compute all the working error variables
			double error = SetPoint - FInput;
//			ITerm += ( ki * error ) * ANormalizedTime;
			FITerm = constrain( FITerm + ( IntegralGain * error ) * ANormalizedTime, 0, 1 );

			double dInput = ( FInput - FLastInput ) * ANormalizedTime;
 
			// Compute PID Output
			float AOutput = constrain( ProportionalGain * error + FITerm - DerivativeGain * dInput, 0, 1 );
	  
			// Remember some variables for next time
			FLastInput = FInput;
			FLastTime = currentMicros;

			if( AOutput == FOutput )
				return;

			if( ClockInputPin.IsConnected() )
				return;

			OutputPin.Notify( &FOutput );
		}

	public:
		PIDController()
		{
			ManualControlInputPin.SetCallback( MAKE_CALLBACK( PIDController::DoManualControlReceive ));
		}

	};
//---------------------------------------------------------------------------
}

#endif

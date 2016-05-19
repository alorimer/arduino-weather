////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_STEERING_DIFFERENTIAL_h
#define _MITOV_STEERING_DIFFERENTIAL_h

#include <Mitov.h>

namespace Mitov
{
	class SteeringDifferential : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		OpenWire::SinkPin	SteeringInputPins[ 2 ];
		OpenWire::SourcePin	MotorsOutputPins[ 2 ];

	protected:
		float FDirection = 0.5f;
		float FSpeed = 0.5f;

	protected:
		void UpdateMotors()
		{
			float	ADirOffset = FDirection - 0.5;

			float	ALeftMotor = FSpeed + ADirOffset;
			float	ARightMotor = FSpeed - ADirOffset;

			if( ALeftMotor > 1.0 )
			{
				ARightMotor -= ( ALeftMotor - 1 );
				ALeftMotor = 1.0;
			}

			else if( ALeftMotor < 0.0 )
			{
				ARightMotor -= ALeftMotor;
				ALeftMotor = 0.0;
			}

			else if( ARightMotor > 1.0 )
			{
				ALeftMotor -= ( ARightMotor - 1 );
				ARightMotor = 1.0;
			}

			else if( ARightMotor < 0.0 )
			{
				ALeftMotor -= ARightMotor;
				ARightMotor = 0.0;
			}

			MotorsOutputPins[ 0 ].Notify( &ALeftMotor );
			MotorsOutputPins[ 1 ].Notify( &ARightMotor );
		}

	protected:
		void DoDirectionReceive( void *_Data )
		{
			float AValue = constrain( *(float *)_Data, 0, 1 );
			if( FDirection == AValue )
				return;

			FDirection = AValue;
			UpdateMotors();
		}

		void DoSpeedReceive( void *_Data )
		{
			float AValue = constrain( *(float *)_Data, 0, 1 );
			if( FSpeed == AValue )
				return;

			FSpeed = AValue;
			UpdateMotors();
		}

	public:
		SteeringDifferential()
		{
			SteeringInputPins[ 0 ].SetCallback( this, (OpenWire::TOnPinReceive)&SteeringDifferential::DoDirectionReceive );
			SteeringInputPins[ 1 ].SetCallback( this, (OpenWire::TOnPinReceive)&SteeringDifferential::DoSpeedReceive );
		}

	};
//---------------------------------------------------------------------------
}

#endif

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_SERVO_h
#define _MITOV_SERVO_h

#include <Mitov.h>

#include <Servo.h>

namespace Mitov
{
	template<int PIN_NUMBER> class MitovServo : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink inherited;

	protected:
		Servo	FServo;
		
	protected:
		virtual void DoReceive( void *_Data ) override
		{
			float AValue = constrain( *((float *)_Data), 0.0f, 1.0f ) * 180;
			FServo.write( AValue );
		}

		virtual void SystemStart() override
		{
			FServo.attach( PIN_NUMBER );
			inherited::SystemStart();
		}

	};
}

#endif

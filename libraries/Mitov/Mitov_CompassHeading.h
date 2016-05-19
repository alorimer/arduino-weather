////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_COMPASSHEADING_h
#define _MITOV_COMPASSHEADING_h

#include <Mitov.h>

namespace Mitov
{
	enum TAngleUnits { auDegree, auRadians, auNormalized };

	class CompassHeading : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		OpenWire::VlaueSinkPin<float>	InputPins[ 2 ];
		OpenWire::TypedSourcePin<float>	OutputPin;

	public:
		float	DeclinationAngle = 0.0;
		TAngleUnits	Units = auDegree;

	protected:
		bool	FChangeOnly = false;

	protected:
		virtual void SystemLoopEnd() 
		{
			inherited::SystemLoopEnd();

			// Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
			// Calculate heading when the magnetometer is level, then correct for signs of axis.
			float AHeading = atan2( InputPins[ 1 ].Value, InputPins[ 0 ].Value ); // Y , X

//			Serial.println( AHeading );

			// Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
			// Find yours here: http://www.magnetic-declination.com/
			// If you cannot find your Declination, comment out these two lines, your compass will be slightly off.

			float	ADeclinationAngle;
			switch( Units )
			{
				case auDegree:
					ADeclinationAngle = DeclinationAngle * M_PI / 180;
					break;

				case auRadians:
					ADeclinationAngle = DeclinationAngle;
					break;

				case auNormalized:
					ADeclinationAngle = DeclinationAngle * 2 * PI;
					break;

			}

			AHeading += ADeclinationAngle;

			// Correct for when signs are reversed.
			if(AHeading < 0)
				AHeading += 2*PI;
    
			// Check for wrap due to addition of declination.
			if(AHeading > 2*PI)
				AHeading -= 2*PI;

			switch( Units )
			{
				case auDegree:
					AHeading *= 180 / M_PI;
					break;

				case auNormalized:
					AHeading /= 2 * PI;
					break;

			}

			OutputPin.SetValue( AHeading, FChangeOnly );

			FChangeOnly = true;
		}

	};
}

#endif

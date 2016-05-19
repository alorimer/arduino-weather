////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_PS2_CONTROLLER_h
#define _MITOV_PS2_CONTROLLER_h

#include <Mitov.h>
#include <Mitov_PS2_Controller_Basic.h>
#include <PS2X_lib.h>

namespace Mitov
{
	class PS2BasicControllerDirect : public Mitov::PS2BasicController
	{
		typedef Mitov::PS2BasicController inherited;

	protected:
		PS2X Fps2x;

	public:
		virtual	bool	ReadDigital( unsigned int AIndex )
		{
			return Fps2x.Button( AIndex );
		}

		virtual	float	ReadAnalog( unsigned int AIndex )
		{
			return ((float)Fps2x.Analog( AIndex )) / 255;
		}

	public:
		PS2BasicControllerDirect( int ADataPinNumber, int ACommandPinNumber, int AAttentionPinNumber, int AClockPinNumber )
		{
			Fps2x.config_gamepad( AClockPinNumber, ACommandPinNumber, AAttentionPinNumber, ADataPinNumber, true, true );
		}

	};
//---------------------------------------------------------------------------
	class PS2Controller : public PS2BasicControllerDirect
	{
		typedef PS2BasicControllerDirect inherited;

	public:
		OpenWire::SinkPin	SmallVibrateMotorInputPin;
		OpenWire::SinkPin	LargeVibrateMotorInputPin;

	protected:
		bool	FSmallMotor = false;
		int8_t	FLargeMotor = 0;

	protected:
		void DoSmallVibrateMotorReceive( void *_Data )
		{
			FSmallMotor = *(bool *)_Data;
		}

		void DoLargeVibrateMotorReceive( void *_Data )
		{
			FLargeMotor = (int8_t)( constrain( *(float *)_Data, 0, 1 ) * 255 + 0.5 );
		}

		virtual void ReadController() override
		{
			Fps2x.read_gamepad( FSmallMotor, FLargeMotor );
		}

	public:
		PS2Controller( int ADataPinNumber, int ACommandPinNumber, int AAttentionPinNumber, int AClockPinNumber ) :
			inherited( ADataPinNumber, ACommandPinNumber, AAttentionPinNumber, AClockPinNumber )
		{
			SmallVibrateMotorInputPin.SetCallback( MAKE_CALLBACK( PS2Controller::DoSmallVibrateMotorReceive ));
			LargeVibrateMotorInputPin.SetCallback( MAKE_CALLBACK( PS2Controller::DoLargeVibrateMotorReceive ));
		}

	};
//---------------------------------------------------------------------------
	class PS2Guitar : public PS2BasicControllerDirect
	{
		typedef PS2BasicControllerDirect inherited;

		virtual void ReadController() override
		{
			Fps2x.read_gamepad( false, 0 );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
}

#endif

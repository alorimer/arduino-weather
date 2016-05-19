////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_PS2_CONTROLLER_SERIAL_h
#define _MITOV_PS2_CONTROLLER_SERIAL_h

#include <Mitov.h>
#include <Mitov_PS2_Controller_Basic.h>

namespace Mitov
{
	class PS2BasicControllerSerial : public Mitov::PS2BasicController
	{
		typedef Mitov::PS2BasicController inherited;

	protected:
		Mitov::BasicSerialPort &FSerial;

	protected:
		void WriteByte( byte AValue )
		{
			while( FSerial.GetStream().available() > 0 )
				Serial.read();

			FSerial.GetStream().write( AValue );
			FSerial.GetStream().flush(); 	//wait for all data transmitted
		}

		byte ReadByte()
		{
			long waitcount=0; 

			while(true)
			{
				if( FSerial.GetStream().available() > 0 )
				{
					byte rec_data = FSerial.GetStream().read();
//					SERIAL_ERR=false; 
					return(rec_data); 
				}
				waitcount++; 
				if(waitcount>50000)
				{
//					SERIAL_ERR=true; 
					return (0xFF); 
				}
			
			}	
		}

	public:
		virtual	bool	ReadDigital( unsigned int AIndex ) override
		{
			WriteByte( AIndex );
			return ( ReadByte() == 0 );
		}

		virtual	float	ReadAnalog( unsigned int AIndex ) override
		{
			WriteByte( AIndex );
			return ((float)ReadByte()) / 255;
//			return ((float)Fps2x.Analog( AIndex )) / 255;
		}

	public:
		PS2BasicControllerSerial( Mitov::BasicSerialPort &ASerial ) :
			FSerial( ASerial )
		{
		}

	};
//---------------------------------------------------------------------------
	class PS2ControllerSerial : public PS2BasicControllerSerial
	{
		typedef PS2BasicControllerSerial inherited;

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

		virtual void ReadController()
		{
			WriteByte( 29 ); // Small Motor
			if( FSmallMotor )
				WriteByte( 1 );

			else
				WriteByte( 0 );

			WriteByte( 30 ); // Large Motor
			WriteByte( FLargeMotor );
		}

	public:
		PS2ControllerSerial( Mitov::BasicSerialPort &ASerial ) :
			inherited( ASerial )
		{
			SmallVibrateMotorInputPin.SetCallback( MAKE_CALLBACK( PS2ControllerSerial::DoSmallVibrateMotorReceive ));
			LargeVibrateMotorInputPin.SetCallback( MAKE_CALLBACK( PS2ControllerSerial::DoLargeVibrateMotorReceive ));
		}

	};
//---------------------------------------------------------------------------
/*
	class PS2GuitarSerial : public PS2BasicControllerSerial
	{
		typedef PS2BasicControllerSerial inherited;

		virtual void ReadController()
		{
			Fps2x.read_gamepad( false, 0 );
		}

	public:
		PS2GuitarSerial( int ADataPinNumber, int ACommandPinNumber, int AAttentionPinNumber, int AClockPinNumber ) :
			inherited( ADataPinNumber, ACommandPinNumber, AAttentionPinNumber, AClockPinNumber )
		{
		}
	};
*/
//---------------------------------------------------------------------------
}

#endif

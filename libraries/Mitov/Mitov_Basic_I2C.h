////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_BASIC_I2C_h
#define _MITOV_BASIC_I2C_h

#include <Mitov.h>
#include <Wire.h>

namespace Mitov
{
//	class I2C : public OpenWire::Component
	namespace I2C
	{
		const uint16_t	I2C_DEFAULT_READ_TIMEOUT	= 1000;
//	public:
		bool ReadBytes( uint8_t devAddr, uint8_t regAddr, uint8_t length, void *data, uint16_t timeout = I2C_DEFAULT_READ_TIMEOUT )
		{
			int8_t count = 0;
			uint32_t t1 = millis();

            // Arduino v1.0.1+, Wire library
            // Adds official support for repeated start condition, yay!

            // I2C/TWI subsystem uses internal buffer that breaks with large data requests
            // so if user requests more than BUFFER_LENGTH bytes, we have to do it in
            // smaller chunks instead of all at once
            for (uint8_t k = 0; k < length; k += min(length, BUFFER_LENGTH)) 
			{
                Wire.beginTransmission(devAddr);
                Wire.write(regAddr);
                Wire.endTransmission();
                Wire.beginTransmission(devAddr);
                Wire.requestFrom(devAddr, (uint8_t)min(length - k, BUFFER_LENGTH));
        
                for (; Wire.available() && (timeout == 0 || millis() - t1 < timeout); count++)
                    ((uint8_t *)data )[count] = Wire.read();
        
                Wire.endTransmission();
            }

			return ( count == length );
		}

		void WriteByte( uint8_t devAddr, uint8_t regAddr, uint8_t AValue )
		{
//			Serial.print( "Address: " ); Serial.print( devAddr, HEX ); Serial.print( " Reg: " ); Serial.print( regAddr, HEX );  Serial.print( " = " ); Serial.println( AValue, BIN );
			Wire.beginTransmission( devAddr );
			Wire.write( regAddr );
			Wire.write( AValue );
			Wire.endTransmission();
		}
	};
//---------------------------------------------------------------------------
	class Basic_I2CChannel;
//---------------------------------------------------------------------------
	class Basic_MultiChannel_SourceI2C : public Mitov::EnabledComponent, public ClockingSupport
	{
		typedef Mitov::EnabledComponent inherited;

	public:
		bool	FModified = false;

	public:
		Mitov::SimpleList<Basic_I2CChannel *>	FChannels;

//	protected:
//		virtual void SystemInit();

//	protected:
//		virtual void DoClockReceive( void * );

	};
//---------------------------------------------------------------------------
	class Basic_I2CChannel : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	public:
		float	FValue = 0.0f;
		float	FNewValue = 0.0f;

//	public:
//		virtual void InitChannel() {}
//		virtual void SendOutput() = 0;

	};
//---------------------------------------------------------------------------
	template<typename T_OWNER> class Basic_Typed_I2CChannel : public Mitov::Basic_I2CChannel
	{
		typedef Mitov::Basic_I2CChannel	inherited;

	public:
		float	InitialValue = 0.0f;

	protected:
		int		FIndex;

	protected:
		T_OWNER	&FOwner;

/*
	protected:
		virtual void DoReceive( void *_Data )
		{
			FNewValue = constrain( *((float *)_Data), 0, 1 );
			if( FNewValue == FValue )
				return;

			FOwner.FModified = true;

			if( FOwner.ClockInputPin.IsConnected() )
				FOwner.FModified = true;

			else
				SendOutput();

		}
*/

	public:
		Basic_Typed_I2CChannel( T_OWNER &AOwner, int AIndex ) :
			FOwner( AOwner ),
			FIndex( AIndex )
		{
			AOwner.FChannels.push_back( this );
		}

	};
//---------------------------------------------------------------------------
/*
	void Basic_MultiChannel_SourceI2C::DoClockReceive( void * )
	{
		if( ! FModified )
			return;

		for( int i =0; i < FChannels.size(); ++i )
			FChannels[ i ]->SendOutput();
	}
*/
//---------------------------------------------------------------------------
/*
	void Basic_MultiChannel_SourceI2C::SystemInit()
	{
		inherited::SystemInit();

		for( int i =0; i < FChannels.size(); ++i )
			FChannels[ i ]->InitChannel();
	}
*/
//---------------------------------------------------------------------------
}
#endif

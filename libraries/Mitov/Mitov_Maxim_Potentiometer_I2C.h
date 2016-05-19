////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_MAXIM_POTENTIOMETER_I2C_h
#define _MITOV_MAXIM_POTENTIOMETER_I2C_h

#include <Mitov.h>
#include <Mitov_Basic_I2C.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class PotentiometerSPIChannel;
//---------------------------------------------------------------------------
	class Maxim_Potentiometer_I2C : public Mitov::Basic_MultiChannel_SourceI2C
	{
		typedef Mitov::Basic_MultiChannel_SourceI2C inherited;

	public:
		byte	Address = 0;

	protected:
		void TransmitAt( byte ARegister, byte Adata )
		{
			byte AAddress = 0b0101000 | (( Address & 0b111 ) << 1 );
			Wire.beginTransmission( AAddress );

			Wire.write( ARegister );
			Wire.write( Adata );

			Wire.endTransmission();
		}

	protected:
		virtual void UpdateOutput()
		{
			float	ANewValue = FChannels[ 0 ]->FNewValue;

			TransmitAt( 0b00010001, (byte)ANewValue );

			FChannels[ 0 ]->FValue = ANewValue;
		}

	protected:
		virtual void SystemStart() override
		{
			inherited::SystemStart();
			UpdateOutput();
		}

		virtual void SystemLoopUpdateHardware() override
		{
			if( FModified )
				if( ! ClockInputPin.IsConnected() )
					UpdateOutput();

			inherited::SystemLoopUpdateHardware();
		}

	protected:
		void DoClockReceive( void * )
		{
			if( FModified )
				UpdateOutput();
		
		}

	};
//---------------------------------------------------------------------------
	class Maxim_Potentiometer_I2C2Channel : public Mitov::Maxim_Potentiometer_I2C
	{
		typedef Mitov::Maxim_Potentiometer_I2C2Channel inherited;

	protected:
		virtual void UpdateOutput()
		{
			float	ANewValue = FChannels[ 0 ]->FNewValue;
			if( ANewValue == FChannels[ 1 ]->FNewValue )
				// Update both channels at once
				TransmitAt( 0b00010011, (byte)ANewValue );

			else
			{
				if( ANewValue != FChannels[ 0 ]->FValue )
					TransmitAt( 0b00010001, (byte)ANewValue );

				if( FChannels[ 1 ]->FNewValue != FChannels[ 1 ]->FValue )
					TransmitAt( 0b00010010, (byte)FChannels[ 1 ]->FNewValue );
			}

			FChannels[ 0 ]->FValue = ANewValue;
			FChannels[ 1 ]->FValue = FChannels[ 1 ]->FNewValue;
		}

	};
//---------------------------------------------------------------------------
	template<int T_MULTIPLIER> class Maxim_Potentiometer_I2C_Channel : public Mitov::Basic_Typed_I2CChannel<Maxim_Potentiometer_I2C>
	{
		typedef Mitov::Basic_Typed_I2CChannel<Maxim_Potentiometer_I2C>	inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			FNewValue = constrain( *((float *)_Data), 0, 1 ) * T_MULTIPLIER;
			if( FNewValue == FValue )
				return;

			FOwner.FModified = true;
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
}

#endif

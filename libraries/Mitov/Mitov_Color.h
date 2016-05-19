////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_COLOR_h
#define _MITOV_COLOR_h

#include <Mitov.h>
#include "Mitov_RandomGenerator.h"

namespace Mitov
{
#define Min Min
#define Max Max
//---------------------------------------------------------------------------
    class RandomColor : public Mitov::CommonRandomGenerator<Mitov::TColor>
    {
		typedef Mitov::CommonRandomGenerator<Mitov::TColor> inherited;

	public:
		virtual void GenerateValue()
		{
			FValue.Red = random( Min.Red, Max.Red + 1 );
			FValue.Green = random( Min.Green, Max.Green + 1 );
			FValue.Blue = random( Min.Blue, Max.Blue + 1 );
		}

		virtual void SystemInit()
		{
			int AMin = MitovMin( Min.Red, Max.Red );
			int AMax = MitovMax( Min.Red, Max.Red );

			Min.Red = AMin;
			Max.Red = AMax;

			AMin = MitovMin( Min.Green, Max.Green );
			AMax = MitovMax( Min.Green, Max.Green );

			Min.Green = AMin;
			Max.Green = AMax;

			AMin = MitovMin( Min.Blue, Max.Blue );
			AMax = MitovMax( Min.Blue, Max.Blue );

			Min.Blue = AMin;
			Max.Blue = AMax;

			inherited::SystemInit();
		}

	public:
		RandomColor() :
			inherited( 0, 0xFFFFFF )
		{
		}

    };
//---------------------------------------------------------------------------
    class BasicColorSource : public CommonSource
    {
    public:
        TColor	InitialValue;

    };
//---------------------------------------------------------------------------
	class AnalogToColor : public BasicColorSource
	{
	public:
		OpenWire::SinkPin	RedInputPin;
		OpenWire::SinkPin	GreenInputPin;
		OpenWire::SinkPin	BlueInputPin;

	protected:
		TColor	FValue;

	protected:
		void DoReceiveRed( void *_Data )
		{
			unsigned char AValue = (*(float *)_Data ) * 255;
			if( FValue.Red == AValue )
				return;

			FValue.Red = AValue;
			OutputPin.Notify( &FValue );
		}

		void DoReceiveGreen( void *_Data )
		{
			unsigned char AValue = (*(float *)_Data ) * 255;
			if( FValue.Green == AValue )
				return;

			FValue.Green = AValue;
			OutputPin.Notify( &FValue );
		}

		void DoReceiveBlue( void *_Data )
		{
			unsigned char AValue = (*(float *)_Data ) * 255;
			if( FValue.Blue == AValue )
				return;

			FValue.Blue = AValue;
			OutputPin.Notify( &FValue );
		}

		virtual void SystemStart() 
		{
			FValue = InitialValue;
			OutputPin.Notify( &FValue );			
		}

	public:
		AnalogToColor()		
		{
			RedInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&AnalogToColor::DoReceiveRed );
			GreenInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&AnalogToColor::DoReceiveGreen );
			BlueInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&AnalogToColor::DoReceiveBlue );
		}

	};
//---------------------------------------------------------------------------
	class ColorToAnalog : public CommonSink
	{
	public:
		OpenWire::SourcePin	RedOutputPin;
		OpenWire::SourcePin	GreenOutputPin;
		OpenWire::SourcePin	BlueOutputPin;

	protected:
		virtual void DoReceive( void *_Data )
		{
			TColor &AColor = *(TColor *)_Data;
			RedOutputPin.Notify( &AColor.Red );
			GreenOutputPin.Notify( &AColor.Green );
			BlueOutputPin.Notify( &AColor.Blue );
		}

	};	
//---------------------------------------------------------------------------
#undef Min
#undef Max
}

#endif

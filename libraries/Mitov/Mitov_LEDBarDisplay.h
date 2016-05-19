////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_LED_BAR_DISPLAY_h
#define _MITOV_LED_BAR_DISPLAY_h

#include <Mitov.h>
#include <Mitov_BasicDisplay.h>

namespace Mitov
{
	template<typename T> class LEDBarBasicDisplay : public Mitov::BasicDisplay<T>
	{
		typedef	Mitov::BasicDisplay<T>	inherited;

	public:
		Mitov::SimpleList<OpenWire::SourcePin>	BarsOutputPins;

	public:
		bool InvertedBars = false;
		bool SingleBar = false;

	public:
		virtual void DoReceive( void *_Data ) override
		{
			inherited::DoReceive( _Data );
			UpdateDisplay();
		}

	protected:
		virtual void UpdateDisplay() = 0;

	protected:
		void DisplayValue( long AValue )
		{
			for( int i = 0; i < BarsOutputPins.size(); ++i )
			{
				bool ABitValue;
				if( SingleBar )
					ABitValue = ( AValue == 0 ) ^ InvertedBars;

				else
					ABitValue = ( AValue > 0 ) ^ InvertedBars;

				BarsOutputPins[ i ].Notify( &ABitValue );
				--AValue;
			}
		}

	};
//---------------------------------------------------------------------------
	class LEDBarDisplay : public Mitov::LEDBarBasicDisplay<long>
	{
		typedef	Mitov::LEDBarBasicDisplay<long>	inherited;

	protected:
		virtual void UpdateDisplay() override
		{
			int AValue = (Enabled) ? FValue : 0;
			DisplayValue( AValue );
		}

	};
//---------------------------------------------------------------------------
	class LEDBarAnalogDisplay : public Mitov::LEDBarBasicDisplay<float>
	{
		typedef	Mitov::LEDBarBasicDisplay<float>	inherited;

	protected:
		virtual void UpdateDisplay() override
		{
			int AValue;
			if( Enabled )
			{
				if( SingleBar )
					AValue = FValue * BarsOutputPins.size();

				else
					AValue = FValue * BarsOutputPins.size() + 0.5;
			}

			else
				AValue = 0;

			DisplayValue( AValue );
		}

	};
//---------------------------------------------------------------------------
}

#endif

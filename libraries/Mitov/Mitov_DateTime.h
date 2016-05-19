////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_DATE_DIME_h
#define _MITOV_DATE_DIME_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
    class EncodeDateTime : public Mitov::BasicCommonMultiInput<long, TDateTime, 7>
	{
		typedef Mitov::BasicCommonMultiInput<long, TDateTime, 7> inherited;

    public:
        TDateTime	InitialValue;
		bool	OnlyModified = false;

	protected:
		virtual TDateTime CalculateOutput() override
		{
			TDateTime ADate;
			ADate.TryEncodeDateTime( InputPins[ 0 ].Value, InputPins[ 1 ].Value, InputPins[ 2 ].Value, InputPins[ 3 ].Value, InputPins[ 4 ].Value, InputPins[ 5 ].Value, InputPins[ 6 ].Value );
			return ADate;
		}

	};
//---------------------------------------------------------------------------
	class DecodeDateTime : public CommonSink
	{
		typedef Mitov::CommonSink inherited;

	public:
		OpenWire::SourcePin	OutputPins[ 7 ];

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			TDateTime &ADate = *(TDateTime *)_Data;
			uint16_t AItem16[ 7 ];
			ADate.DecodeDateTime( AItem16[ 0 ], AItem16[ 1 ], AItem16[ 2 ], AItem16[ 3 ], AItem16[ 4 ], AItem16[ 5 ], AItem16[ 6 ] );
			for( int i = 0; i < 7; ++ i )
			{
				long int AValue = AItem16[ i ];
				OutputPins[ i ].Notify( &AValue );
			}
		}
	};	
//---------------------------------------------------------------------------
}

#endif

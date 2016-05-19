////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_REMEMBER_h
#define _MITOV_REMEMBER_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	template<typename T_STORE, typename T_DATA> class Remember : public CommonEnableFilter
	{
		typedef CommonFilter inherited;

	public:
		OpenWire::SinkPin	RememberInputPin;
		OpenWire::SinkPin	RecallInputPin;

	protected:
		T_STORE FInData;
		T_STORE FSavedData;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			FInData = *(T_DATA*)_Data;
		}

	protected:
		void DoReceiveRemember( void *_Data )
		{
			if( Enabled )
				FSavedData = FInData;
		}

		void DoRecallSnapshot( void *_Data )
		{
			if( Enabled )
				OutputPin.SendValue( FSavedData );
		}

	public:
		Remember( T_STORE AData ) : 
			FInData( AData ), 
			FSavedData( AData )
		{
			RememberInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&Remember::DoReceiveRemember );
			RecallInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&Remember::DoRecallSnapshot );
		}
	};
//---------------------------------------------------------------------------
}

#endif

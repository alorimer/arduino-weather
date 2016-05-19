////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_SNAPSHOT_h
#define _MITOV_SNAPSHOT_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	template<typename T_STORE, typename T_DATA> class Snapshot : public CommonEnableFilter
	{
		typedef CommonFilter inherited;

	public:
		OpenWire::SinkPin	SnapshotInputPin;

	protected:
		T_STORE FData;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			FData = *(T_DATA*)_Data;
		}

	protected:
		void TakeSnapshot()
		{
			OutputPin.SendValue( FData );
		}

	protected:
		void DoReceiveSnapshot( void *_Data )
		{
			if( Enabled )
				TakeSnapshot();
		}

	public:
		Snapshot( T_STORE AData ) : 
			FData( AData )
		{
			SnapshotInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&Snapshot::DoReceiveSnapshot );
		}
	};
//---------------------------------------------------------------------------
}

#endif

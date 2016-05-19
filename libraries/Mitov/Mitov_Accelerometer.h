////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_ACCELEROMETER_h
#define _MITOV_ACCELEROMETER_h

#include <Mitov.h>

namespace Mitov
{
	class Memsic2125Accelerometer : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		OpenWire::SinkPin	XInputPin;
		OpenWire::SinkPin	YInputPin;

		OpenWire::SourcePin	XOutputPin;
		OpenWire::SourcePin	YOutputPin;

	public:
		bool	Enabled = true;

	protected:
		class DataSet : public OpenWire::Object
		{
		protected:
			OpenWire::SourcePin	*FOutputPin;

			unsigned long	FStartTime;
			bool			FOldValue;
			const Memsic2125Accelerometer	*FOwner;

		public:
			virtual void DoReceive( void *_Data )
			{
				if( ! FOwner->Enabled )
					return;

				bool AValue = *(bool *)_Data;
				if( FOldValue == AValue )
					return;

				unsigned long ANow = micros();
				FOldValue = AValue;
				if( AValue )
				{
					FStartTime = ANow;
					return;
				}

				long APeriod = ANow - FStartTime;

				if( APeriod == 0 )
					APeriod = 1;

				float AAcceleration = (( APeriod / 10 ) - 500) * 8;

				FOutputPin->Notify( &AAcceleration );
			}

		public:
			DataSet( const Memsic2125Accelerometer *AOwner, OpenWire::SinkPin *AInputPin, OpenWire::SourcePin *AOutputPin ) :
				FOwner( AOwner ),
				FOutputPin( AOutputPin ),
				FOldValue( false ),
				FStartTime( 0 )
			{
				AInputPin->SetCallback( this, (OpenWire::TOnPinReceive)&DataSet::DoReceive );
			}
		};


	protected:
		DataSet	FXDataSet;
		DataSet	FYDataSet;

	public:
		Memsic2125Accelerometer() :
			FXDataSet( this, &XInputPin, &XOutputPin ),
			FYDataSet( this, &YInputPin, &YOutputPin )
		{
		}
	};
}

#endif

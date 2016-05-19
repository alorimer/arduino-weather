////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_SPEED_TO_SPEED_AND_DIRECTION_h
#define _MITOV_SPEED_TO_SPEED_AND_DIRECTION_h

#include <Mitov.h>

namespace Mitov
{
	class SpeedToSpeedAndDirection : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink inherited;

	public:
		OpenWire::SourcePin	DirectionOutputPin;
		OpenWire::SourcePin	SpeedOutputPin;

	public:
		bool	Enabled : 1;
		bool	InverseForward : 1;
		bool	InverseReverse : 1;

	public:
		void	SetEnabled( bool AValue ) 
		{
			if( Enabled == AValue )
				return;

			Enabled = AValue;
			UpdateOutputs();
		}

		void	SetInverseForward( bool AValue ) 
		{
			if( InverseForward == AValue )
				return;

			InverseForward = AValue;
			UpdateOutputs();
		}

		void	SetInverseReverse( bool AValue ) 
		{
			if( InverseReverse == AValue )
				return;

			InverseReverse = AValue;
			UpdateOutputs();
		}

	protected:
		float	FCurrentSpeed = 0.5f;

	protected:
		virtual void DoReceive( void *_Data )
		{
			float ASpeed = constrain( *(float *)_Data, 0.0, 1.0 );
			if( FCurrentSpeed == ASpeed )
				return;

			FCurrentSpeed = ASpeed;
			UpdateOutputs();
		}

		void	UpdateOutputs()
		{
			if( Enabled )
			{
				float AOutSpeed = abs( FCurrentSpeed - 0.5 ) * 2;
				bool ADirection = FCurrentSpeed > 0.5;

				if( ADirection )
				{
					if( InverseForward )
						AOutSpeed = 1 - AOutSpeed;
				}

				else
				{
					if( InverseReverse )
						AOutSpeed = 1 - AOutSpeed;
				}

				SpeedOutputPin.Notify( &AOutSpeed );
				DirectionOutputPin.Notify( &ADirection );
			}

			else
			{
				SpeedOutputPin.SendValue( 0 );
				DirectionOutputPin.SendValue( false );
			}
		}

	protected:
		virtual void SystemStart()
		{
			inherited::SystemStart();
			UpdateOutputs();
		}

	public:
		SpeedToSpeedAndDirection() :
			Enabled( true ),
			InverseForward( false ),
			InverseReverse( false )
		{
		}

	};
}

#endif

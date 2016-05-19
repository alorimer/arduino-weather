////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_SCHMITT_TRIGGER_h
#define _MITOV_SCHMITT_TRIGGER_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	template<typename T> class SchmittTrigger : public CommonEnableFilter
	{
		typedef CommonEnableFilter inherited;

    public:
		bool	InitialValue : 1;
		bool	Inverted : 1;

	protected:
        T	FValue;
		T	FThreshold;
		T	FCurrentValue;

    public:
		void SetValue( T AValue )
		{
			if( FValue == AValue )
				return;

			FValue = AValue;
			ProcessOutput();
		}

		void SetThreshold( T AValue )
		{
			if( FThreshold == AValue )
				return;

			FThreshold = AValue;
			ProcessOutput();
		}

		void SetInverted( bool AValue )
		{
			if( Inverted == AValue )
				return;

			Inverted = AValue;
			ProcessOutput();
		}

	protected:
		void ProcessOutput()
		{
			if( ! inherited::Enabled )
				return;

			bool AValue;

			if( Inverted )
			{
				if( InitialValue )
					AValue = ( FCurrentValue < FValue + FThreshold );

				else
					AValue = ( FCurrentValue < FValue - FThreshold );

			}

			else
			{
				if( InitialValue )
					AValue = ( FCurrentValue > FValue - FThreshold );

				else
					AValue = ( FCurrentValue > FValue + FThreshold );

			}

			if( InitialValue == AValue )
				return;

			InitialValue = AValue;
			OutputPin.Notify( &AValue );
		}

	protected:
		virtual void DoReceive( void *_Data )
		{
			T AValue = *(T*)_Data;
			if( AValue == FCurrentValue )
				return;

			FCurrentValue = AValue;
			ProcessOutput();
		}

	public:
		SchmittTrigger( T AValue, T AThreshold ) : 
			FValue( AValue ),
			FThreshold( AThreshold ),
			InitialValue( false ),
			Inverted( false )
		{
		}

	};
//---------------------------------------------------------------------------
}

#endif
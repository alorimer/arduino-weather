////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_BASIC_GPIO_h
#define _MITOV_BASIC_GPIO_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class BasicGPIOChannel;
//---------------------------------------------------------------------------
	template<typename T_BASECLASS> class BasicGPIO : public T_BASECLASS
	{
		typedef T_BASECLASS inherited;

	public:
		OpenWire::ConnectSinkPin	ReadInputPin;

	public:
		Mitov::SimpleList<BasicGPIOChannel *>	FChannels;

	protected:
		virtual void PerformRead() = 0;

		void DoReadInputReceive( void * _Data )
		{
			PerformRead();
		}

	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros ) 
		{
			if( ! ReadInputPin.IsConnected() )
				PerformRead();
		}

	public:
		BasicGPIO()
		{
			ReadInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&BasicGPIO::DoReadInputReceive );
		}

	};
//---------------------------------------------------------------------------
	template<typename T_BASECLASS> class EnableBasicGPIO : public BasicGPIO<T_BASECLASS>
	{
		typedef BasicGPIO<T_BASECLASS> inherited;

	public:
		void	SetEnabled( bool AValue )
		{
			if( inherited::Enabled == AValue )
				return;

			inherited::Enabled = AValue;
			UpdateEnable();
		}

	protected:
		virtual void UpdateEnable() = 0;

	};
//---------------------------------------------------------------------------
	class BasicGPIOChannel : public OpenWire::Object
	{
	public:
		OpenWire::SourcePin	OutputPin;

	protected:
		bool FInValue = false;

	public:
		void SendOutput()
		{
			OutputPin.Notify( &FInValue );
		}

		void UpdateOutput( bool AValue )
		{
			if( FInValue == AValue )
				return;

			FInValue = AValue;

			OutputPin.Notify( &FInValue );
		}

		virtual void UpdateInput() = 0;

	};
//---------------------------------------------------------------------------
	template<typename T_OWNER> class OwnedBasicGPIOChannel : public BasicGPIOChannel
	{
	public:
		OpenWire::SinkPin	InputPin;

	public:
        void SetIsOutput( bool AValue )
        {
            if( FIsOutput == AValue )
                return;

            FIsOutput = AValue;
            PinDirectionsInit();
        }

        void SetIsPullUp( bool AValue )
        {
            if( FIsPullUp == AValue )
                return;

            FIsPullUp = AValue;
            PinDirectionsInit();
        }

	protected:
		T_OWNER &FOwner;
		bool FValue;
		int  FIndex;
        bool FIsOutput;
        bool FIsPullUp;
		bool FCombinedInOut;

	protected:
        virtual void PinDirectionsInit() = 0;

	protected:
		void DoDataReceive( void * _Data )
		{
			bool AValue = *(bool *)_Data;
			if( FValue == AValue )
				return;

			FValue = AValue;
			FOwner.SetChannelValue( FIndex, AValue );
		}

	public:
		OwnedBasicGPIOChannel( T_OWNER &AOwner, int AIndex, bool AInitialValue, bool AIsOutput, bool AIsPullUp, bool AIsCombinedInOut ) :
			FOwner( AOwner ),
			FIndex( AIndex ),
			FIsOutput( AIsOutput ),
			FIsPullUp( AIsPullUp ),
			FCombinedInOut( AIsCombinedInOut ),
			FValue( AInitialValue )
		{
			AOwner.FChannels.push_back( this );

			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&OwnedBasicGPIOChannel::DoDataReceive );
//			PinDirectionsInit();
		}

	};
//---------------------------------------------------------------------------
}

#endif

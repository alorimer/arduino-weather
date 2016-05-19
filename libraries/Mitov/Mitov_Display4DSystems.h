////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_DISPLAY_4D_SYSTEMS_h
#define _MITOV_DISPLAY_4D_SYSTEMS_h

#include <Mitov.h>
#include <genieArduino.h>

namespace Mitov
{


/*
	void myGenieEventHandler(void)
	{
		Serial.println( "myGenieEventHandler" );
	}
*/
	class Display4DSystems;
//---------------------------------------------------------------------------
	class ViSiGenieBasicObject : public OpenWire::Object
	{
	protected:
		Display4DSystems &FOwner;

	public:
		virtual void Start() {}
		virtual void Process() {}

	public:
		ViSiGenieBasicObject( Display4DSystems &AOwner );

	};
//---------------------------------------------------------------------------
	class ViSiGenieBasicOutObject
	{
	public:
		virtual bool ProcessOut( Genie &AGenie, genieFrame &Event ) = 0;

	public:
		ViSiGenieBasicOutObject( Display4DSystems &AOwner );

	};
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX> class ViSiGenieBasicTypedOutObject : public ViSiGenieBasicOutObject
	{
		typedef	ViSiGenieBasicOutObject inherited;

	protected:
		virtual void PrcessInValue( uint16_t AValue ) = 0;

	public:
		virtual bool ProcessOut( Genie &AGenie, genieFrame &Event ) override
		{
//			Serial.println( "ProcessOut" );
			if( Event.reportObject.cmd == GENIE_REPORT_EVENT )
			{
/*
				Serial.println( "GENIE_REPORT_EVENT" );
				Serial.print( Event.reportObject.object );
				Serial.print( " - " );
				Serial.println( Event.reportObject.index );
*/
				if( Event.reportObject.object == V_OBJECT )
					if( Event.reportObject.index == V_INDEX )
					{
						uint16_t AValue = AGenie.GetEventData(&Event);
//						Serial.println( AValue );
						PrcessInValue( AValue );

						return true;
					}

				return false;
			}
		}


	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX> class ViSiGenieTypedCharOut : public ViSiGenieBasicTypedOutObject<V_OBJECT, V_INDEX>
	{
		typedef	ViSiGenieBasicTypedOutObject<V_OBJECT, V_INDEX> inherited;

	public:
		OpenWire::SourcePin	OutputPin;

//	public:
//		bool	EnterNewLine;

	protected:
		virtual void PrcessInValue( uint16_t AValue ) override
		{
			char *ATypedValue = (char *)AValue;
			OutputPin.Notify( &ATypedValue );
//			if( EnterNewLine )
//				if( *ATypedValue == '\r' )
//					OutputPin.SendValue( '\n' );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class ViSiGenieBasicIn : public ViSiGenieBasicObject
	{
		typedef	ViSiGenieBasicObject inherited;

	public:
		OpenWire::SinkPin	InputPin;

	protected:
		virtual void DoReceive( void *_Data ) = 0;

	public:
		ViSiGenieBasicIn( Display4DSystems &AOwner ) :
			inherited( AOwner )
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ViSiGenieBasicIn::DoReceive );
		}

	};
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX, typename T_DATA> class ViSiGenieBasicTypedIn : public ViSiGenieBasicIn
	{
		typedef	ViSiGenieBasicIn inherited;

	public:
		OpenWire::SinkPin	RefreshInputPin;

	public:
		bool	OnlyChanged : 1;

	public:
		bool		FStarted : 1;
		bool		FReceived : 1;

	public:
		T_DATA		FOldValue;
		T_DATA		FValue;

		
	protected:
		virtual void DoReceive( void *_Data );

		virtual void DoRefreshReceive( void *_Data )
		{
			FStarted = false;
		}

	public:
		ViSiGenieBasicTypedIn( Display4DSystems &AOwner, T_DATA AInitialValue ) :
			inherited( AOwner ),
			FValue( AInitialValue ),
			FOldValue( AInitialValue ),
			FStarted( false ),
			FReceived( false ),
			OnlyChanged( true )
		{
			RefreshInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ViSiGenieBasicTypedIn::DoRefreshReceive );
		}
	};
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX, typename T_DATA> class ViSiGenieTypedIn : public ViSiGenieBasicTypedIn<V_OBJECT, V_INDEX, T_DATA>
	{
		typedef	ViSiGenieBasicTypedIn<V_OBJECT, V_INDEX, T_DATA> inherited;

	protected:
		virtual uint16_t	GetValue()	{ return inherited::FValue; };

	public:
		virtual void Process();

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX, typename T_DATA> class ViSiGenieTypedInOut : public ViSiGenieTypedIn<V_OBJECT, V_INDEX, T_DATA>, public ViSiGenieBasicTypedOutObject<V_OBJECT, V_INDEX>
	{
		typedef	ViSiGenieTypedIn<V_OBJECT, V_INDEX, T_DATA> inherited;

	public:
		OpenWire::SourcePin	OutputPin;
/*
	public:
		virtual bool ProcessOut( Genie &AGenie, genieFrame &Event )
		{
		}
*/
	protected:
		virtual void PrcessInValue( uint16_t AValue ) override
		{
			T_DATA ATypedValue = (T_DATA)AValue;
			OutputPin.Notify( &ATypedValue );
		}

	public:
		ViSiGenieTypedInOut( Display4DSystems &AOwner, T_DATA AInitialValue ) :
			inherited( AOwner, AInitialValue ),
			ViSiGenieBasicTypedOutObject<V_OBJECT, V_INDEX>( AOwner )
		{
		}
	};
//---------------------------------------------------------------------------
/*
	template<int V_OBJECT, int V_INDEX> class ViSiGenieTypedAnalogIn : public ViSiGenieTypedIn<V_OBJECT, V_INDEX, float>
	{
		typedef	ViSiGenieTypedIn<V_OBJECT, V_INDEX, float> inherited;

	protected:
		virtual uint16_t	GetValue()	{ return inherited::FValue + 0.5; } override;

	public:
		ViSiGenieTypedAnalogIn( Display4DSystems &AOwner ) :
			inherited( AOwner, 0.0 )
		{
		}
	};
*/
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX> class ViSiGenieTypedUnsignedIn : public ViSiGenieTypedIn<V_OBJECT, V_INDEX, uint32_t>
	{
		typedef	ViSiGenieTypedIn<V_OBJECT, V_INDEX, uint32_t> inherited;

	public:
		using inherited::inherited;
	};
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX> class ViSiGenieTypedUnsignedInOut : public ViSiGenieTypedInOut<V_OBJECT, V_INDEX, uint32_t>
	{
		typedef	ViSiGenieTypedInOut<V_OBJECT, V_INDEX, uint32_t> inherited;

	public:
		using inherited::inherited;
	};
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX> class ViSiGenieTypedDigitalIn : public ViSiGenieTypedIn<V_OBJECT, V_INDEX, bool>
	{
		typedef	ViSiGenieTypedIn<V_OBJECT, V_INDEX, bool> inherited;

	protected:
		virtual uint16_t	GetValue() override { return inherited::FValue ? 1 : 0; }

	public:
		ViSiGenieTypedDigitalIn( Display4DSystems &AOwner ) :
			inherited( AOwner, false )
		{
		}
	};
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX> class ViSiGenieTypedDigitalInOut : public ViSiGenieTypedInOut<V_OBJECT, V_INDEX, bool>
	{
		typedef	ViSiGenieTypedInOut<V_OBJECT, V_INDEX, bool> inherited;

	protected:
		virtual uint16_t	GetValue() override	{ return inherited::FValue ? 1 : 0; } 

	protected:
		bool	FOldReceivedValue = false;

	protected:
		virtual void PrcessInValue( uint16_t AValue )
		{
			bool ATypedValue = (bool)AValue;
			if( ! FOldReceivedValue )
				if( ! ATypedValue )
					inherited::OutputPin.SendValue( true );

			FOldReceivedValue = ATypedValue;
			inherited::OutputPin.Notify( &ATypedValue );
		}

	public:
		ViSiGenieTypedDigitalInOut( Display4DSystems &AOwner ) :
			inherited( AOwner, false )
		{
		}
	};
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX> class ViSiGenieTypedColorInOut : public ViSiGenieTypedInOut<V_OBJECT, V_INDEX, Mitov::TColor>
	{
		typedef	ViSiGenieTypedInOut<V_OBJECT, V_INDEX, Mitov::TColor> inherited;

	protected:
		virtual uint16_t	GetValue() override	
		{ 
			return ( ( inherited::FValue.Red >> 3 ) << ( 6 + 5 )) | ( ( inherited::FValue.Green >> 2 ) & 0b111111 ) << 5 | ( ( inherited::FValue.Blue >> 3 ) & 0b11111 );
		}

		virtual void PrcessInValue( uint16_t AValue ) override
		{
			Mitov::TColor ATypedValue; // = AValue;
			ATypedValue.Red = ( AValue & 0b1111100000000000 ) >> ( 6 + 5 - 3 ); //0b1111100000000000 
			ATypedValue.Green = ( AValue & 0b11111100000 ) >> ( 5 - 2 ); 
			ATypedValue.Blue = ( AValue & 0b11111 ) << 3; 
			inherited::OutputPin.Notify( &ATypedValue );
		}

	public:
		ViSiGenieTypedColorInOut( Display4DSystems &AOwner ) :
			inherited( AOwner, false )
		{
		}
	};
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX> class ViSiGenieTypedStringIn : public ViSiGenieBasicTypedIn<V_OBJECT, V_INDEX, String>
	{
		typedef	ViSiGenieBasicTypedIn<V_OBJECT, V_INDEX, String> inherited;

	protected:
		virtual void DoReceive( void *_Data ) override;
		virtual void Process() override;

	public:
		ViSiGenieTypedStringIn( Display4DSystems &AOwner ) :
			inherited( AOwner, "" )
		{
		}
	};
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX> class ViSiGenieTypedClockInOut : public ViSiGenieBasicIn, public ViSiGenieBasicOutObject
	{
		typedef	ViSiGenieBasicIn inherited;

	public:
		OpenWire::SourcePin	OutputPin;

	protected:
		virtual void DoReceive( void *_Data ) override;

		virtual bool ProcessOut( Genie &AGenie, genieFrame &Event ) override
		{
//			Serial.println( "ProcessOut" );
			if( Event.reportObject.cmd == GENIE_REPORT_EVENT )
			{
/*
				Serial.println( "GENIE_REPORT_EVENT" );
				Serial.print( Event.reportObject.object );
				Serial.print( " - " );
				Serial.println( Event.reportObject.index );
*/
				if( Event.reportObject.object == V_OBJECT )
					if( Event.reportObject.index == V_INDEX )
					{
						OutputPin.Notify( NULL );
//						uint16_t AValue = AGenie.GetEventData(&Event);
//						Serial.println( AValue );
//						PrcessInValue( AValue );

						return true;
					}

				return false;
			}
		}

	public:
		ViSiGenieTypedClockInOut( Display4DSystems &AOwner ) :
			inherited( AOwner ),
			ViSiGenieBasicOutObject( AOwner )
		{
		}
	};
//---------------------------------------------------------------------------
	template<int V_INDEX> class ViSiGenieSpectrum : public ViSiGenieBasicObject
	{
		typedef	ViSiGenieBasicObject inherited;

	public:
		Mitov::SimpleList<OpenWire::VlaueChangeSinkPin<bool> > ColumnsInputPins;

	public:
		virtual void Process() override;

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class Display4DSystems : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		OpenWire::SourcePin	ResetOutputPin;

	public:
		float	Contrast = 1.0f;

	public:
		Mitov::SimpleList<ViSiGenieBasicObject *>		FElements;
		Mitov::SimpleList<ViSiGenieBasicOutObject *>	FOutElements;
		bool	FModified = true;

	public:
		Genie FGenie;

	public:
		void	SetContrast( float AValue )
		{
			AValue = constrain( AValue, 0.0, 1.0 );
			if( AValue == Contrast )
				return;

			Contrast = AValue;
			FGenie.WriteContrast( Contrast * 15 + 0.5 );
		}

	protected:
		Mitov::BasicSerialPort &FSerial;

	protected:
		virtual void SystemStart() 
		{
			FGenie.Begin( FSerial.GetStream() );

  //FGenie.AttachEventHandler(myGenieEventHandler); // Attach the user function Event Handler for processing events
			bool AValue = true;
			ResetOutputPin.Notify( &AValue );

			delay( 100 );

			AValue = false;
			ResetOutputPin.Notify( &AValue );

			delay( 3500 );
			FGenie.WriteContrast( Contrast );

			for( int i = 0; i < FElements.size(); ++ i )
				FElements[ i ]->Start();

			inherited::SystemStart();
		}

		virtual void SystemLoopEnd() 
		{
			FGenie.DoEvents(false); // This calls the library each loop to process the queued responses from the display

			if( FModified )
				for( int i = 0; i < FElements.size(); ++ i )
					FElements[ i ]->Process();

			genieFrame AEvent;
			if( FGenie.DequeueEvent(&AEvent))
			{
//				Serial.println( "DequeueEvent" );
				for( int i = 0; i < FOutElements.size(); ++ i )
					if( FOutElements[ i ]->ProcessOut( FGenie, AEvent ))
						break;
			}

			inherited::SystemLoopEnd();
		}


	public:
		Display4DSystems( Mitov::BasicSerialPort &ASerial ) :
			FSerial( ASerial )
		{
		}
	};
//---------------------------------------------------------------------------
	class ViSiGenieSounds : public ViSiGenieBasicObject
	{
		typedef	ViSiGenieBasicObject inherited;

	public:
		Mitov::SimpleList<OpenWire::IndexedSinkPin> TracksStartInputPins;

	public:
		OpenWire::SinkPin	StopInputPin;
		OpenWire::SinkPin	PauseInputPin;
		OpenWire::SinkPin	ResumeInputPin;

	public:
		float	Volume;

	public:
		void	SetVolume( float AValue )
		{
			AValue = constrain( AValue, 0.0, 1.0 );
			if( AValue == Volume )
				return;

			Volume = AValue;
			FOwner.FGenie.WriteObject( GENIE_OBJ_SOUND, 1, Volume * 100 + 0.5 );
		}

	public:
		virtual void Start() 
		{
			for( int i = 0; i < TracksStartInputPins.size(); i ++ )
			{
				TracksStartInputPins[ i ].Index = i;
				TracksStartInputPins[ i ].SetCallback( this, (OpenWire::TOnPinIndexedReceive)&ViSiGenieSounds::DoIndexReceive );
			}

			FOwner.FGenie.WriteObject( GENIE_OBJ_SOUND, 1, Volume * 100 + 0.5 );
		}

	protected:
		void DoIndexReceive( int AIndex, void *_Data )
		{
			FOwner.FGenie.WriteObject( GENIE_OBJ_SOUND, 0, AIndex );
		}

		void DoReceiveStop( void *_Data )
		{
			FOwner.FGenie.WriteObject( GENIE_OBJ_SOUND, 4, 0 );
		}

		void DoReceivePause( void *_Data )
		{
			FOwner.FGenie.WriteObject( GENIE_OBJ_SOUND, 2, 0 );
		}

		void DoReceiveResume( void *_Data )
		{
			FOwner.FGenie.WriteObject( GENIE_OBJ_SOUND, 3, 0 );
		}

	public:
		ViSiGenieSounds( Display4DSystems &AOwner ) :
			inherited( AOwner )
		{
			StopInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ViSiGenieSounds::DoReceiveStop );
			PauseInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ViSiGenieSounds::DoReceivePause );
			ResumeInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ViSiGenieSounds::DoReceiveResume );
		}
	};
//---------------------------------------------------------------------------
	ViSiGenieBasicObject::ViSiGenieBasicObject( Display4DSystems &AOwner ) :
		FOwner( AOwner )
	{
		FOwner.FElements.push_back( this );
	}
//---------------------------------------------------------------------------
	ViSiGenieBasicOutObject::ViSiGenieBasicOutObject( Display4DSystems &AOwner )
	{
		AOwner.FOutElements.push_back( this );
	}
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX, typename T_DATA> void ViSiGenieBasicTypedIn<V_OBJECT, V_INDEX, T_DATA>::DoReceive( void *_Data )
	{
		FValue = *(T_DATA *)_Data;
		if( OnlyChanged && FStarted )
			if( FValue == FOldValue )
				return;

		FReceived = true;
		FOwner.FModified = true;
	}
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX, typename T_DATA> void ViSiGenieTypedIn<V_OBJECT, V_INDEX, T_DATA>::Process()
	{
		if( inherited::OnlyChanged && inherited::FStarted )
			if( inherited::FOldValue == inherited::FValue )
				return;

		if( ! inherited::FReceived )
			return;

		inherited::FStarted = true;
		inherited::FOldValue = inherited::FValue;

		inherited::FOwner.FGenie.WriteObject(V_OBJECT, V_INDEX, GetValue() );
	}
//---------------------------------------------------------------------------
	template<int V_INDEX> void ViSiGenieSpectrum<V_INDEX>::Process() 
	{
		for( int i = 0; i < ColumnsInputPins.size(); i ++ )
			if( ColumnsInputPins[ i ].OldValue != ColumnsInputPins[ i ].Value )
			{
				uint16_t AValue = ( i << 8 ) | ( ColumnsInputPins[ i ].Value & 0xFF );
				FOwner.FGenie.WriteObject( GENIE_OBJ_SPECTRUM, V_INDEX, AValue );
			}

	}
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX> void ViSiGenieTypedStringIn<V_OBJECT, V_INDEX>::DoReceive( void *_Data )
	{
		inherited::FValue = (char *)_Data;
		if( inherited::OnlyChanged && inherited::FStarted )
			if( inherited::FValue == inherited::FOldValue )
				return;

		inherited::FReceived = true;
		inherited::FOwner.FModified = true;
	}
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX> void ViSiGenieTypedStringIn<V_OBJECT, V_INDEX>::Process()
	{
		if( inherited::OnlyChanged && inherited::FStarted )
			if( inherited::FOldValue == inherited::FValue )
				return;

		if( ! inherited::FReceived )
			return;

		inherited::FStarted = true;
		inherited::FOldValue = inherited::FValue;

		inherited::FOwner.FGenie.WriteStr(V_INDEX, inherited::FValue.c_str() );
	}
//---------------------------------------------------------------------------
	template<int V_OBJECT, int V_INDEX> void ViSiGenieTypedClockInOut<V_OBJECT, V_INDEX>::DoReceive( void *_Data )
	{
		FOwner.FGenie.WriteObject( V_OBJECT, V_INDEX, 0 );
	}
//---------------------------------------------------------------------------
}

#endif

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_KEYPAD_h
#define _MITOV_KEYPAD_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class KeypadBasicKeyElement;
	class BasicKeypad;
//---------------------------------------------------------------------------
	class KeypadKeys
	{
	public:
		virtual void RegisterKey( int ARow, int ACol, KeypadBasicKeyElement *AKey ) = 0;
		virtual BasicKeypad *GetRootOwner() = 0;
	};
//---------------------------------------------------------------------------
	class KeypadBasicKeyElement;
//---------------------------------------------------------------------------
	class BasicKeypad : public OpenWire::Component, public KeypadKeys
	{
		typedef OpenWire::Component inherited;

	public:
		uint32_t	DebounceInterval = 50;

	};
//---------------------------------------------------------------------------
	template< int C_X, int C_Y > class Keypad : public BasicKeypad
	{
		typedef BasicKeypad inherited;

	public:
//		Mitov::SimpleList<OpenWire::VlaueSinkPin<bool> >	RowsInputPins;
		Mitov::SimpleList<OpenWire::SourcePin>	ColumnsOutputPins;

	public:
		const uint8_t	*_Pins;

	protected:
		bool	FInScanning = false;

		KeypadBasicKeyElement	*FKeyMap[ C_X ][ C_Y ];

	public:
		virtual BasicKeypad *GetRootOwner() override
		{
			return this;
		}

		virtual void RegisterKey( int ARow, int ACol, KeypadBasicKeyElement *AKey ) override
		{
			FKeyMap[ ACol ][ ARow ] = AKey;
		}

	protected:
		virtual void SystemInit()
		{
			for( int i = 0; i < C_Y ; ++i )
				pinMode( _Pins[ i ], INPUT_PULLUP );

			inherited::SystemInit();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros ) override;

	};
//---------------------------------------------------------------------------
	class KeypadBasicKeyElement // : public OpenWire::Component
	{
	public:
		virtual void SetButtonValue( unsigned long currentMicros, bool AValue )
		{
		}

	};
//---------------------------------------------------------------------------
	template<typename T_OWNER> class KeypadKeyElement : public KeypadBasicKeyElement
	{
		typedef KeypadBasicKeyElement inherited;

	protected:
		T_OWNER &FOwner;

	public:
		KeypadKeyElement( T_OWNER &AOwner ) :
			FOwner( AOwner )
		{
		}

		KeypadKeyElement( T_OWNER &AOwner, int ARow, int ACol ) :
			FOwner( AOwner )
		{
			AOwner.RegisterKey( ACol, ARow, this );
		}

	};
//---------------------------------------------------------------------------
	template<typename T_OWNER> class KeypadBasicKey : public KeypadKeyElement<T_OWNER>
	{
		typedef KeypadKeyElement<T_OWNER> inherited;

/*
	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			inherited::SystemLoopBegin( currentMicros );
		}
*/
	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class DigitalKeypadKey : public KeypadBasicKey<BasicKeypad>
	{
		typedef KeypadBasicKey<BasicKeypad> inherited;

	public:
		OpenWire::SourcePin	OutputPin;

	protected:
		unsigned long	FLastTime = 0;
		bool	FValue : 1;
		bool	FLastValue : 1;

	public:
		virtual void SetButtonValue( unsigned long currentMicros, bool AValue )
		{
			if( AValue != FLastValue )
				FLastTime = millis();

			FLastValue = AValue;
			if( FValue != FLastValue )
				if( millis() - FLastTime > FOwner.DebounceInterval )
				{
					FValue = FLastValue;
					OutputPin.SendValue( ! FValue );
				}

		}

	public:
		DigitalKeypadKey( BasicKeypad &AOwner, int ARow, int ACol ) :
			inherited( AOwner, ARow, ACol ),
			FLastValue( false ),
			FValue( false )
		{
		}
	};
//---------------------------------------------------------------------------
	class CharacterKeyGroup : public KeypadKeyElement<KeypadKeys>, public KeypadKeys
	{
		typedef KeypadKeyElement<KeypadKeys> inherited;

	public:
		OpenWire::SourcePin	OutputPin;

	public:
		virtual BasicKeypad *GetRootOwner() override
		{
			return FOwner.GetRootOwner();
		}

		virtual void RegisterKey( int ARow, int ACol, KeypadBasicKeyElement *AKey ) override
		{
			FOwner.RegisterKey( ARow, ACol, AKey );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class CharKeypadKey : public KeypadBasicKey<CharacterKeyGroup>
	{
		typedef KeypadBasicKey<CharacterKeyGroup> inherited;

	public:
		char	Character = 'a';

	protected:
		unsigned long	FLastTime = 0;
		bool	FValue = false;
		bool	FLastValue = false;

		BasicKeypad	*FRootOwner;

	public:
		virtual void SetButtonValue( unsigned long currentMicros, bool AValue )
		{
			if( AValue != FLastValue )
				FLastTime = millis();

			FLastValue = AValue;
			if( FValue != FLastValue )
				if( millis() - FLastTime > FRootOwner->DebounceInterval )
				{
					FValue = FLastValue;
					if( ! FValue )
						FOwner.OutputPin.Notify( &Character );

//					OutputPin.Notify( &FValue );
				}

		}

	public:
		CharKeypadKey( CharacterKeyGroup &AOwner, int ARow, int ACol ) :
			inherited( AOwner, ARow, ACol )
		{
			FRootOwner = AOwner.GetRootOwner();
		}
	};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
	template< int C_X, int C_Y > void Keypad< C_X, C_Y >::SystemLoopBegin( unsigned long currentMicros )
	{
		if( ! FInScanning )
		{
			FInScanning = true;
			for( int i = 0; i < C_X; ++i )
				ColumnsOutputPins[ i ].SendValue( false );
		}

		for( int i = 0; i < C_Y; ++i )
  			if( ! digitalRead( _Pins[ i ] ))
				FInScanning = false;

//		if( ! FInScanning )
//			Serial.println( "----------------------------" );

		for( int i = 0; i < C_X; ++i )
		{
			if( ! FInScanning )
			{
				for( int j = 0; j < C_X; ++j )
				{
//					Serial.print( i != j ); Serial.print( " " );
					ColumnsOutputPins[ j ].SendValue( i != j );
				}

//				Serial.println( "" );
			}

//			if( ! FInScanning )
//				Serial.println( "+++++++++++++++++++++++++" );

			for( int j = 0; j < C_Y; ++j )
				if( FKeyMap[ i ][ j ] )
				{
//					if( ! FInScanning )
//					{
//						if( ! digitalRead( _Pins[ j ] ) )
//						{
//							Serial.print( i ); Serial.print( "," ); Serial.println( j );
//						}
//						Serial.print( i ); Serial.print( "," ); Serial.print( j ); Serial.print( " = " ); Serial.print( digitalRead( _Pins[ j ] ) ); Serial.print( "  " ); 
//					}

					FKeyMap[ i ][ j ]->SetButtonValue( currentMicros, digitalRead( _Pins[ j ] ));
				}

//			if( ! FInScanning )
//				Serial.println( "" );
		}
			
		inherited::SystemLoopBegin( currentMicros );
	}
//---------------------------------------------------------------------------
}

#endif

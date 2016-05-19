////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_IQUID_CRYSTAL_DISPLAY_h
#define _MITOV_IQUID_CRYSTAL_DISPLAY_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class LiquidCrystalDisplay;
//---------------------------------------------------------------------------
	class LiquidCrystalElementBasic : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public: // Public for the print access
		LCD	*FLcd;

	public:
		virtual void DisplayInit() {}
		virtual void DisplayStart() {}

	public:
		LiquidCrystalElementBasic( Mitov::LiquidCrystalDisplay &AOwner );

	};
//---------------------------------------------------------------------------
	class LiquidCrystalElementBasicPositionedField : public LiquidCrystalElementBasic
	{
		typedef Mitov::LiquidCrystalElementBasic inherited;

	public:
		unsigned long Column = 0;
		unsigned long Row = 0;

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class LiquidCrystalElementBasicPositionedWidthField : public LiquidCrystalElementBasicPositionedField
	{
		typedef Mitov::LiquidCrystalElementBasicPositionedField inherited;

	public:
		unsigned long Width = 16;

	public:
		void ClearLine()
		{
			FLcd->setCursor( Column, Row );
		}

		void NewLine( int AClearSize )
		{
			for( int i = 0; i < Width - AClearSize; ++ i )
				FLcd->print( ' ' );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class LiquidCrystalElementTextField : public LiquidCrystalElementBasicPositionedWidthField
	{
		typedef Mitov::LiquidCrystalElementBasicPositionedWidthField inherited;

	public:
		String	InitialValue;

	protected:
		virtual void SystemStart() override
		{
			inherited::SystemStart();
			ClearLine();
			int AClearSize = FLcd->print( InitialValue );
			NewLine( AClearSize );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	template<typename T> class LiquidCrystalElementBasicPositionedWidthTypedField : public LiquidCrystalElementBasicPositionedWidthField
	{
		typedef Mitov::LiquidCrystalElementBasicPositionedWidthField inherited;

	public:
		T	InitialValue;

	public:
		OpenWire::SinkPin	InputPin;

	protected:
		virtual int PrintValue( T AValue ) = 0;

		void ClearPrintValue( T AValue )
		{
			InitialValue = AValue;
//			Serial.println( AValue );
			ClearLine();
			int AClearSize = PrintValue( AValue );
			NewLine( AClearSize );
		}

	protected:
		virtual void SystemStart() override
		{
			inherited::SystemStart();
//			char AText[ 50 ];
//			itoa( InitialValue, AText, Base );
//			int AClearSize = FLcd->print( AText );
			ClearPrintValue( InitialValue );
		}

		void DoReceiveData( void *_Data )
		{
			ClearPrintValue( *(T*)_Data );
		}

	public:
		LiquidCrystalElementBasicPositionedWidthTypedField( Mitov::LiquidCrystalDisplay &AOwner, T AInitialValue ) :
			inherited( AOwner ),
			InitialValue( AInitialValue )
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&LiquidCrystalElementBasicPositionedWidthTypedField::DoReceiveData );
		}

	};
//---------------------------------------------------------------------------
	class LiquidCrystalElementIntegerField : public LiquidCrystalElementBasicPositionedWidthTypedField<long>
	{
		typedef Mitov::LiquidCrystalElementBasicPositionedWidthTypedField<long> inherited;

	public:
		int	Base = 10;

	protected:
		virtual int PrintValue( long AValue ) override
		{
			char AText[ 50 ];
			itoa( AValue, AText, Base );
			return FLcd->print( AText );
		}

	public:
		LiquidCrystalElementIntegerField( Mitov::LiquidCrystalDisplay &AOwner ) :
			inherited( AOwner, 0 )
		{
		}

	};
//---------------------------------------------------------------------------
	class LiquidCrystalElementAnalogField : public LiquidCrystalElementBasicPositionedWidthTypedField<float>
	{
		typedef Mitov::LiquidCrystalElementBasicPositionedWidthTypedField<float> inherited;

	public:
		int	MinWidth = 1;
		int	Precision = 3;

	protected:
		virtual int PrintValue( float AValue ) override
		{
			char AText[ 50 ];
			dtostrf( AValue,  MinWidth, Precision, AText );
			return FLcd->print( AText );
		}

	public:
		LiquidCrystalElementAnalogField( Mitov::LiquidCrystalDisplay &AOwner ) :
			inherited( AOwner, 0 )
		{
		}

	};
//---------------------------------------------------------------------------
	class LiquidCrystalElementDigitalField : public LiquidCrystalElementBasicPositionedWidthTypedField<bool>
	{
		typedef Mitov::LiquidCrystalElementBasicPositionedWidthTypedField<bool> inherited;

	public:
		String	TrueValue = "true";
		String	FalseValue = "false";

	protected:
		virtual int PrintValue( bool AValue ) override
		{
			if( AValue )
				return FLcd->print( TrueValue );

			return FLcd->print( FalseValue );
		}

	public:
		LiquidCrystalElementDigitalField( Mitov::LiquidCrystalDisplay &AOwner ) :
			inherited( AOwner, 0 )
		{
		}

	};
//---------------------------------------------------------------------------
	// TODO: Implement setCursor() and createChar()
	class LiquidCrystalDisplay : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		OpenWire::SinkPin	ScrollLeftInputPin;
		OpenWire::SinkPin	ScrollRightInputPin;
		OpenWire::SinkPin	ClearInputPin;
		OpenWire::SinkPin	HomeInputPin;

	public:
		bool Enabled : 1;
		bool AutoScroll : 1;
		bool RightToLeft : 1;
		bool ShowCursor : 1;
		bool Blink : 1;

	protected:
		uint8_t FCols;
		uint8_t FRows;
		uint8_t FCursorLine = 0;
		uint8_t FCursorPos = 0;

	public: // Public for the print access
		LCD	*FLcd;
		Mitov::SimpleList<LiquidCrystalElementBasic *>	FElements;

	public:
		void ClearLine()
		{
			FLcd->setCursor( 0, FCursorLine );
			FCursorPos = 0;
//			for( int i = 0; i < FCols; ++ i )
//				FLcd->print( ' ' );

//			FLcd->setCursor( 0, FCursorLine );
		}

		void NewLine( int AClearSize )
		{
			for( int i = 0; i < FCols - FCursorPos - AClearSize; ++ i )
				FLcd->print( ' ' );

			++FCursorLine;
			FCursorPos = 0;
			if( FCursorLine >= FRows )
				FCursorLine = 0;

//			FLcd->setCursor( 0, FCursorLine );
		}

		void MoveChars( int ACount )
		{
			++FCursorPos;
		}

	public:
		void SetEnabled( bool AValue )
		{
			if( Enabled == AValue )
				return;

			Enabled = AValue;
			UpdateEnabled();
		}

		void SetAutoScroll( bool AValue )
		{
			if( AutoScroll == AValue )
				return;

			AutoScroll = AValue;
			UpdateAutoScroll();
		}

		void SetRightToLeft( bool AValue )
		{
			if( RightToLeft == AValue )
				return;

			RightToLeft = AValue;
			UpdateRightToLeft();
		}

		void SetShowCursor( bool AValue )
		{
			if( ShowCursor == AValue )
				return;

			ShowCursor = AValue;
			UpdateShowCursor();
		}

		void SetBlink( bool AValue )
		{
			if( Blink == AValue )
				return;

			Blink = AValue;
			UpdateBlink();
		}

	protected:
		void UpdateEnabled()
		{
			if( Enabled )
				FLcd->display();

			else
				FLcd->noDisplay();

		}

		void UpdateAutoScroll()
		{
			if( AutoScroll )
				FLcd->autoscroll();

			else
				FLcd->noAutoscroll();

		}

		void UpdateRightToLeft()
		{
			if( RightToLeft )
				FLcd->rightToLeft();

			else
				FLcd->leftToRight();

		}

		void UpdateShowCursor()
		{
			if( ShowCursor )
				FLcd->cursor();

			else
				FLcd->noCursor();

		}

		void UpdateBlink()
		{
			if( Blink )
				FLcd->blink();

			else
				FLcd->noBlink();

		}

		void DoScrollLeft( void * )
		{
			FLcd->scrollDisplayLeft();
		}

		void DoScrollRight( void * )
		{
			FLcd->scrollDisplayRight();
		}

		void DoClear( void * )
		{
			FLcd->clear();
		}

		void DoHome( void * )
		{
			FLcd->home();
		}

	public:
		virtual void SystemInit() override
		{
			inherited::SystemInit();

			FLcd->begin( FCols, FRows );
			UpdateEnabled();
			UpdateAutoScroll();
			UpdateRightToLeft();
			UpdateShowCursor();
			UpdateBlink();

			for( Mitov::SimpleList<LiquidCrystalElementBasic *>::iterator Iter = FElements.begin(); Iter != FElements.end(); ++Iter )
				( *Iter)->DisplayInit();

//			FLcd->setCursor(0,0);
		}

		virtual void SystemStart() override
		{
			inherited::SystemStart();

			for( Mitov::SimpleList<LiquidCrystalElementBasic *>::iterator Iter = FElements.begin(); Iter != FElements.end(); ++Iter )
				( *Iter)->DisplayStart();
		}

	public:
		LiquidCrystalDisplay( LCD *ALcd, unsigned int ACols, unsigned int ARows ) :
			FLcd( ALcd ),
			FCols( ACols ),
			FRows( ARows ),
			Enabled( true ),
			AutoScroll( false ),
			RightToLeft( false ),
			ShowCursor( false ),
			Blink( false )
		{
			ScrollLeftInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&LiquidCrystalDisplay::DoScrollLeft );
			ScrollRightInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&LiquidCrystalDisplay::DoScrollRight );
			ClearInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&LiquidCrystalDisplay::DoClear );
			HomeInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&LiquidCrystalDisplay::DoHome );
		}

/*
		virtual ~LiquidCrystalDisplay()
		{
			delete FLcd;
		}
*/
	};
//---------------------------------------------------------------------------
	class LiquidCrystalDisplayI2C : public LiquidCrystalDisplay
	{
		typedef Mitov::LiquidCrystalDisplay inherited;

	public:
		bool	Backlight = true;

	public:
		void SetBacklight( bool AValue )
		{
			if( Backlight == AValue )
				return;

			Backlight = AValue;
			UpdateBacklight();
		}

	public:
		virtual void SystemInit() override
		{
			inherited::SystemInit();
			UpdateBacklight();
		}

	public:
		void UpdateBacklight()
		{
			if( Backlight )
				inherited::FLcd->setBacklight( 255 );

			else
				inherited::FLcd->setBacklight( 0 );

		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class LiquidCrystalElementDefineCustomCharacter : public LiquidCrystalElementBasic
	{
		typedef Mitov::LiquidCrystalElementBasic inherited;

	protected:
		uint8_t FCharMap[ 8 ];

		uint8_t	FIndex;

	public:
		virtual void DisplayInit() override
		{
			FLcd->createChar( FIndex, FCharMap );
		}

	public:
		LiquidCrystalElementDefineCustomCharacter( Mitov::LiquidCrystalDisplay &AOwner, uint8_t AIndex, uint8_t Byte0, uint8_t Byte1, uint8_t Byte2, uint8_t Byte3, uint8_t Byte4, uint8_t Byte5, uint8_t Byte6, uint8_t Byte7 ) :
			inherited( AOwner ),
			FIndex( AIndex )
		{
			AOwner.FElements.push_back( this );

			FCharMap[ 0 ] = Byte0;
			FCharMap[ 1 ] = Byte1;
			FCharMap[ 2 ] = Byte2;
			FCharMap[ 3 ] = Byte3;
			FCharMap[ 4 ] = Byte4;
			FCharMap[ 5 ] = Byte5;
			FCharMap[ 6 ] = Byte6;
			FCharMap[ 7 ] = Byte7;
		}

	};
//---------------------------------------------------------------------------
	class LiquidCrystalElementCustomCharacterField : public LiquidCrystalElementBasicPositionedField
	{
		typedef Mitov::LiquidCrystalElementBasicPositionedField inherited;

	public:
		long	Index = 0;

	protected:
		bool	FModified = false;

	public:
		void SetIndex( long	AValue )
		{
			if( AValue > 7 )
				AValue = 7;

			else if( AValue < 0 )
				AValue = 0;

			if( Index == AValue )
				return;

			Index = AValue;
			FModified = true;
		}

		void SetColumn( unsigned long AValue )
		{
			if( AValue < 0 )
				AValue = 0;

			if( Column == AValue )
				return;

			Column = AValue;
			FModified = true;
		}

		void SetRow( unsigned long AValue )
		{
			if( AValue < 0 )
				AValue = 0;

			if( Row == AValue )
				return;

			Row = AValue;
			FModified = true;
		}

	public:
		virtual void DisplayStart() override
		{
			DisplayCharacter();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			inherited::SystemLoopBegin( currentMicros );
			if( FModified )
			{
				DisplayCharacter();
				FModified = false;
			}
		}

	protected:
		void DisplayCharacter()
		{
			FLcd->setCursor( Column, Row );
			FLcd->write( (uint8_t) Index );
		}

	public:
		LiquidCrystalElementCustomCharacterField( Mitov::LiquidCrystalDisplay &AOwner ) :
			inherited( AOwner )
		{
			AOwner.FElements.push_back( this );
		}

	};
//---------------------------------------------------------------------------
	template<typename T_LCD, T_LCD *T_LCD_INSTANCE, typename T> class LiquidCrystalDisplayInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			T_LCD_INSTANCE->ClearLine();
			int AClearSize = T_LCD_INSTANCE->FLcd->print( *(T*)_Data );
			T_LCD_INSTANCE->NewLine( AClearSize );
		}
	};
//---------------------------------------------------------------------------
	template<typename T_LCD, T_LCD *T_LCD_INSTANCE> class LiquidCrystalDisplayCharInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
//			Serial.print( (char*)_Data );
//			if( *(byte*)_Data < ' ' )
//				return;

			if( *(char*)_Data == '\r' )
				T_LCD_INSTANCE->NewLine( 0 );

			else if( *(char*)_Data == '\n' )
				T_LCD_INSTANCE->ClearLine();

			else if( *(char*)_Data >= ' ' )
			{
				int AClearSize = T_LCD_INSTANCE->FLcd->print( *(char*)_Data );
				T_LCD_INSTANCE->MoveChars( AClearSize );
			}
//			int AClearSize = T_LCD_INSTANCE->FLcd->print( (char*)_Data );
//			T_LCD_INSTANCE->NewLine( AClearSize );
		}
	};
//---------------------------------------------------------------------------
	template<typename T_LCD, T_LCD *T_LCD_INSTANCE> class LiquidCrystalDisplayStringInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data )
		{
//			Serial.print( (char*)_Data );
//			if( *(byte*)_Data < ' ' )
//				return;

			T_LCD_INSTANCE->ClearLine();
			int AClearSize = T_LCD_INSTANCE->FLcd->print( (char*)_Data );
			T_LCD_INSTANCE->NewLine( AClearSize );
		}
	};
//---------------------------------------------------------------------------
	template<typename T_LCD, T_LCD *T_LCD_INSTANCE, typename T_OBJECT> class LiquidCrystalDisplayObjectInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data )
		{
			T_LCD_INSTANCE->ClearLine();
			int AClearSize = T_LCD_INSTANCE->FLcd->print( ((T_OBJECT *)_Data)->ToString().c_str() );
			T_LCD_INSTANCE->NewLine( AClearSize );
		}
	};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
	LiquidCrystalElementBasic::LiquidCrystalElementBasic( LiquidCrystalDisplay &AOwner ) :
		FLcd( AOwner.FLcd )
	{
	}
//---------------------------------------------------------------------------
}

#endif

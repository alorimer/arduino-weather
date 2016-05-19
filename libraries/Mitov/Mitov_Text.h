////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_TEXT_h
#define _MITOV_TEXT_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class TextValue : public Mitov::CommonSource, public Mitov::ClockingSupport
	{
		typedef Mitov::CommonSource inherited;

	public:
		String Value;

	protected:
		virtual void SystemStart() override
		{
			inherited::SystemStart();
			OutputPin.Notify( (void *)Value.c_str() );
		}

		virtual void DoClockReceive( void *_Data ) override
		{
			OutputPin.Notify( (void *)Value.c_str() );
		}

	public:
		TextValue( char *AValue ) :
			Value( AValue )
		{
		}

	};
//---------------------------------------------------------------------------
	class BindableTextValue : public TextValue
	{
		typedef TextValue inherited;

	protected:
		String OldValue = inherited::Value;

	protected:
		virtual void SystemInit()
		{
			inherited::SystemInit();
			OldValue = inherited::Value;
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			if( inherited::Value == OldValue )
				return;

			OldValue = inherited::Value;
			inherited::OutputPin.Notify( (void *)OldValue.c_str() );
		}

	public:
		BindableTextValue( char *AValue ) :
			inherited( AValue ),
			OldValue( AValue )
		{
		}

	};
//---------------------------------------------------------------------------
	class BasicFormattedText;
//---------------------------------------------------------------------------
	class FormattedTextElementBasic : public OpenWire::Object // : public OpenWire::Component
	{
//		typedef OpenWire::Component inherited;
	protected:
		BasicFormattedText &FOwner;

	public:
		virtual String GetText() = 0;

		virtual void SystemStart()
		{
		}

	public:
		FormattedTextElementBasic( BasicFormattedText &AOwner );

	};
//---------------------------------------------------------------------------
	class BasicFormattedText : public Mitov::CommonSource, public Mitov::ClockingSupport
	{
		typedef Mitov::CommonSource inherited;

	public:
		Mitov::SimpleList<FormattedTextElementBasic *>	FElements;

	protected:
		bool FModified = false;

	public:
		inline void SetModified()
		{
			FModified = true;
		}

	};
//---------------------------------------------------------------------------
	template <typename T_STRING> class BasicTypedFormattedText : public Mitov::BasicFormattedText
	{
		typedef Mitov::BasicFormattedText inherited;

	protected:
		struct TStringItem
		{
			T_STRING	Text;
			FormattedTextElementBasic *Element;
		};

	protected:
		Mitov::SimpleList<TStringItem *>	FReadyElements;

	public:
		void AddReadyElement( T_STRING ATextItem, int AIndex )
		{
//			Serial.print( "AddElement: " ); Serial.print( ATextItem ); Serial.println( AIndex );

			TStringItem	*AItem = new TStringItem;
			AItem->Text = ATextItem;
//			Serial.print( "AddElement: " ); Serial.println( AItem->Text );
			if( AIndex < FElements.size() )
				AItem->Element = FElements[ AIndex ];

			else
				AItem->Element = NULL;

			FReadyElements.push_back( AItem );

//			Serial.println( FReadyElements[ FReadyElements.size() - 1 ]->Text );
//			Serial.println( "DEBUG>>" );
//			for( Mitov::SimpleList<TStringItem *>::iterator Iter = FReadyElements.begin(); Iter != FReadyElements.end(); ++Iter )
//				Serial.println( ( *Iter )->Text );

//			Serial.println( "<<DEBUG" );
		}

	protected:
		void ProcessSendOutput()
		{
//			Serial.println( "ProcessSendOutput" );
			String AText;
			for( typename Mitov::SimpleList<TStringItem *>::iterator Iter = FReadyElements.begin(); Iter != FReadyElements.end(); ++Iter )
			{
//				Serial.println( ( *Iter )->Text );
				AText += ( *Iter )->Text;
				if( ( *Iter )->Element )
					AText += ( *Iter )->Element->GetText();
			}

//			Serial.println( AText );
			inherited::OutputPin.Notify( (void *)AText.c_str() );
			FModified = false;
		}

	protected:
		virtual void SystemLoopEnd() override
		{
			if( FModified )
				if( ! ClockInputPin.IsConnected() )
					ProcessSendOutput();

			inherited::SystemLoopEnd();
		}

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			ProcessSendOutput();
		}

	};
//---------------------------------------------------------------------------
	class FormattedText_Fixed : public Mitov::BasicTypedFormattedText<char *>
	{
		typedef Mitov::BasicTypedFormattedText<char *> inherited;

	public:
		void AddNullElement( char *ATextItem )
		{
			TStringItem	*AItem = new TStringItem;

			AItem->Text = ATextItem;
			AItem->Element = NULL;

			inherited::FReadyElements.push_back( AItem );
		}

	protected:
		virtual void SystemStart() override
		{
			for( Mitov::SimpleList<FormattedTextElementBasic *>::iterator Iter = FElements.begin(); Iter != FElements.end(); ++Iter )
				(*Iter )->SystemStart();
			
			inherited::SystemStart();
			ProcessSendOutput();
		}

	};
//---------------------------------------------------------------------------
	class FormattedText : public Mitov::BasicTypedFormattedText<String>
	{
		typedef Mitov::BasicTypedFormattedText<String> inherited;

	public:
		String Text;

	protected:
		void InitElements()
		{
			FReadyElements.clear();
			String	ATextItem;
			String	AIndexText;
			bool	AInEscape = false;

//			Serial.println( "INIT" );
//			Serial.println( Text );
//			delay( 1000 );

			for( int i = 0; i < Text.length(); ++ i )
			{
				char AChar = Text[ i ];
				if( AInEscape )
				{
					if( AChar >= '0' && AChar <= '9' )
						AIndexText += AChar;

					else
					{
						if( AChar == '%' )
						{
							if( AIndexText.length() == 0 )
								ATextItem += '%';

							else
							{
								AddReadyElement( ATextItem, AIndexText.toInt() );
								ATextItem = "";
							}

						}

						else
						{
							if( AIndexText.length() == 0 )
								ATextItem += '%';

							else
							{
								AddReadyElement( ATextItem, AIndexText.toInt() );
								ATextItem = "";
							}

							ATextItem += AChar;
						}

						AInEscape = false;
					}
				}

				else
				{
					if( AChar == '%' )
					{
						AInEscape = true;
						AIndexText = "";
					}

					else
						ATextItem += AChar;

				}

			}

			if( AInEscape )
				AddReadyElement( ATextItem, AIndexText.toInt() );

			else if( ATextItem.length() )
			{
				TStringItem	*AItem = new TStringItem;

				AItem->Text = ATextItem;
				AItem->Element = NULL;

				FReadyElements.push_back( AItem );
			}

//			Serial.println( "DEBUG>>" );
//			for( Mitov::SimpleList<TStringItem *>::iterator Iter = FReadyElements.begin(); Iter != FReadyElements.end(); ++Iter )
//				Serial.println( ( *Iter )->Text );

//			Serial.println( "<<DEBUG" );
		}

	protected:
		virtual void SystemStart() override
		{
			for( Mitov::SimpleList<FormattedTextElementBasic *>::iterator Iter = FElements.begin(); Iter != FElements.end(); ++Iter )
				(*Iter )->SystemStart();
			
			InitElements();
			inherited::SystemStart();
			ProcessSendOutput();
		}

	};
//---------------------------------------------------------------------------
	template<typename T_LCD, T_LCD *T_LCD_INSTANCE, typename T> class TextFormatElementInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			T_LCD_INSTANCE->SetValue( String( *(T*)_Data ));
		}
	};
//---------------------------------------------------------------------------
	template<typename T_LCD, T_LCD *T_LCD_INSTANCE> class TextFormatElementStringInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			T_LCD_INSTANCE->SetValue( (char*)_Data );
		}
	};
//---------------------------------------------------------------------------
	template<typename T_LCD, T_LCD *T_LCD_INSTANCE, typename T_OBJECT> class TextFormatElementObjectInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			T_LCD_INSTANCE->SetValue( ((T_OBJECT *)_Data)->ToString() );
		}
	};
//---------------------------------------------------------------------------
	class FormattedTextElementText : public FormattedTextElementBasic
	{
		typedef Mitov::FormattedTextElementBasic inherited;

	public:
		String	InitialValue;
		String	FValue;

	public:
		void SetValue( String AValue )
		{
			FOwner.SetModified();
			FValue = AValue;
		}

	public:
		virtual String GetText()
		{
			return FValue;
		}

	public:
		virtual void SystemStart()
		{
//			inherited::SystemStart();
			FValue = InitialValue;
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class FormattedTextInputElement : public FormattedTextElementBasic
	{
		typedef Mitov::FormattedTextElementBasic inherited;

	public:
		OpenWire::SinkPin	InputPin;

	protected:
		virtual void DoReceive( void *_Data ) = 0;

	public:
		FormattedTextInputElement( BasicFormattedText &AOwner ) :
			inherited( AOwner )
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&FormattedTextInputElement::DoReceive );
		}

	};
//---------------------------------------------------------------------------
	template<typename T> class FormattedTextElementTyped : public FormattedTextInputElement
	{
		typedef Mitov::FormattedTextInputElement inherited;

	public:
		T	InitialValue;
		T	FValue;

	public:
		virtual void SystemStart()
		{
//			inherited::SystemStart();
			FValue = InitialValue;
		}

	protected:
		virtual void DoReceive( void *_Data )
		{
			FOwner.SetModified();
			FValue = *(T *)_Data;
			InitialValue = FValue;
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class FormattedTextElementInteger : public Mitov::FormattedTextElementTyped<long>
	{
		typedef Mitov::FormattedTextElementTyped<long> inherited;

	public:
		int	Base = 10;

	public:
		virtual String GetText()
		{
			char AText[ 50 ];
			itoa( FValue, AText, Base );

			return AText;
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class FormattedTextElementAnalog : public Mitov::FormattedTextElementTyped<float>
	{
		typedef Mitov::FormattedTextElementTyped<float> inherited;

	public:
		int	MinWidth = 1;
		int	Precision = 3;

	public:
		virtual String GetText()
		{
			char AText[ 50 ];
			dtostrf( FValue,  MinWidth, Precision, AText );

			return AText;
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class FormattedTextElementDigital : public Mitov::FormattedTextElementTyped<bool>
	{
		typedef Mitov::FormattedTextElementTyped<bool> inherited;

	public:
		String	TrueValue = "true";
		String	FalseValue = "false";

	public:
		virtual String GetText()
		{
			if( FValue )
				return TrueValue;

			return FalseValue;
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
	FormattedTextElementBasic::FormattedTextElementBasic( BasicFormattedText &AOwner ) :
		FOwner( AOwner )
	{
		AOwner.FElements.push_back( this );
	}
//---------------------------------------------------------------------------
}

#endif

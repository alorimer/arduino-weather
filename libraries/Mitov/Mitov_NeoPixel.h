////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_NEO_PIXEL_h
#define _MITOV_NEO_PIXEL_h

#include <Mitov.h>

#include <MitovEmbedded_Adafruit_NeoPixel/MitovEmbedded_Adafruit_NeoPixel.h>

namespace Mitov
{
	class NeoPixelsRunningColorGroup;
//---------------------------------------------------------------------------
	class NeoPixelsController
	{
	public:
		virtual void SetPixelColor( int AIndex, TColor AColor ) = 0;
		virtual TColor GetPixelColor( int AIndex ) = 0;

	};
//---------------------------------------------------------------------------
	class NeoPixelsCommonGroup : public OpenWire::Component
	{
	public:
		NeoPixelsController	*FOwner;
		int			FStartPixel;

	public:
		virtual void StartPixels( NeoPixelsController *AOwner, int &AStartPixel )
		{
			FOwner = AOwner;
			FStartPixel = AStartPixel;
		}

		virtual void PixelsClock( unsigned long currentMicros )
		{
		}

	};
//---------------------------------------------------------------------------
	class NeoPixelsBasicGroup : public NeoPixelsCommonGroup
	{
		typedef NeoPixelsCommonGroup inherited;

	public:
		int		CountPixels = 10;

	public:
		virtual void StartPixels( NeoPixelsController *AOwner, int &AStartPixel )
		{
			inherited::StartPixels( AOwner, AStartPixel );
			AStartPixel += CountPixels;
		}

	};
//---------------------------------------------------------------------------
	class NeoPixels : public OpenWire::Component, public NeoPixelsController, public Mitov::ClockingSupport
	{
		typedef OpenWire::Component inherited;

	public:
		float   Brightness = 1.0f;
		Mitov::SimpleObjectList<NeoPixelsCommonGroup*>	PixelGroups;

	public:
		bool	FModified;

	protected:
		int	FPinNumber;

	public:
		void SetBrightness( float AValue )
		{
			if( Brightness == AValue )
				return;

			IntSetBrightness( AValue );
		}

	public:
		void SetPixelColor( int AIndex, TColor AColor )
		{
			FPixel.setPixelColor( AIndex, AColor.Red, AColor.Green, AColor.Blue );
			FModified = true;
		}

		TColor GetPixelColor( int AIndex )
		{
			return TColor( FPixel.getPixelColor( AIndex ), true );
		}

	protected:
		MitovEmbedded_Adafruit_NeoPixel	&FPixel;

	protected:
		void IntSetBrightness( float AValue )
		{
			Brightness = AValue;
			FPixel.setBrightness( AValue * 255 );
			FModified = true;
		}

		virtual void SystemInit()
		{
			FPixel.setPin( FPinNumber );

			FPixel.begin();
			IntSetBrightness( Brightness );

			int AStartPixel = 0;
			for( int i = 0; i < PixelGroups.size(); ++i )
				PixelGroups[ i ]->StartPixels( this, AStartPixel );
			

			inherited::SystemInit();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			for( int i = 0; i < PixelGroups.size(); ++i )
				PixelGroups[ i ]->PixelsClock( currentMicros );

			inherited::SystemLoopBegin( currentMicros );
		}

		virtual void SystemLoopEnd()
		{
			if( FModified )
				if( ! ClockInputPin.IsConnected())
				{
					FPixel.show();
					FModified = false;
				}

			inherited::SystemLoopEnd();
		}

		virtual void DoClockReceive(void *) override
		{
			if( FModified )
			{
				FPixel.show();
				FModified = false;
			}
		}

	public:
		NeoPixels( int APinNumber, MitovEmbedded_Adafruit_NeoPixel &APixel ) :
			FPinNumber( APinNumber ),
		    FPixel( APixel )
		{
		}

	};
//---------------------------------------------------------------------------
	class NeoPixelsBasicInitialColorGroup : public NeoPixelsBasicGroup
	{
		typedef NeoPixelsBasicGroup inherited;

	public:
		TColor	InitialColor;

		virtual void StartPixels( NeoPixelsController *AOwner, int &AStartPixel )
		{
			inherited::StartPixels( AOwner, AStartPixel );

			for( int i = 0; i < CountPixels; ++i )
				FOwner->SetPixelColor( FStartPixel + i, InitialColor );
		}

	};
//---------------------------------------------------------------------------
	class NeoPixelsBasicColorGroup : public NeoPixelsBasicInitialColorGroup
	{
		typedef NeoPixelsBasicInitialColorGroup inherited;

	protected:
		TColor	FColor;

	public:
		OpenWire::SinkPin	ColorInputPin;

	public:
		void SetInitialColor( TColor AValue )
		{
			if( InitialColor == AValue )
				return;

			InitialColor = AValue;
			FColor = AValue;
			ApplyColorsAll(); 
//			FOwner->FModified = true;
		}

	protected:
		virtual void ApplyColorsAll()
		{
			for( int i = 0; i < CountPixels; ++i )
				FOwner->SetPixelColor( FStartPixel + i, FColor );
		}

		virtual void ApplyColors() {}

		void IntSetColor( TColor AValue )
		{
			if( FColor == AValue )
				return;

			FColor = AValue;
			ApplyColors(); 
//			FOwner->FModified = true;
		}

		void DoReceiveColor( void *_Data )
		{
			IntSetColor( *(TColor *)_Data );
		}

	protected:
		virtual void StartPixels( NeoPixelsController *AOwner, int &AStartPixel )
		{
			inherited::StartPixels( AOwner, AStartPixel );
//			Serial.println( CountPixels );
			IntSetColor( InitialColor );
//			FOwner->FModified = true;
//			Serial.println( FStartPixel );
		}

	public:
		NeoPixelsBasicColorGroup()
		{
			ColorInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&NeoPixelsBasicColorGroup::DoReceiveColor );
		}

	};
//---------------------------------------------------------------------------
	class NeoPixelsGroup : public NeoPixelsBasicInitialColorGroup
	{
		typedef NeoPixelsBasicInitialColorGroup inherited;

	protected:
		class PixelVlaueSinkPin : public OpenWire::VlaueSinkPin<TColor>
		{
			typedef OpenWire::VlaueSinkPin<TColor> inherited;

		public:
			NeoPixelsGroup *FOwner;
			int				FIndex;

		public:
			virtual void Receive( void *_Data )
			{
				TColor AValue = *(TColor *)_Data;
				if( AValue != Value )
					FOwner->FOwner->SetPixelColor( FIndex, AValue );

				inherited::Receive( _Data );
			}
		};

	public:
		Mitov::SimpleList<PixelVlaueSinkPin> InputPins;

	protected:
		virtual void StartPixels( NeoPixelsController *AOwner, int &AStartPixel )
		{
			inherited::StartPixels( AOwner, AStartPixel );
			for( int i = 0; i < InputPins.size(); ++i )
			{
				InputPins[ i ].FOwner = this;
				InputPins[ i ].FIndex = FStartPixel + i;
				InputPins[ i ].Value = InitialColor;
//				FOwner->SetPixelColor( FStartPixel + i, InitialColor );
//				Iter->SetCallback( this, (OpenWire::TOnPinReceive)&NeoPixelsGroup::DoReceive );
//				Iter->Value = T_VALUE;
			}

		}

	};
//---------------------------------------------------------------------------
	class NeoPixelsRepeatGroup : public NeoPixelsBasicInitialColorGroup, public NeoPixelsController
	{
		typedef NeoPixelsBasicInitialColorGroup inherited;

	public:
		Mitov::SimpleObjectList<NeoPixelsCommonGroup*>	PixelGroups;

	protected:
		int FSubPixelCount;
		int FRepeatCount;

	public:
		virtual void SetPixelColor( int AIndex, TColor AColor )
		{
			for( int i = 0; i < FRepeatCount; ++i )
				FOwner->SetPixelColor( FStartPixel + AIndex + i * FSubPixelCount, AColor );
		}

		virtual TColor GetPixelColor( int AIndex )
		{
			return FOwner->GetPixelColor( FStartPixel + AIndex );
		}

	public:
		virtual void StartPixels( NeoPixelsController *AOwner, int &AStartPixel ) 
		{
			inherited::StartPixels( AOwner, AStartPixel );

			FSubPixelCount = 0;
			for( int i = 0; i < PixelGroups.size(); ++i )
				PixelGroups[ i ]->StartPixels( this, FSubPixelCount );

			if( FSubPixelCount == 0 )
				FRepeatCount = 0;

			else
				FRepeatCount = ( CountPixels + FSubPixelCount - 1 ) / FSubPixelCount;

		}

		virtual void PixelsClock( unsigned long currentMicros )
		{
			inherited::PixelsClock( currentMicros );

			for( int i = 0; i < PixelGroups.size(); ++i )
				PixelGroups[ i ]->PixelsClock( currentMicros );
		}
	};
//---------------------------------------------------------------------------
	class NeoPixelsSingleColorGroup : public NeoPixelsBasicColorGroup
	{
		typedef NeoPixelsBasicColorGroup inherited;

	protected:
		virtual void ApplyColors() override
		{
			ApplyColorsAll();
		}

	};
//---------------------------------------------------------------------------
	class NeoPixelsReversedProperty
	{
	protected:
		NeoPixelsRunningColorGroup &FOwner;

	public:
		bool Reversed : 1;
		bool AllPixels : 1;

	public:
		void SetReversed( bool AValue );

	public:
		NeoPixelsReversedProperty( NeoPixelsRunningColorGroup &AOwner ) :
			FOwner( AOwner ),
			Reversed( false ),
			AllPixels( false )
		{
		}
	};
//---------------------------------------------------------------------------
	class NeoPixelsRunningColorGroup : public NeoPixelsBasicColorGroup
	{
		typedef NeoPixelsBasicColorGroup inherited;

	public:
		OpenWire::ConnectSinkPin	StepInputPin;
		OpenWire::SourcePin	ColorOutputPin;

	public:
		NeoPixelsReversedProperty	Reversed;

	public:
		void ReversePixels()
		{
			for( int i = 0; i < CountPixels / 2; ++i )
			{
				TColor AOldColor1 = FOwner->GetPixelColor( FStartPixel + ( CountPixels - i - 1 ));
				TColor AOldColor2 = FOwner->GetPixelColor( FStartPixel + i );

				FOwner->SetPixelColor( FStartPixel + i, AOldColor1 );
				FOwner->SetPixelColor( FStartPixel + ( CountPixels - i - 1 ), AOldColor2 );
			}
		}

	protected:
		void AnimatePixels()
		{
			if( Reversed.Reversed )
			{
				TColor AOldColor = FOwner->GetPixelColor( FStartPixel );
				ColorOutputPin.Notify( &AOldColor );
				for( int i = 0; i < CountPixels - 1; ++i )
				{
					AOldColor = FOwner->GetPixelColor( FStartPixel + i + 1 );
					FOwner->SetPixelColor( FStartPixel + i, AOldColor );
				}

				FOwner->SetPixelColor( FStartPixel + CountPixels - 1, FColor );
			}

			else
			{
				TColor AOldColor = FOwner->GetPixelColor( FStartPixel + CountPixels - 1 );
				ColorOutputPin.Notify( &AOldColor );
				for( int i = CountPixels - 1; i--; )
				{
					AOldColor = FOwner->GetPixelColor( FStartPixel + i );
					FOwner->SetPixelColor( FStartPixel + i + 1, AOldColor );
				}

				FOwner->SetPixelColor( FStartPixel, FColor );
			}
//			FOwner->FModified = true;
		}

		void DoReceiveStep( void *_Data )
		{
			AnimatePixels();
		}

	protected:
		virtual void PixelsClock( unsigned long currentMicros ) override
		{
			if( StepInputPin.IsConnected())
				return;

			AnimatePixels();
		}

	public:
		NeoPixelsRunningColorGroup() :
		  Reversed( *this )
		{
			StepInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&NeoPixelsRunningColorGroup::DoReceiveStep );
		}
	};
//---------------------------------------------------------------------------
	inline void NeoPixelsReversedProperty::SetReversed( bool AValue )
	{
		if( Reversed == AValue )
			return;

		Reversed = AValue;
		if( AllPixels )
			FOwner.ReversePixels();

	}
//---------------------------------------------------------------------------
	class NeoPixelsColorPixelGroup : public NeoPixelsBasicColorGroup
	{
	public:
		OpenWire::SinkPin	IndexInputPin;

	public:
		uint32_t InitialIndex = 0;

	protected:
		unsigned long	FIndex;
		bool			FModified;

	protected:
		void DoReceiveIndex( void *_Data )
		{
			unsigned long AIndex = *(unsigned long *)_Data;
			if( AIndex > CountPixels )
				AIndex = CountPixels;

			if( FIndex == AIndex )
				return;

			FIndex = AIndex;
			FModified = true;
		}

		virtual void PixelsClock( unsigned long currentMicros )
		{
			if( FModified )
			{
				FOwner->SetPixelColor( FStartPixel + FIndex, FColor );
//				FOwner->FModified = true;
				FModified = false;
			}
		}

		virtual void ApplyColors() 
		{
			FModified = true;
		}

	public:
		NeoPixelsColorPixelGroup()
		{
			IndexInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&NeoPixelsColorPixelGroup::DoReceiveIndex );
		}
	};
//---------------------------------------------------------------------------
}

#endif

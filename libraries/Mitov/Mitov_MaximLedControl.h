////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_MAXIM_LED_CONTROL_h
#define _MITOV_MAXIM_LED_CONTROL_h

#include <Mitov.h>
#include <Mitov_Basic_SPI.h>
//#include <Mitov_7Segment_Common.h>

namespace Mitov
{
	const byte C_MaximSegments[16] =
	{
		0b1111110,  // = 0
		0b0110000,  // = 1
		0b1101101,  // = 2
		0b1111001,  // = 3
		0b0110011,  // = 4
		0b1011011,  // = 5
		0b1011111,  // = 6
		0b1110000,  // = 7
		0b1111111,  // = 8
		0b1111011,  // = 9
		0b1110111,  // = A
		0b0011111,  // = B
		0b0001101,  // = C
		0b0111101,  // = D
		0b1001111,  // = E
		0b1000111   // = F
	};
//---------------------------------------------------------------------------
	const byte C_MaximDecimalPoint[2] =
	{
		0b00000000,
		0b10000000
	};
//---------------------------------------------------------------------------
	class MaximLedCommonGroup;
//---------------------------------------------------------------------------
	class MaximLedGroupOwner
	{
	public:
		Mitov::SimpleObjectList<MaximLedCommonGroup*>	FPixelGroups;

	public:
		virtual void SetPixelValue( int AIndex, bool AValue ) = 0;
		virtual bool GetPixelValue( int AIndex ) = 0;

	};
//---------------------------------------------------------------------------
	class MaximLedCommonGroup : public OpenWire::Component
	{
	protected:
		MaximLedGroupOwner &FOwner;

		bool	Enabled = true;

	public:
		void SetEnabled( bool AValue )
		{
			if( Enabled == AValue )
				return;

			Enabled = AValue;
			UpdateDisplay();
		}

	public:
		virtual void StartPixels( int &AStartPixel ) = 0;
		virtual void PixelsClock( unsigned long currentMicros )
		{
		}

	protected:
		virtual void UpdateDisplay() {}

	public:
		MaximLedCommonGroup( MaximLedGroupOwner &AOwner ) :
			FOwner( AOwner )
		{
			FOwner.FPixelGroups.push_back( this );
		}
	};
//---------------------------------------------------------------------------
	template<int COUNT_CONTROLLERS> class MaximLedController : public Mitov::Basic_SPI, public Mitov::ClockingSupport, public Mitov::MaximLedGroupOwner
	{
		typedef Mitov::Basic_SPI inherited;

	protected:
		static const	byte OP_DIGIT0 = 1;
		static const	byte OP_DECODEMODE = 9;
		static const	byte OP_INTENSITY = 10;
		static const	byte OP_SCANLIMIT = 11;
		static const	byte OP_SHUTDOWN = 12;
		static const	byte OP_DISPLAYTEST = 15;

	protected:
//		int		FNumDevices;

		byte	FRegistersCurrent[ COUNT_CONTROLLERS * 8 ];
		byte	FRegisters[ COUNT_CONTROLLERS * 8 ];

		bool	FModified = false;

	public:
		float	Intensity = 1.0f;

	protected:
		void SetIntensity( float AValue )
		{
			if( Intensity == AValue )
				return;

			Intensity = AValue;
			UpdateIntensity();
		}

		void SetEnabled( bool AValue )
		{
			if( Enabled == AValue )
				return;

			Enabled = AValue;
			UpdateShutDown();
		}

	public:
		virtual void SetPixelValue( int AIndex, bool AValue )
		{
			if( AValue )
				FRegisters[ AIndex / 8 ] |= 1 << ( AIndex & 7 );

			else
				FRegisters[ AIndex / 8 ] &= ~( 1 << ( AIndex & 7 ));

			FModified = true;
		}

		virtual bool GetPixelValue( int AIndex )
		{
			return FRegisters[ AIndex / 8 ] & ( 1 << ( AIndex & 7 ));
		}

	protected:
		void spiTransferAll( byte opcode, byte data )
		{
			//enable the line 
			ChipSelectOutputPin.SendValue( false );
			//Now shift out the data 

			for(int i = 0; i < COUNT_CONTROLLERS; ++i )
			{    
				FSPI.transfer( opcode );
				FSPI.transfer( data );
			}
    
			//latch the data onto the display
			ChipSelectOutputPin.SendValue( true );
		}

/*
		void spiTransfer( int addr, byte opcode, byte data )
		{
			//enable the line 
			ChipSelectOutputPin.SendValue( false );
			//Now shift out the data 

			for(int i = 1; i < COUNT_CONTROLLERS; ++i )
				FSPI.transfer16( 0 ); // Skip one chip
      
			FSPI.transfer( opcode );
			FSPI.transfer( data );
    
			//latch the data onto the display
			ChipSelectOutputPin.SendValue( true );
		}    
*/
	protected:
		inline void UpdateIntensity()
		{
			spiTransferAll( OP_INTENSITY, Intensity * 15 + 0.5 );
		}

		inline void UpdateShutDown()
		{
			spiTransferAll( OP_SHUTDOWN, Enabled );
		}

		void UpdatePixels()
		{
			uint16_t ABuffer[ COUNT_CONTROLLERS ];
			FModified = false;

			for(;;)
			{
				bool HasChange = false;
				for( int AControllerIndex = 0; AControllerIndex < COUNT_CONTROLLERS; ++AControllerIndex )
				{
					ABuffer[ AControllerIndex ] = 0;
					for( int j = 0; j < 8; ++j )
					{
						int ARegIndex = AControllerIndex * 8 + j;
						uint16_t ANewRegValue = FRegisters[ ARegIndex ];
						if( FRegistersCurrent[ ARegIndex ] != ANewRegValue )
						{
							FRegistersCurrent[ ARegIndex ] = ANewRegValue;
							ABuffer[ AControllerIndex ] = ( ANewRegValue << 8 ) | ( OP_DIGIT0 + j );
							HasChange = true;
							break;
						}
					}
				}

				if( !HasChange )
					break;

				//enable the line 
				ChipSelectOutputPin.SendValue( false );
				FSPI.transfer( ABuffer, COUNT_CONTROLLERS * 2 );

				//latch the data onto the display
				ChipSelectOutputPin.SendValue( true );
			}
		}

	protected:
		virtual void SystemInit()
		{
			int AStartPixel = 0;
			for( int i = 0; i < FPixelGroups.size(); ++i )
				FPixelGroups[ i ]->StartPixels( AStartPixel );	

//			FNumDevices = ( AStartPixel + 63 ) / 64;

//			FRegisters.SetCount( FNumDevices * 8, 0 );
//			FRegistersCurrent.SetCount( FNumDevices * 8, 0 );

			spiTransferAll( OP_DISPLAYTEST, 0 );
			spiTransferAll( OP_SCANLIMIT, 7 );
			spiTransferAll( OP_DECODEMODE, 0 );

			UpdateShutDown();
			UpdateIntensity();

//			if( FModified )


			for( int i = 0; i < COUNT_CONTROLLERS * 8; ++i )
				FRegistersCurrent[ i ] = FRegisters[ i ] + 1;

			UpdatePixels();

//			spiTransferAll( OP_DIGIT0, 255 );
			inherited::SystemInit();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			for( int i = 0; i < FPixelGroups.size(); ++i )
				FPixelGroups[ i ]->PixelsClock( currentMicros );

			inherited::SystemLoopBegin( currentMicros );
		}

		virtual void SystemLoopEnd()
		{
			if( FModified )
				if( ! ClockInputPin.IsConnected())
					UpdatePixels();

			inherited::SystemLoopEnd();
		}

	protected:
		virtual void DoClockReceive( void * )
		{
			if( FModified )
				UpdatePixels();
		}

	public:
		MaximLedController( BasicSPI &ASPI ) :
			inherited( ASPI )
		{
			for( int i = 0; i < COUNT_CONTROLLERS * 8; ++i )
				FRegisters[ i ] = 0;
		}
	};
//---------------------------------------------------------------------------
	class MaximLedBasicGroup : public MaximLedCommonGroup
	{
		typedef Mitov::MaximLedCommonGroup inherited;

	public:
		int		CountPixels = 64;

	public:
		virtual void StartPixels( int &AStartPixel )
		{
//			inherited::StartPixels( AStartPixel );
			AStartPixel += CountPixels;
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MaximLedBasicInitialValueGroup : public MaximLedBasicGroup
	{
		typedef Mitov::MaximLedBasicGroup inherited;

	public:
		bool	InitialValue = false;

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MaximLedGroupBasicBooleanPixels : public MaximLedBasicInitialValueGroup
	{
		typedef Mitov::MaximLedBasicInitialValueGroup inherited;

	protected:
		class PixelVlaueSinkPin : public OpenWire::VlaueSinkPin<bool>
		{
			typedef OpenWire::VlaueSinkPin<bool> inherited;

		public:
			MaximLedGroupBasicBooleanPixels	*FOwner;
			int								 FIndex;

		public:
			virtual void Receive( void *_Data )
			{
				bool AValue = *(bool *)_Data;
				if( AValue != Value )
					if( FOwner->Enabled )
						FOwner->FOwner.SetPixelValue( FIndex, AValue );

				inherited::Receive( _Data );
			}
		};

	protected:
		virtual void UpdateDisplay()
		{
			for( int i = 0; i < InputPins.size(); ++i )
			{
				if( Enabled )
					FOwner.SetPixelValue( InputPins[ i ].FIndex, InputPins[ i ].Value );

				else
					FOwner.SetPixelValue( InputPins[ i ].FIndex, false );
			}
		}

	public:
		Mitov::SimpleList<PixelVlaueSinkPin> InputPins;

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MaximLedGroup : public MaximLedGroupBasicBooleanPixels
	{
		typedef Mitov::MaximLedGroupBasicBooleanPixels inherited;

	public:
		virtual void StartPixels( int &AStartPixel )
		{
			for( int i = 0; i < InputPins.size(); ++i )
			{
				InputPins[ i ].FOwner = this;
				InputPins[ i ].FIndex = AStartPixel + i;
				InputPins[ i ].Value = InitialValue;
				FOwner.SetPixelValue( AStartPixel + i, InitialValue );
			}

			inherited::StartPixels( AStartPixel );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MaximLedGroupSection7Segments : public MaximLedGroupBasicBooleanPixels
	{
		typedef Mitov::MaximLedGroupBasicBooleanPixels inherited;

	public:
		virtual void StartPixels( int &AStartPixel )
		{
			for( int i = 0; i < 7; ++i )
			{
				InputPins[ i ].FOwner = this;
				InputPins[ i ].FIndex = AStartPixel + 6 - i;
				InputPins[ i ].Value = InitialValue;
				FOwner.SetPixelValue( AStartPixel + 6 - i, InitialValue );
			}

			InputPins[ 7 ].FOwner = this;
			InputPins[ 7 ].FIndex = AStartPixel + 7;
			InputPins[ 7 ].Value = InitialValue;
			FOwner.SetPixelValue( AStartPixel + 7, InitialValue );

			inherited::StartPixels( AStartPixel );
		}

	public:
		MaximLedGroupSection7Segments( MaximLedGroupOwner &AOwner ) :
			inherited( AOwner )
		{
			InputPins.SetCount( 8 );
			CountPixels = 8;
		}
	};
//---------------------------------------------------------------------------
	class MaximLedGroupValueSection7Segments : public MaximLedCommonGroup
	{
		typedef Mitov::MaximLedCommonGroup inherited;

	public:
		OpenWire::SinkPin	InputPin;
		OpenWire::SinkPin	DecmalPointInputPin;

	public:
		int		InitialValue = 0;
		bool	InitialDecmalPointValue = false;

	protected:
		int		FValue;
		bool	FDecmalPointValue;

		int		FStartPixel;

	public:
		virtual void StartPixels( int &AStartPixel )
		{
//			inherited::StartPixels( AStartPixel );
			FStartPixel = AStartPixel;
			AStartPixel += 8;
		}

	protected:
		virtual void UpdateDisplay()
		{
			int AValue = (Enabled) ? C_MaximSegments[ FValue & 0xF ] | C_MaximDecimalPoint[ FDecmalPointValue & 1 ] : 0;

			for( int i = 0; i < 8; ++i )
				FOwner.SetPixelValue( FStartPixel + i, ( AValue >> i ) & 1 );
		}

	protected:
		void DoReceive( void *_Data )
		{
			int AValue = *(int *)_Data;
			if( FValue == AValue )
				return;

			FValue = AValue;
			UpdateDisplay();
		}

		void DoReceiveDecmalPoint( void *_Data )
		{
			bool AValue = *(bool *)_Data;
			if( FDecmalPointValue == AValue )
				return;

			FDecmalPointValue = AValue;
			UpdateDisplay();
		}

		virtual void SystemStart() 
		{
			FValue = InitialValue;
			FDecmalPointValue = InitialDecmalPointValue;
			inherited::SystemStart();
			UpdateDisplay();
		}

	public:
		MaximLedGroupValueSection7Segments( MaximLedGroupOwner &AOwner ) :
			inherited( AOwner )
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MaximLedGroupValueSection7Segments::DoReceive );
			DecmalPointInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MaximLedGroupValueSection7Segments::DoReceiveDecmalPoint );
		}
	};
//---------------------------------------------------------------------------
	class MaximLedGroupBasicMultiDigitDisplay7Segments : public MaximLedCommonGroup
	{
		typedef Mitov::MaximLedCommonGroup inherited;

	public:
		uint32_t CountDigits = 8;
		bool	ReversedOrder = false;

	protected:
		int	FStartPixel;

	public:
		virtual void StartPixels( int &AStartPixel )
		{
//			inherited::StartPixels( AStartPixel );
			FStartPixel = AStartPixel;
			AStartPixel += CountDigits * 8;
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	template<typename T_DATA> class MaximLedGroupTypedValueDisplay7Segments : public MaximLedGroupBasicMultiDigitDisplay7Segments
	{
		typedef Mitov::MaximLedGroupBasicMultiDigitDisplay7Segments inherited;

	public:
		OpenWire::SinkPin	InputPin;

	public:
		T_DATA	InitialValue;

	protected:
		T_DATA	FValue;

	protected:
		virtual byte GetSegmentsValue( int &ADigit ) = 0;

		virtual void UpdateDisplay() override
		{
			for( int ADigit = 0; ADigit < CountDigits; ++ ADigit )
			{
				byte AValue;
				if( inherited::Enabled )
					AValue = GetSegmentsValue( ADigit );

				else
					AValue = 0;

				if( ReversedOrder )
				{
					for( int i = 0; i < 8; ++i )
						FOwner.SetPixelValue( FStartPixel + ( CountDigits - ADigit - 1 ) * 8 + i, ( AValue >> i ) & 1 );
				}

				else
				{
					for( int i = 0; i < 8; ++i )
						FOwner.SetPixelValue( FStartPixel + ADigit * 8 + i, ( AValue >> i ) & 1 );
				}
			}
		}

	protected:
		void DoReceive( void *_Data )
		{
			T_DATA AValue = *(T_DATA *)_Data;
			if( FValue == AValue )
				return;

			FValue = AValue;
			UpdateDisplay();
		}

		virtual void SystemStart() override
		{
			FValue = InitialValue;
			inherited::SystemStart();
			UpdateDisplay();
		}

	public:
		MaximLedGroupTypedValueDisplay7Segments( MaximLedGroupOwner &AOwner ) :
			inherited( AOwner ),
			InitialValue( 0 ) // Needed for specialization
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MaximLedGroupTypedValueDisplay7Segments::DoReceive );
		}
	};
//---------------------------------------------------------------------------
	template<> MaximLedGroupTypedValueDisplay7Segments<String>::MaximLedGroupTypedValueDisplay7Segments( MaximLedGroupOwner &AOwner ) :
			inherited( AOwner )
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MaximLedGroupTypedValueDisplay7Segments::DoReceive );
		}
//---------------------------------------------------------------------------
	class MaximLedGroupIntegerDisplay7Segments : public MaximLedGroupTypedValueDisplay7Segments<int32_t>
	{
		typedef Mitov::MaximLedGroupTypedValueDisplay7Segments<int32_t> inherited;

	public:
		bool	LeadingZeroes = false;

	protected:
		Mitov::SimpleList<unsigned int>	FPowers;
		long	FMaxValue; // The biggest possible to display value
		long	FMinValue; // The smallest possible to display value

	protected:
		virtual byte	GetSegmentsValue( int &ADigit )
		{
			int AValue;
			if( FValue > FMaxValue )
				AValue = 0b1000000;

			else if( FValue < FMinValue )
				AValue = 0b0001000;

			else
			{
				AValue = ( (int)abs( FValue ) ) / FPowers[ ADigit ];
				if( ( FValue < 0 ) && ( ADigit > 0 ))
				{
					if( ( !LeadingZeroes ) && ( AValue == 0 ) )
					{
						int APreviousValue = ( -FValue ) / FPowers[ ADigit - 1 ];
						if( APreviousValue > 0 )
							AValue = 0b0000001;

						else
							AValue = 0;

					}

					else
					{
						if( ADigit == CountDigits - 1 )
							AValue = 0b1000000;

						else
						{
							int ADigitValue = AValue % 10;
							AValue = C_MaximSegments[ ADigitValue ];
						}
					}
				}

				else
				{
					if( ( !LeadingZeroes ) && ( AValue == 0 ) )
						AValue = 0;

					else
					{
						int ADigitValue = AValue % 10;
						AValue = C_MaximSegments[ ADigitValue ];
					}
				}
			}

			return AValue;
		}

	public:
		virtual void SystemStart() 
		{
			FPowers.SetCount( CountDigits );
			for( int i = 0; i < CountDigits; ++i )
				FPowers[ i ] = pow( 10, i ) + 0.5;

			FMaxValue = pow( 10, CountDigits ) + 0.5 - 1;
			FMinValue = -( pow( 10, CountDigits - 1 ) + 0.5 - 1 );

			inherited::SystemStart();
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MaximLedGroupUnsignedBitPixelsSection : public MaximLedBasicGroup
	{
		typedef Mitov::MaximLedBasicGroup inherited;

	public:
		OpenWire::SinkPin	InputPin;

	public:
		uint32_t	InitialValue = 0;

	protected:
		uint32_t	FValue = 0;
		int			FStartPixel;

	protected:
		virtual void StartPixels( int &AStartPixel )
		{
			FValue = InitialValue;
			FStartPixel = AStartPixel;
			inherited::StartPixels( AStartPixel );
		}

		virtual void UpdateDisplay()
		{
			for( int i = 0; i < CountPixels; ++i )
				FOwner.SetPixelValue( FStartPixel + i, Enabled && ( FValue >> i ) & 1 );

		}

		void DoReceive( uint32_t *_Data )
		{
			uint32_t AValue = *(uint32_t *)_Data;
			if( FValue == AValue )
				return;

			FValue = AValue;
			UpdateDisplay();
		}

	public:
		MaximLedGroupUnsignedBitPixelsSection( MaximLedGroupOwner &AOwner ) :
			inherited( AOwner )
		{
			CountPixels = 32;

			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MaximLedGroupUnsignedBitPixelsSection::DoReceive );
		}
	};
//---------------------------------------------------------------------------
	class MaximLedGroupAnalogDisplay7Segments : public MaximLedGroupTypedValueDisplay7Segments<float>
	{
		typedef Mitov::MaximLedGroupTypedValueDisplay7Segments<float> inherited;

	public:
		int		Precision = 2;

	protected:
		char	*FBuffer;
		char	*FStartChar;
		byte	FTextLength;
		byte	FFirstPos;

		char	FDecimalPointPos;

	public:
		virtual byte	GetSegmentsValue( int &ADigit )
		{
			int AValue;
			int ANumDigits = CountDigits;
			if( ! FStartChar )
			{
				FStartChar = dtostrf( FValue, 1, Precision, FBuffer );
//				Serial.println( FStartChar );
				String AText( FStartChar );
				FTextLength = AText.length(); // strlen( FStartChar ); 
//				FTextLength = strlen( FStartChar ); 
//				Serial.println( FTextLength );
				FDecimalPointPos = AText.indexOf( '.' );
				if( Precision == 0 )
				{
					if( FTextLength <= ANumDigits )
						FFirstPos = FTextLength - 1;

					else
						FFirstPos = ANumDigits - 1;

				
//					Serial.println( FStartChar );
//					Serial.println( FTextLength );
//					Serial.println( FFirstPos );
//					Serial.println( FDecimalPointPos );
				}

				else
				{
					if( FTextLength <= ANumDigits + 1 )
						FFirstPos = FTextLength - 1;

					else
						FFirstPos = ANumDigits + 1 - 1;
				}				

//				Serial.println( FFirstPos );
			}

			int ACorrectedTextLength = FTextLength;
			if( Precision > 0 )
				--ACorrectedTextLength;

			if( ( ACorrectedTextLength - Precision ) > ANumDigits )
			{
				if( FValue > 0 )
					AValue = 0b1000000; // Overflow +

				else
					AValue = 0b0001000; // Overflow -
			}

			else
			{
				int ATextPos = FFirstPos - ADigit;
				if( ATextPos < 0 )
					AValue = 0;

				else
				{
					if( ATextPos < 0 )
						return( 0 );

					bool ADecimalPoint = ( FStartChar[ ATextPos ] == '.' );
					if( ATextPos <= FDecimalPointPos )
						--ATextPos;

//					if( ADecimalPoint )
//						--ATextPos;

					if( ATextPos < 0 )
						return( 0 );

/*
					if( FDigit == 0 )
					{
						Serial.println( FStartChar );
						Serial.println( ATextPos );
					}
*/
					if( FStartChar[ ATextPos ] == '-' )
						AValue = 0b0000001;

					else
					{
						AValue = FStartChar[ ATextPos ] - '0';
						AValue = C_MaximSegments[ AValue ];
					}

					if( ADecimalPoint )
						AValue |= 0x80;

				}
			}

			return AValue;
		}

		virtual void UpdateDisplay()
		{
			FStartChar = NULL;
			inherited::UpdateDisplay();
		}

	protected:
		virtual void SystemStart() 
		{
			FBuffer = new char[ 15 + Precision ];
			inherited::SystemStart();
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MaximLedBasicValueGroup : public MaximLedBasicInitialValueGroup
	{
		typedef MaximLedBasicInitialValueGroup inherited;

	public:
		OpenWire::SinkPin	InputPin;

	protected:
		bool	FValue;

	protected:
		int FStartPixel;

	public:
		virtual void StartPixels( int &AStartPixel )
		{
			FStartPixel = AStartPixel;
			inherited::StartPixels( AStartPixel );
		}

	protected:
		void DoReceive( void *_Data )
		{
			bool AValue = *(bool *)_Data;
			if( FValue == AValue )
				return;

			FValue = AValue;
			UpdateDisplay();
		}

		virtual void SystemStart() 
		{
			FValue = InitialValue;
			inherited::SystemStart();
			UpdateDisplay();
		}

	public:
		MaximLedBasicValueGroup( MaximLedGroupOwner &AOwner ) :
			inherited( AOwner )
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MaximLedBasicValueGroup::DoReceive );
		}
	};
//---------------------------------------------------------------------------
	class MaximLedSingleValueGroup : public MaximLedBasicValueGroup
	{
		typedef MaximLedBasicValueGroup inherited;

	protected:
		virtual void UpdateDisplay() override
		{
			for( int i = 0; i < CountPixels; ++i )
			{
				if( Enabled )
					FOwner.SetPixelValue( FStartPixel + i, FValue );

				else
					FOwner.SetPixelValue( FStartPixel + i, false );
			}
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MaximLedIndexedPixelGroup : public MaximLedBasicValueGroup
	{
		typedef MaximLedBasicValueGroup inherited;

	public:
		OpenWire::SinkPin	IndexInputPin;

	public:
		uint32_t InitialIndex = 0;

	protected:
		unsigned long FIndex;

	protected:
		void DoReceiveIndex( void *_Data )
		{
			unsigned long AIndex = *(unsigned long *)_Data;
			if( AIndex > CountPixels )
				AIndex = CountPixels;

			if( FIndex == AIndex )
				return;

			FIndex = AIndex;
			UpdateDisplay();
		}

		virtual void UpdateDisplay() override
		{
			if( Enabled )
				FOwner.SetPixelValue( FStartPixel + FIndex, FValue );

			else
				FOwner.SetPixelValue( FStartPixel + FIndex, false );
		}

	public:
		MaximLedIndexedPixelGroup( MaximLedGroupOwner &AOwner ) :
			inherited( AOwner )
		{
			IndexInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MaximLedIndexedPixelGroup::DoReceiveIndex );
		}
	};
//---------------------------------------------------------------------------
	class MaximLedIndexed2DPixelGroup : public MaximLedBasicValueGroup
	{
		typedef MaximLedBasicValueGroup inherited;

	public:
		OpenWire::SinkPin IndexInputPins[ 2 ];

	public:
		unsigned long Width = 8;
		unsigned long InitialIndexX = 0;
		unsigned long InitialIndexY = 0;

	protected:
		unsigned long FIndexX;
		unsigned long FIndexY;

	protected:
		void DoReceiveIndexX( void *_Data )
		{
			unsigned long AIndex = *(unsigned long *)_Data;
			if( AIndex > Width )
				AIndex = Width;

			if( FIndexX == AIndex )
				return;

			FIndexX = AIndex;
			UpdateDisplay();
		}

		void DoReceiveIndexY( void *_Data )
		{
			unsigned long AIndex = *(unsigned long *)_Data;
			unsigned long AHeight = ( CountPixels + Width - 1 ) / Width;
			if( AIndex > AHeight )
				AIndex = AHeight;

			if( FIndexY == AIndex )
				return;

			FIndexY = AIndex;
			UpdateDisplay();
		}

		virtual void UpdateDisplay()
		{
			unsigned long AIndex = FStartPixel + FIndexY * Width + FIndexX;
			if( Enabled )
				FOwner.SetPixelValue( AIndex, FValue );

			else
				FOwner.SetPixelValue( AIndex, false );
		}

	public:
		MaximLedIndexed2DPixelGroup( MaximLedGroupOwner &AOwner ) :
			inherited( AOwner )
		{
			IndexInputPins[ 0 ].SetCallback( this, (OpenWire::TOnPinReceive)&MaximLedIndexed2DPixelGroup::DoReceiveIndexX );
			IndexInputPins[ 1 ].SetCallback( this, (OpenWire::TOnPinReceive)&MaximLedIndexed2DPixelGroup::DoReceiveIndexY );
		}
	};
//---------------------------------------------------------------------------
	class MaximLedRunningPixelGroup;
//---------------------------------------------------------------------------
	class MaximLedPixelsReversedProperty
	{
	protected:
		MaximLedRunningPixelGroup &FOwner;

	public:
		bool Reversed : 1;
		bool AllPixels : 1;

	public:
		void SetReversed( bool AValue );

	public:
		MaximLedPixelsReversedProperty( MaximLedRunningPixelGroup &AOwner ) :
			FOwner( AOwner ),
			Reversed( false ),
			AllPixels( false )
		{
		}
	};
//---------------------------------------------------------------------------
	class MaximLedRunningPixelGroup : public MaximLedBasicValueGroup
	{
		typedef MaximLedBasicValueGroup inherited;

	public:
		OpenWire::ConnectSinkPin	StepInputPin;
		OpenWire::SourcePin	OutputPin;

	public:
		MaximLedPixelsReversedProperty	Reversed;

	public:
		void ReversePixels()
		{
			for( int i = 0; i < CountPixels / 2; ++i )
			{
				bool AOldValue1 = FOwner.GetPixelValue( FStartPixel + ( CountPixels - i - 1 ));
				bool AOldValue2 = FOwner.GetPixelValue( FStartPixel + i );

				FOwner.SetPixelValue( FStartPixel + i, AOldValue1 );
				FOwner.SetPixelValue( FStartPixel + ( CountPixels - i - 1 ), AOldValue2 );
			}
		}

	protected:
		void AnimatePixels()
		{
			if( Reversed.Reversed )
			{
				bool AOldValue = FOwner.GetPixelValue( FStartPixel );
				OutputPin.Notify( &AOldValue );
				for( int i = 0; i < CountPixels - 1; ++i )
				{
					AOldValue = FOwner.GetPixelValue( FStartPixel + i + 1 );
					FOwner.SetPixelValue( FStartPixel + i, AOldValue );
				}

				FOwner.SetPixelValue( FStartPixel + CountPixels - 1, FValue );
			}

			else
			{
				bool AOldValue = FOwner.GetPixelValue( FStartPixel + CountPixels - 1 );
				OutputPin.Notify( &AOldValue );
				for( int i = CountPixels - 1; i--; )
				{
					AOldValue = FOwner.GetPixelValue( FStartPixel + i );
					FOwner.SetPixelValue( FStartPixel + i + 1, AOldValue );
				}

				FOwner.SetPixelValue( FStartPixel, FValue );
			}
//			FOwner->FModified = true;
		}

		void DoReceiveStep( void *_Data )
		{
			AnimatePixels();
		}

	public:
		virtual void PixelsClock( unsigned long currentMicros )
		{
			if( StepInputPin.IsConnected())
				return;

			AnimatePixels();
		}

	public:
		MaximLedRunningPixelGroup( MaximLedGroupOwner &AOwner ) :
			inherited( AOwner ),
			Reversed( *this )
		{
			StepInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MaximLedRunningPixelGroup::DoReceiveStep );
		}
	};
//---------------------------------------------------------------------------
	inline void MaximLedPixelsReversedProperty::SetReversed( bool AValue )
	{
		if( Reversed == AValue )
			return;

		Reversed = AValue;
		if( AllPixels )
			FOwner.ReversePixels();

	}
//---------------------------------------------------------------------------
	class MaximLedRepeatGroup : public MaximLedBasicInitialValueGroup, public MaximLedGroupOwner
	{
		typedef MaximLedBasicInitialValueGroup inherited;

	protected:
		int FSubPixelCount;
		int FRepeatCount;
		int	FStartPixel;

	public:
		virtual void SetPixelValue( int AIndex, bool AValue )
		{
			for( int i = 0; i < FRepeatCount; ++i )
				FOwner.SetPixelValue( FStartPixel + AIndex + i * FSubPixelCount, AValue );
		}

		virtual bool GetPixelValue( int AIndex )
		{
			return FOwner.GetPixelValue( FStartPixel + AIndex );
		}

	public:
		virtual void StartPixels( int &AStartPixel ) 
		{
			FStartPixel = AStartPixel;
			inherited::StartPixels( AStartPixel );

			FSubPixelCount = 0;
			for( int i = 0; i < FPixelGroups.size(); ++i )
				FPixelGroups[ i ]->StartPixels( FSubPixelCount );

			if( FSubPixelCount == 0 )
				FRepeatCount = 0;

			else
				FRepeatCount = ( CountPixels + FSubPixelCount - 1 ) / FSubPixelCount;

		}

		virtual void PixelsClock( unsigned long currentMicros )
		{
			inherited::PixelsClock( currentMicros );

			for( int i = 0; i < FPixelGroups.size(); ++i )
				FPixelGroups[ i ]->PixelsClock( currentMicros );
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
}

#endif

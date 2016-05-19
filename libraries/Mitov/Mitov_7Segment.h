////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_7SEGMENT_h
#define _MITOV_7SEGMENT_h

#include <Mitov.h>
#include <Mitov_BasicDisplay.h>

namespace Mitov
{
	const byte C_Segments[16] =
	{
			0b0111111,  // = 0
			0b0000110,  // = 1
			0b1011011,  // = 2
			0b1001111,  // = 3
			0b1100110,  // = 4
			0b1101101,  // = 5
			0b1111101,  // = 6
			0b0000111,  // = 7
			0b1111111,  // = 8
			0b1101111,  // = 9
			0b1110111,  // = A
			0b1111100,  // = B
			0b1011000,  // = C
			0b1011110,  // = D
			0b1111001,  // = E
			0b1110001   // = F
	};
//---------------------------------------------------------------------------
	template<typename T> class DisplayBasic7Segment : public Mitov::BasicDisplay<T>
	{
		typedef	Mitov::BasicDisplay<T>	inherited;

	public:
		Mitov::SimpleList<OpenWire::SourcePin>	SegmentPins;

	public:
		bool InvertedSegments;

	public:
		DisplayBasic7Segment() :
			InvertedSegments( false )
		{
			SegmentPins.SetCount(16);
		}

	};
//---------------------------------------------------------------------------
	class Display7Segment : public Mitov::DisplayBasic7Segment<long>
	{
		typedef	Mitov::DisplayBasic7Segment<long>	inherited;

	protected:
		virtual void DoReceive( void *_Data )
		{
			inherited::DoReceive( _Data );
			UpdateDisplay();
		}

		virtual void UpdateDisplay()
		{
			int AValue = (Enabled) ? C_Segments[ FValue & 0xF ] : 0;
			for( int i = 0; i < 16; ++i )
			{
				bool ABitValue = AValue & 1;
				ABitValue ^= InvertedSegments;
				SegmentPins[ i ].Notify( &ABitValue );
				AValue >>= 1;
			}
		}

	};
//---------------------------------------------------------------------------
	template<typename T> class DisplayBasicDynamic7Segment : public Mitov::DisplayBasic7Segment<T>
	{
		typedef	Mitov::DisplayBasic7Segment<T>	inherited;

	public:
		Mitov::SimpleList<OpenWire::SourcePin>	DigitPins;

	public:
		unsigned long	RefreshInterval = 1000;
		bool			InvertedDigits = false;

	protected:
		unsigned long	FLastTime = 0;
		int				FDigit = 0;

	protected:
		virtual int	GetSegmentsValue() = 0;

		virtual void UpdateDisplay()
		{
			int AValue;
			if( inherited::Enabled )
				AValue = GetSegmentsValue();

			else
				AValue = 0;

			for( int i = 0; i < DigitPins.size(); ++i )
			{
				bool ABitValue = false;
				ABitValue ^= InvertedDigits;
				DigitPins[ i ].Notify( &ABitValue );
			}

			for( int i = 0; i < 16; ++i )
			{
				bool ABitValue = AValue & 1;
				ABitValue ^= inherited::InvertedSegments;
				inherited::SegmentPins[ i ].Notify( &ABitValue );
				AValue >>= 1;
			}

			AValue = FDigit;
			for( int i = 0; i < DigitPins.size(); ++i )
			{
				bool ABitValue = ( AValue-- == 0 );
				ABitValue ^= InvertedDigits;
				DigitPins[ i ].Notify( &ABitValue );
			}
		}

	public:
		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			if( ( currentMicros - FLastTime ) < RefreshInterval )
				return;

			++FDigit;

			if( FDigit >= DigitPins.size() )
				FDigit = 0;

			FLastTime = currentMicros;
			UpdateDisplay();
		}

	};
//---------------------------------------------------------------------------
	class DisplayIntegerDynamic7Segment : public Mitov::DisplayBasicDynamic7Segment<long>
	{
		typedef	Mitov::DisplayBasicDynamic7Segment<long>	inherited;

	public:
		bool	LeadingZeroes = false;

	protected:
		Mitov::SimpleList<unsigned int>	FPowers;
		long	FMaxValue; // The biggest possible to display value
		long	FMinValue; // The smallest possible to display value

	protected:
		virtual int	GetSegmentsValue()
		{
			int AValue;
			if( FValue > FMaxValue )
				AValue = 0b0000001;

			else if( FValue < FMinValue )
				AValue = 0b0001000;

			else
			{
				AValue = ( (int)abs( FValue ) ) / FPowers[ FDigit ];
				if( ( FValue < 0 ) && ( FDigit > 0 ))
				{
					if( ( !LeadingZeroes ) && ( AValue == 0 ) )
					{
						int APreviousValue = ( -FValue ) / FPowers[ FDigit - 1 ];
						if( APreviousValue > 0 )
							AValue = 0b1000000;

						else
							AValue = 0;

					}
					else
					{
						if( FDigit == DigitPins.size() - 1 )
							AValue = 0b1000000;

						else
						{
							int ADigitValue = AValue % 10;
							AValue = C_Segments[ ADigitValue ];
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
						AValue = C_Segments[ ADigitValue ];
					}
				}
			}

			return AValue;
		}

		virtual void SystemInit()
		{
			FPowers.SetCount( DigitPins.size() );
			for( int i = 0; i < DigitPins.size(); ++i )
				FPowers[ i ] = pow( 10, i ) + 0.5;

			FMaxValue = pow( 10, DigitPins.size() ) + 0.5 - 1;
			FMinValue = -( pow( 10, DigitPins.size() - 1 ) + 0.5 - 1 );

			inherited::SystemInit();
		}

	};
//---------------------------------------------------------------------------
	class DisplayAnalogDynamic7Segment : public Mitov::DisplayBasicDynamic7Segment<float>
	{
		typedef	Mitov::DisplayBasicDynamic7Segment<float>	inherited;

	public:
		unsigned char	Precision = 2;

	protected:
		char	*FBuffer;
		char	*FStartChar;
		unsigned char	FTextLength;
		unsigned char	FFirstPos;

		char			FDecimalPointPos;

	protected:
		virtual int	GetSegmentsValue()
		{
			int AValue;
			int ANumDigits = DigitPins.size();
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
					AValue = 0b0000001; // Overflow +

				else
					AValue = 0b0001000; // Overflow -
			}

			else
			{
				int ATextPos = FFirstPos - FDigit;
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
						AValue = 0b1000000;

					else
					{
						AValue = FStartChar[ ATextPos ] - '0';
						AValue = C_Segments[ AValue ];
					}

					if( ADecimalPoint )
						AValue |= 0x80;

				}
			}

			return AValue;
		}

		virtual void DoReceive( void *_Data )
		{
			FStartChar = NULL;
			inherited::DoReceive( _Data );
		}

	public:
		virtual void SystemInit()
		{
			FBuffer = new char[ 15 + Precision ];
			inherited::SystemInit();
		}

/*
		virtual ~DisplayAnalogDynamic7Segment()
		{
			delete [] FBuffer;
		}
*/
	};

}

#endif

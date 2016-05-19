////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_CONVERTERS_h
#define _MITOV_CONVERTERS_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	template<typename T> class TypedToAnalog : public CommonFilter
	{
		typedef CommonFilter inherited;

	public:
		float	Scale = 1.0;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			float AFloatValue = ( *(T*)_Data ) * Scale;
			OutputPin.Notify( &AFloatValue );
		}

	};
//---------------------------------------------------------------------------
	class AnalogToInteger : public CommonFilter
	{
		typedef CommonFilter inherited;

	public:
		bool	Round = true;
		float	Scale = 1.0;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			float AFloatValue = *(float*)_Data * Scale;
			if( Round )
				AFloatValue += 0.5f;

			long AIntValue = AFloatValue;
			OutputPin.Notify( &AIntValue );
		}

	};
//---------------------------------------------------------------------------
	class AnalogToUnsigned : public CommonFilter
	{
		typedef CommonFilter inherited;

	public:
		bool	Round = true;
		bool	Constrain = true;
		float	Scale = 1.0;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			float AFloatValue = *(float*)_Data * Scale;
			if( Round )
				AFloatValue += 0.5f;

			long AIntValue = AFloatValue;
			if( Constrain )
				AIntValue &= 0x7FFFFFFF;

			OutputPin.Notify( &AIntValue );
		}

	};
//---------------------------------------------------------------------------
	class AnalogToText : public CommonFilter
	{
		typedef CommonFilter inherited;

	public:
		int	MinWidth = 1;
		int	Precision = 3;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			float AFloatValue = *(float*)_Data;
			char AText[ 50 ];
			dtostrf( AFloatValue,  MinWidth, Precision, AText );
			OutputPin.Notify( AText );
		}

	};
//---------------------------------------------------------------------------
	template<typename T> class TypedToText : public CommonFilter
	{
		typedef CommonFilter inherited;

	public:
		int	Base = 10;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			T AValue = *(T*)_Data;
			char AText[ 50 ];
			itoa( AValue, AText, Base );
			OutputPin.Notify( AText );
		}

	};
//---------------------------------------------------------------------------
	class DigitalToText : public CommonFilter
	{
		typedef CommonFilter inherited;

	public:
		String	TrueValue = "true";
		String	FalseValue = "false";

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			bool AValue = *(bool *)_Data;
			const char *AText;
			if( AValue )
				AText = TrueValue.c_str();

			else
				AText = FalseValue.c_str();

			OutputPin.Notify( (void*)AText );
		}

	};
//---------------------------------------------------------------------------
	template <typename T, T C_TRUE, T T_FALSE> class DigitalToType : public CommonFilter
	{
		typedef CommonFilter inherited;

	public:
		T	TrueValue = C_TRUE;
		T	FalseValue = T_FALSE;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			if( *(bool *)_Data )
				OutputPin.Notify( &TrueValue );

			else
				OutputPin.Notify( &FalseValue );

		}

	};
//---------------------------------------------------------------------------
	template <typename T> class DigitalToTypeParams : public CommonFilter
	{
		typedef CommonFilter inherited;

	public:
		T	TrueValue;
		T	FalseValue;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			if( *(bool *)_Data )
				OutputPin.Notify( &TrueValue );

			else
				OutputPin.Notify( &FalseValue );

		}

	public:
		DigitalToTypeParams( T ATrueValue, T AFalseValue ) :
			TrueValue( ATrueValue ),
			FalseValue( AFalseValue )
		{
		}
	};
//---------------------------------------------------------------------------
	class TextToAnalog : public CommonFilter
	{
		typedef CommonFilter inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			char * AText = (char*)_Data;
			float AValue = strtod( AText, NULL );
			OutputPin.Notify( &AValue );
		}

	};
//---------------------------------------------------------------------------
	class TextToInteger : public CommonFilter
	{
		typedef CommonFilter inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			char * AText = (char*)_Data;
			long AValue = atoi( AText );
			OutputPin.Notify( &AValue );
		}

	};
//---------------------------------------------------------------------------
	class TextToUnsigned : public CommonFilter
	{
		typedef CommonFilter inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			char * AText = (char*)_Data;
			unsigned long AValue = atoi( AText );
			OutputPin.Notify( &AValue );
		}

	};
//---------------------------------------------------------------------------
	class UnsignedToInteger : public CommonFilter
	{
		typedef CommonFilter inherited;

	public:
		bool Constrain = true;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			unsigned long AValue = *(unsigned long*)_Data;
			if( Constrain )
				AValue &= 0x7FFFFFFF;

			OutputPin.Notify( &AValue );
		}

	};
//---------------------------------------------------------------------------
	class IntegerToUnsigned : public CommonFilter
	{
		typedef CommonFilter inherited;

	public:
		bool Constrain = true;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			long AValue = *(long*)_Data;
			if( Constrain )
				AValue &= 0x7FFFFFFF;

			OutputPin.Notify( &AValue );
		}

	};
//---------------------------------------------------------------------------
	template<int C_NUM_INPUTS> class UnsignedToDigital : public CommonSink
	{
		typedef CommonSink inherited;

	public:
		OpenWire::SourcePin OutputPins[ C_NUM_INPUTS ];

	public:
		uint32_t	InitialValue = 0;

	protected:
		uint32_t	FOldValue = 0;

	protected:
		void SetValue( uint32_t AValue, bool AUpdate )
		{
			for( long i = 0; i < C_NUM_INPUTS; ++i )
			{
				unsigned long ABit = ((unsigned long)1) << i;
				bool AOldBitValue = ( FOldValue & ABit );
				bool ANewBitValue = ( AValue & ABit );
				if( AUpdate || AOldBitValue != ANewBitValue )
					OutputPins[ i ].Notify( (void *)&GBooleanConst[ ANewBitValue ] );

			}

			FOldValue = AValue;
		}

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			unsigned long AValue = *(unsigned long *)_Data;
			if( FOldValue == AValue )
				return;

			SetValue( AValue, false );
		}

		virtual void SystemInit()
		{
			inherited::SystemInit();
			SetValue( InitialValue, true );
		}

	};
//---------------------------------------------------------------------------
	class ReversableConverter : public CommonTypedFilter<float>
	{
		typedef CommonTypedFilter<float> inherited;

	public:
		bool	Reverse = false;

	};
//---------------------------------------------------------------------------
	class CelsiusToFahrenheit : public ReversableConverter
	{
		typedef ReversableConverter inherited;

	protected:
        virtual float FilterValue( float AValue ) override
        {
			if( Reverse )
				return ( AValue - 32.0 ) / ( 9.0/5.0 );

			else
				return AValue * ( 9.0/5.0 ) + 32.0;
        }

	};
//---------------------------------------------------------------------------
	class CelsiusToKelvin : public ReversableConverter
	{
		typedef ReversableConverter inherited;

	protected:
        virtual float FilterValue( float AValue ) override
        {
			if( Reverse )
				return AValue - 273.15;

			else
				return AValue + 273.15;
        }

	};
//---------------------------------------------------------------------------
	template<int T_SIZE> class CharToText : public CommonImplementedEnableFilter
	{
		typedef CommonImplementedEnableFilter inherited;

	public:
		uint32_t	MaxLength = 100;
		bool	Truncate = false;
		bool	UpdateOnEachChar = false;

	protected:
		byte	FBuffer[ T_SIZE + 1 ];
		uint32_t	FIndex = 0;

	protected:
		void SendBufferNoReset()
		{
			FBuffer[ FIndex ] = '\0';
			inherited::OutputPin.Notify( FBuffer );
		}

		void SendBuffer()
		{
			SendBufferNoReset();
			FIndex = 0;
		}

		virtual void ReceiveValue( void *_Data ) override
		{
			char AValue = *(char *)_Data;
			if( AValue == '\n' )
				return;

			if( AValue == '\r' )
			{
				SendBuffer();
				return;
			}
				
			if( FIndex >= T_SIZE )
			{
				if( Truncate )
					return;

				SendBuffer();
			}

			FBuffer[ FIndex++ ] = AValue;
			if( UpdateOnEachChar )
				SendBufferNoReset();

		}

	};
//---------------------------------------------------------------------------
	class TextToChar : public CommonImplementedEnableFilter
	{
		typedef CommonImplementedEnableFilter inherited;

	public:
		bool	AddReturn = true;
		bool	AddNewLine = true;

	protected:
		virtual void ReceiveValue( void *_Data ) override
		{
			char *AValue = (char *)_Data;
			while( *AValue )
			{
				inherited::OutputPin.Notify( AValue );
				++AValue;
			}

			if( AddReturn )
				inherited::OutputPin.SendValue( '\r' );

			if( AddNewLine )
				inherited::OutputPin.SendValue( '\n' );
		}
	};
//---------------------------------------------------------------------------
	class PressureToAltitude : public CommonTypedFilter<float>
	{
	public:
		float BaseLinePressure = 0;

	protected:
        virtual float FilterValue( float AValue ) override
        {
			return ( 44330.0 * ( 1 - pow( AValue / BaseLinePressure, 1 / 5.255 )));
        }

	};
//---------------------------------------------------------------------------
	class AltitudePressureToSeaLevelPressure : public CommonTypedFilter<float>
	{
	public:
		float Altitude = 0;

	protected:
        virtual float FilterValue( float AValue ) override
        {
			return ( AValue / pow( 1 - ( Altitude / 44330.0 ), 5.255 ));
        }

	};
//---------------------------------------------------------------------------
}

#endif

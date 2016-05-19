////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_MATH_h
#define _MITOV_MATH_h

#include <Mitov.h>

namespace Mitov
{
#define Min Min
#define Max Max
//---------------------------------------------------------------------------
	#define MITOV_PI 3.14159265359
//---------------------------------------------------------------------------
    template<typename T> class CommonValueMathFilter : public CommonTypedFilter<T>
    {
		typedef CommonTypedFilter<T> inherited;

    public:
        T   Value;

    public:
        CommonValueMathFilter( T AValue = 0 ) : Value( AValue ) {}

    };
//---------------------------------------------------------------------------
    template<typename T> class AddValue : public CommonValueMathFilter<T>
	{
		typedef CommonValueMathFilter<T> inherited;

	protected:
        virtual T FilterValue( T AValue )
        {
            return AValue + inherited::Value;
        }

	};
//---------------------------------------------------------------------------
	template<typename T> class SubtractValue : public CommonValueMathFilter<T>
	{
		typedef CommonValueMathFilter<T> inherited;

	protected:
        virtual T FilterValue( T AValue )
        {
            return AValue - inherited::Value;
        }

	};
//---------------------------------------------------------------------------
	template<typename T> class SubtractFromValue : public CommonValueMathFilter<T>
	{
		typedef CommonValueMathFilter<T> inherited;

	protected:
        virtual T FilterValue( T AValue )
        {
            return inherited::Value - AValue;
        }

	};
//---------------------------------------------------------------------------
    template<typename T> class MultiplyByValue : public CommonValueMathFilter<T>
	{
		typedef CommonValueMathFilter<T> inherited;

	protected:
        virtual T FilterValue( T AValue )
        {
            return AValue * inherited::Value;
        }

	public:
		MultiplyByValue() : 
			inherited( 1 )
		{
		}
	};
//---------------------------------------------------------------------------
    template<typename T> class DivideByValue : public CommonValueMathFilter<T>
	{
		typedef CommonValueMathFilter<T> inherited;

	protected:
        virtual T FilterValue( T AValue )
        {
            return AValue / inherited::Value;
        }

	public:
		DivideByValue() : 
			inherited( 1 )
		{
		}
	};
//---------------------------------------------------------------------------
	template<typename T> class DivideValue : public CommonValueMathFilter<T>
	{
		typedef CommonValueMathFilter<T> inherited;

	protected:
        virtual T FilterValue( T AValue )
        {
            return inherited::Value / AValue;
        }

	};
//---------------------------------------------------------------------------
	template<typename T> class Inverse : public CommonTypedFilter<T>
	{
		typedef CommonTypedFilter<T> inherited;

	protected:
        virtual T FilterValue( T AValue )
        {
            return -AValue;
        }

	};
//---------------------------------------------------------------------------
	template<typename T> class Abs : public CommonTypedFilter<T>
	{
		typedef CommonTypedFilter<T> inherited;

	protected:
        virtual T FilterValue( T AValue )
        {
            return abs( AValue );
        }

	};
//---------------------------------------------------------------------------
	class MinLimit : public CommonValueMathFilter<float>
	{
	protected:
        virtual float FilterValue( float AValue )
        {
			return ( AValue < Value ) ? Value : AValue;
        }
	};
//---------------------------------------------------------------------------
	class MaxLimit : public CommonValueMathFilter<float>
	{
		typedef CommonValueMathFilter<float> inherited;

	protected:
        virtual float FilterValue( float AValue )
        {
			return ( AValue > Value ) ? Value : AValue;
        }

	public:
		MaxLimit() :
			inherited( 1.0 )
		{
		}
	};
//---------------------------------------------------------------------------
	class Limit : public CommonTypedFilter<float>
	{
	public:
		float	Min = 0.0;
		float	Max = 1.0;

	protected:
        virtual float FilterValue( float AValue )
        {
			return ( AValue > Max ) ? 
					Max : 
					( AValue < Min ) ? Min : AValue;
        }

	};
//---------------------------------------------------------------------------
	enum CompareValueType { ctEqual, ctNotEqual, ctBigger, ctSmaller, ctBiggerOrEqual, ctSmallerOrEqual };
//---------------------------------------------------------------------------
	template<typename T> class CompareValue : public CommonEnableFilter
	{
		typedef CommonEnableFilter inherited;

    public:
        T	Value;

	protected:
		T	FCurrentValue;

    public:
		CompareValueType	CompareType : 3;
		bool				OnlyChanged : 1;

	protected:
		bool	FOutputValue : 1;
		bool	FStarted : 1;
		bool	FProcessedOnce : 1;

    public:
		void SetValue( T AValue )
		{
			if( OnlyChanged )
				if( Value == AValue )
					return;

			Value = AValue;
			ProcessOutput();
		}

	protected:
		void ProcessOutput()
		{
			if( ! inherited::Enabled )
				return;

//			Serial.print( FCurrentValue ); Serial.print( " ? " ); Serial.println( Value );

			bool ABoolValue;
			switch( CompareType )
			{
			case ctEqual:
				ABoolValue = ( FCurrentValue == Value );
				break;

			case ctNotEqual:
				ABoolValue = ( FCurrentValue != Value );
				break;

			case ctBigger:
				ABoolValue = ( FCurrentValue > Value );
				break;

			case ctSmaller:
				ABoolValue = ( FCurrentValue < Value );
				break;

			case ctBiggerOrEqual:
				ABoolValue = ( FCurrentValue >= Value );
				break;

			case ctSmallerOrEqual:
				ABoolValue = ( FCurrentValue <= Value );
				break;

			}

			if( ! OnlyChanged )
				if( FOutputValue )
				{
					FOutputValue = false;
					OutputPin.SendValue( false );
				}

			if( ( !FStarted ) || ( FOutputValue != ABoolValue ) )
			{
				OutputPin.Notify( &ABoolValue );
				FOutputValue = ABoolValue;
				FStarted = true;
			}
		}

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			T AValue = *(T*)_Data;

			if( OnlyChanged )
				if( FProcessedOnce )
					if( AValue == FCurrentValue )
						return;

			FCurrentValue = AValue;
			FProcessedOnce = true;

			ProcessOutput();
		}

	public:
		CompareValue( T AInitialValue ) : 
			Value( AInitialValue ),
			FCurrentValue( AInitialValue ),
			CompareType( ctEqual ),
			OnlyChanged( true ),
			FProcessedOnce( false ),
			FStarted( false ),
			FOutputValue( false )
		{
		}

	};
//---------------------------------------------------------------------------
	class CompareStringValue : public CompareValue<String>
	{
		typedef CompareValue<String> inherited;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			String AValue = String( (char*)_Data );

			if( OnlyChanged )
				if( FProcessedOnce )
					if( AValue == FCurrentValue )
						return;

			FCurrentValue = AValue;
			FProcessedOnce = true;

			ProcessOutput();
		}

	public:
		CompareStringValue() : 
			inherited( String() )
		{
		}
	};
//---------------------------------------------------------------------------
	class AveragePeriod : public CommonFilter
	{
	public:
		unsigned long	Period = 1000;

	protected:
		float FSum = 0.0f;
		unsigned long	FCount = 0;
		unsigned long	FLastTime = 0;

	protected:
		virtual void DoReceive( void *_Data ) override
		{
			FSum += *(float *)_Data;
			++FCount;
		}

		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			if( currentMicros - FLastTime < Period )
				return;

			if( ! FCount )
				return;

			float AValue = FSum / FCount;

			FSum = 0.0;
			FCount = 0;
			FLastTime = currentMicros;

			OutputPin.Notify( &AValue );
		}

	};
//---------------------------------------------------------------------------
	template<typename T, int C_NUM_INPUTS> class CommonMathMultiInput : public CommonClockedMultiInput<T, C_NUM_INPUTS>
	{
		typedef CommonClockedMultiInput<T, C_NUM_INPUTS> inherited;

    public:
        bool Enabled = true;

	};
//---------------------------------------------------------------------------
	template<typename T, int C_NUM_INPUTS> class Add : public CommonMathMultiInput<T, C_NUM_INPUTS>
	{
		typedef CommonMathMultiInput<T, C_NUM_INPUTS> inherited;

	protected:
		virtual T CalculateOutput() override
		{
			T AValue = 0;
			for( int i = 0; i < C_NUM_INPUTS; ++i )
				AValue += inherited::InputPins[ i ].Value;

			return AValue;
		}
	};
//---------------------------------------------------------------------------
	template<typename T, int C_NUM_INPUTS> class Multiply : public CommonMathMultiInput<T, C_NUM_INPUTS>
	{
		typedef CommonMathMultiInput<T, C_NUM_INPUTS> inherited;

	protected:
		virtual T CalculateOutput() override
		{
			T AValue = 1;
			for( int i = 0; i < C_NUM_INPUTS; ++i )
				AValue *= inherited::InputPins[ i ].Value;

			return AValue;
		}
	};
//---------------------------------------------------------------------------
	class ValueRange
	{
	public:
		float Min = 0;
		float Max = 1;

	};
//---------------------------------------------------------------------------
	class MapRange : public CommonTypedFilter<float>
	{
		typedef CommonTypedFilter<float> inherited;

	public:
		ValueRange	InputRange;
		ValueRange	OutputRange;

	protected:
        virtual float FilterValue( float AValue ) override
        {
			return (( AValue - InputRange.Min ) * (OutputRange.Max - OutputRange.Min) / (InputRange.Max - InputRange.Min)) + OutputRange.Min;
        }
	};
//---------------------------------------------------------------------------
    template<typename T> class RaiseToPower : public CommonValueMathFilter<T>
	{
		typedef CommonValueMathFilter<T> inherited;

	protected:
        virtual T FilterValue( T AValue ) override
        {
            return pow( AValue, inherited::Value );
        }

	};
//---------------------------------------------------------------------------
	class Sine : public CommonTypedFilter<float>
	{
		typedef CommonTypedFilter<float> inherited;

	protected:
        virtual float FilterValue( float AValue ) override
        {
            return sin( AValue );
        }

	};
//---------------------------------------------------------------------------
	class Cosine : public CommonTypedFilter<float>
	{
		typedef CommonTypedFilter<float> inherited;

	protected:
        virtual float FilterValue( float AValue ) override
        {
            return cos( AValue );
        }

	};
//---------------------------------------------------------------------------
	class RadToDegrees : public CommonTypedFilter<float>
	{
		typedef CommonTypedFilter<float> inherited;

	protected:
        virtual float FilterValue( float AValue ) override
        {
            return AValue * 180 / MITOV_PI;
        }

	};
//---------------------------------------------------------------------------
	class DegreesToRad : public CommonTypedFilter<float>
	{
		typedef CommonTypedFilter<float> inherited;

	protected:
        virtual float FilterValue( float AValue ) override
        {
            return AValue * MITOV_PI / 180;
        }

	};
//---------------------------------------------------------------------------
    class AndUnsignedValue : public CommonValueMathFilter<uint32_t>
	{
		typedef CommonValueMathFilter<uint32_t> inherited;

	protected:
        virtual uint32_t FilterValue( uint32_t AValue ) override
        {
            return AValue & Value;
        }

	};
//---------------------------------------------------------------------------
    class OrUnsignedValue : public CommonValueMathFilter<uint32_t>
	{
		typedef CommonValueMathFilter<uint32_t> inherited;

	protected:
        virtual uint32_t FilterValue( uint32_t AValue ) override
        {
            return AValue | Value;
        }

	};
//---------------------------------------------------------------------------
    class XorUnsignedValue : public CommonValueMathFilter<uint32_t>
	{
		typedef CommonValueMathFilter<uint32_t> inherited;

	protected:
        virtual uint32_t FilterValue( uint32_t AValue ) override
        {
            return AValue ^ Value;
        }

	};
//---------------------------------------------------------------------------
    class NotUnsignedValue : public CommonTypedFilter<uint32_t>
	{
		typedef CommonTypedFilter<uint32_t> inherited;

	protected:
        virtual uint32_t FilterValue( uint32_t AValue ) override
        {
            return ~AValue;
        }

	};
//---------------------------------------------------------------------------
#undef Min
#undef Max
}

#endif

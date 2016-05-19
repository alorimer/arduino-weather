////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_STRUCTURE_h
#define _MITOV_STRUCTURE_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class BasicMakeStructureElement : public OpenWire::Component
	{
	public:
		virtual	bool GetIsPopulated() { return true; }
		virtual	bool GetIsModified() { return false; }
		virtual	void ClearModified() {}
		virtual	void GetData( unsigned char *&AData, unsigned char &AOffset ) {}
	};
//---------------------------------------------------------------------------
	class BasicSplitStructureElement : public OpenWire::Component
	{
	public:
		virtual void Extract( unsigned char *&AData, unsigned char &AOffset ) {}

	};
//---------------------------------------------------------------------------
	template<typename T> class BasicTypedStructureSinkElement : public BasicMakeStructureElement
	{
	public:
		OpenWire::SinkPin	InputPin;

	protected:
		T		FValue = 0;
		bool	FPopulated : 1;
		bool	FModified : 1;

/*
	public:
		T	InitialValue;

		virtual void SystemInit()
		{
			inherited::SystemInit();
			FValue = InitialValue;
		}
*/
	public:
		virtual	bool GetIsPopulated() override
		{ 
			return FPopulated;
		}

		virtual	bool GetIsModified() override
		{ 
			return FModified;
		}

		virtual	void ClearModified() override
		{
			FModified = false;
		}

		virtual	void GetData( unsigned char *&AData, unsigned char &AOffset ) override
		{ 
			if( AOffset )
			{
//				Serial.println( "TTT" );
				++AData;
				AOffset = 0;
			}

#ifdef VISUINO_ESP8266
			memcpy( AData, &FValue, sizeof( T ));
#else
			*(T*)AData = FValue;
#endif
//			memcpy( AData, &FValue, sizeof( T ));
			AData += sizeof( T );
			FModified = false;
		}

	protected:
		virtual void DoReceive( void *_Data )
		{
			T AValue = *(T*)_Data;
			FPopulated = true;
			if( AValue == FValue )
				return;

			FValue = AValue;
			FModified = true;
		}

	public:
		BasicTypedStructureSinkElement() :
			FPopulated( false ),
			FModified( false )
		{
			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&BasicTypedStructureSinkElement::DoReceive );
		}

	};
//---------------------------------------------------------------------------
	template<typename T> class BasicTypedStructureSourceElement : public BasicSplitStructureElement
	{
	public:
		OpenWire::SourcePin	OutputPin;

	public:
		virtual void Extract( unsigned char *&AData, unsigned char &AOffset ) override
		{ 
			if( AOffset )
			{
				++ AData;
				AOffset = 0;
			}

			T AValue;
			AValue = *((T *)AData );
			OutputPin.Notify( &AValue );

			AData += sizeof( T );
		}
	};
//---------------------------------------------------------------------------
	class DigitalStructureSourceElement : public BasicTypedStructureSourceElement<bool>
	{
	public:
/*
		virtual int  Start( bool &AAllign ) override
		{ 
			AAllign = false;
			return 1; 
		}
*/
		virtual void Extract( unsigned char *&AData, unsigned char &AOffset ) override
		{
			unsigned char AValue = *AData;
			bool ABoolValue = (( AValue & ( 1 << AOffset )) != 0 );
			++AOffset;

			if( AOffset == 8 )
			{
				AOffset = 0;
				++AData;
			}

			OutputPin.Notify( &ABoolValue );
		}
	};
//---------------------------------------------------------------------------
	class DigitalStructureSinkElement : public BasicTypedStructureSinkElement<bool>
	{
	public:
		virtual	void GetData( unsigned char *&AData, unsigned char &AOffset ) override
		{
			*AData &= 0xFF >> ( 8 - AOffset ); // Zero the upper bits
			if( FValue )
				*AData |= 1 << AOffset;	// Set the bit

			++AOffset;
			if( AOffset == 8 )
			{
				AOffset = 0;
				++AData;
			}
		}
	};
//---------------------------------------------------------------------------
	template<int T_BUFFER_SIZE> class MakeStructure : public Mitov::CommonSource, public Mitov::ClockingSupport
	{
		typedef Mitov::CommonSource inherited;

	public:
		Mitov::SimpleObjectList<BasicMakeStructureElement*>	Elements;

		bool	OnlyModified = false;

	protected:
		byte FBuffer[ T_BUFFER_SIZE ];

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
//			Serial.println( "++++++" );
			int AElementCount = Elements.size();
			if( OnlyModified )
			{
				bool AModified = false;
				for( int i = 0; i < AElementCount; ++i )
					if( Elements[ i ]->GetIsModified() )
					{
						AModified = true;
						break;
					}

				if( !AModified )
					return;
			}

			else
			{
				for( int i = 0; i < AElementCount; ++i )
					if( ! Elements[ i ]->GetIsPopulated() )
						return;

			}

			byte *ADataPtr = FBuffer;
			unsigned char AOffset = 0;
			for( int i = 0; i < AElementCount; ++i )
				Elements[ i ]->GetData( ADataPtr, AOffset );

//			Serial.println( ASize );

			
			OutputPin.SendValue( Mitov::TDataBlock( T_BUFFER_SIZE, FBuffer ));

/*
			ADataPtr = FBuffer;
			int ASize = T_BUFFER_SIZE;

			while( ASize-- )
				OutputPin.Notify( ADataPtr++ );
*/
		}

	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			if( !ClockInputPin.IsConnected() )
				DoClockReceive( NULL );

//			delay( 1000 );
			inherited::SystemLoopBegin( currentMicros );
		}
/*
		virtual void SystemStart()
		{
			FBufferSize = 0;
			FAllElements.push_back( &HeadMarker );
			for( int i = 0; i < Elements.size(); ++i )
				FAllElements.push_back( Elements[ i ] );

			FAllElements.push_back( &Checksum );

			for( int i = 0; i < FAllElements.size(); ++i )
			{
				bool AAlligned = false;
				FBufferSize += FAllElements[ i ]->GetSize( AAlligned );
//				Serial.println( FBufferSize );
				if( AAlligned )
				{
					FBufferSize = ( FBufferSize + 7 ) / 8;
					FBufferSize *= 8;
				}

			}

//			Serial.println( FBufferSize );

			FBufferSize = ( FBufferSize + 7 ) / 8;
			FBuffers[ 0 ] = new unsigned char[ FBufferSize * 2 ];
			FBuffers[ 1 ] = new unsigned char[ FBufferSize * 2 ];

			inherited::SystemStart();
		}
*/
	};
//---------------------------------------------------------------------------
	template<int T_BUFFER_SIZE> class SplitStructure : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink inherited;

	public:
		Mitov::SimpleObjectList<BasicSplitStructureElement*>	Elements;

	protected:
		byte FBuffer[ T_BUFFER_SIZE ];
		byte *FDataPtr = FBuffer;

	protected:
		virtual void DoReceive( void *_Data )
		{
			Mitov::TDataBlock ABlock = *(Mitov::TDataBlock *)_Data;
			while( ABlock.Size -- )
			{
			    *FDataPtr ++ = *ABlock.Data ++;

				if( ( FDataPtr - FBuffer ) >= T_BUFFER_SIZE )
				{
					unsigned char AOffset = 0;
					byte *ADataPtr = FBuffer;
					for( int i = 0; i < Elements.size(); ++i )
						Elements[ i ]->Extract( ADataPtr, AOffset );

					FDataPtr = FBuffer;
				}
			}
		}

/*
		virtual void SystemStart() override
		{
			FDataPtr = FBuffer;
			inherited::SystemStart();
		}
*/
	};
//---------------------------------------------------------------------------

}

#endif
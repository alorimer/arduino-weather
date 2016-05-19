////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_MICRO_SD_CARD_h
#define _MITOV_MICRO_SD_CARD_h

#include <Mitov.h>

#include <SPI.h>
#include <SD.h>

namespace Mitov
{
	class MicroSDCard_Intf
	{
	public:
		virtual bool DoExists( String APathName ) = 0;
		virtual bool DoCreate( String APathName ) = 0;
		virtual bool DoRemove( String APathName ) = 0;
		virtual bool DoOpen( String APathName, uint8_t mode, File &AFile ) = 0;

	};
//---------------------------------------------------------------------------
	class MicroSDCardElementBasic : public OpenWire::Object
	{
		typedef OpenWire::Object inherited;

	public:
		String	PathName;

	protected:
		Mitov::MicroSDCard_Intf	&FOwner;

	public:
		MicroSDCardElementBasic( Mitov::MicroSDCard_Intf &AOwner ) :
			FOwner( AOwner )
		{
		}

	};
//---------------------------------------------------------------------------
	class MicroSDCardElementOperation : public OpenWire::Object
	{
	public:
		OpenWire::SinkPin	ClockInputPin;
		OpenWire::SourcePin	OutputPin;

	};
//---------------------------------------------------------------------------
	class MicroSDCardElementDirectory : public MicroSDCardElementBasic, public MicroSDCard_Intf
	{
		typedef Mitov::MicroSDCardElementBasic inherited;

	public:
		MicroSDCardElementOperation	CheckExists;
		MicroSDCardElementOperation	Create;
		MicroSDCardElementOperation	Remove;

	public:
		virtual bool DoExists( String APathName ) override
		{
			return FOwner.DoExists( JoinPath( APathName ));
		}

		virtual bool DoCreate( String APathName ) override
		{
			return FOwner.DoCreate( JoinPath( APathName ));
		}

		virtual bool DoRemove( String APathName ) override
		{
			return FOwner.DoRemove( JoinPath( APathName ));
		}

		virtual bool DoOpen( String APathName, uint8_t mode, File &AFile ) override
		{
			return FOwner.DoOpen( JoinPath( APathName ), mode, AFile );
		}

	protected:
		String JoinPath( String AValue )
		{
			String APath = PathName;
			if( ! APath.endsWith( "/" ) )
				APath = APath + "/";

			if( AValue.startsWith( "/" ) )
				AValue.remove( 0, 1 );

			return APath + AValue;
		}

	protected:
		void DoCheckExistsClockReceive( void *_Data )
		{
			CheckExists.OutputPin.SendValue( FOwner.DoExists( PathName ));
		}

		void DoCreateClockReceive( void *_Data )
		{
			Create.OutputPin.SendValue( FOwner.DoCreate( PathName ));
		}

		void DoRemoveClockReceive( void *_Data )
		{
			Remove.OutputPin.SendValue( FOwner.DoRemove( PathName ));
		}

	public:
		MicroSDCardElementDirectory( Mitov::MicroSDCard_Intf &AOwner ) :
			inherited( AOwner )
		{
			CheckExists.ClockInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MicroSDCardElementDirectory::DoCheckExistsClockReceive );
			Create.ClockInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MicroSDCardElementDirectory::DoCreateClockReceive );
			Remove.ClockInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MicroSDCardElementDirectory::DoRemoveClockReceive );
		}

	};
//---------------------------------------------------------------------------
	class MicroSDCardElementClocked : public MicroSDCardElementBasic
	{
		typedef Mitov::MicroSDCardElementBasic inherited;

    public:
		OpenWire::SinkPin	ClockInputPin;

	protected:
		virtual void DoClockReceive( void *_Data ) = 0;

	public:
		MicroSDCardElementClocked( Mitov::MicroSDCard_Intf &AOwner ) :
			inherited( AOwner )
		{
			ClockInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MicroSDCardElementClocked::DoClockReceive );
		}

	};
//---------------------------------------------------------------------------
	class MicroSDCardElementClockedResult : public MicroSDCardElementClocked
	{
		typedef Mitov::MicroSDCardElementClocked inherited;

	public:
		OpenWire::SourcePin	OutputPin;

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MicroSDCardElementList : public MicroSDCardElementClockedResult
	{
		typedef Mitov::MicroSDCardElementClockedResult inherited;

	public:
		OpenWire::SourcePin	FailedOutputPin;

	public:
		bool	Recursive = false;

	protected:
		void ProcessLevel( String APath, String APrefix )
		{
			File AFile;
			if( ! FOwner.DoOpen( APath, FILE_READ, AFile ))
			{
				FailedOutputPin.Notify( NULL );
				return;
			}

			if( APath.endsWith( "/" ))
				APath = APath.substring( 0, APath.length() - 1 );

			while (true) 
			{
				File AEntry =  AFile.openNextFile();
				if (! AEntry) 
				  // no more files
				  break;

				String AItemName = APrefix + AEntry.name(); 
				OutputPin.Notify( (char *)AItemName.c_str() );

				if( Recursive )
					if( AEntry.isDirectory())
						ProcessLevel( APath + '/' + AEntry.name(), AItemName + '/' );

				AEntry.close();
			  }

			AFile.close();
		}

	protected:
		virtual void DoClockReceive( void *_Data )
		{
//			Serial.println( "" );
//			Serial.println( PathName );
//			Serial.println( "" );
			if( PathName = "" )
				ProcessLevel( "/", "" );

			else
				ProcessLevel( PathName, "" );
/*
			File AFile;
			if( ! FOwner.DoOpen( PathName, FILE_READ, AFile ))
			{
				FailedOutputPin.Notify( NULL );
				return;
			}

			while (true) 
			{
				File AEntry =  AFile.openNextFile();
				if (! AEntry) 
				  // no more files
				  break;

				OutputPin.Notify( AEntry.name() );
				AEntry.close();
			  }

			AFile.close();
*/
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class MicroSDCardElementFile : public MicroSDCardElementBasic
	{
		typedef Mitov::MicroSDCardElementBasic inherited;

	public:
		OpenWire::SourcePin	ReadPositionOutputPin;
		OpenWire::SourcePin	WritePositionOutputPin;

		OpenWire::SinkPin	CloseInputPin;
		OpenWire::SinkPin	FlushInputPin;

	public:
		bool Enabled : 1;
		bool NewLine : 1;
		bool KeepFlushed : 1;
		bool KeepClosed : 1;

	public:
		bool FHasWrite : 1;

	protected:
		bool        FWriteMoved : 1;
		bool        FReadMoved : 1;

		uint32_t	FWritePosition = 0;
		uint32_t	FReadPosition = 0;

	public:
		File FFile;

	public:
		template<typename T> void Print( T AValue )
		{
			if( ! TryOpen() )
				return;

			PrepareWrite();

//			Serial.println( "TEST2" );
			if( NewLine )
				FFile.println( AValue );

			else
				FFile.print( AValue );

			UpdateWrite();
		}

		void Write( void *_Data, size_t ASize )
		{
			if( ! TryOpen() )
				return;

			PrepareWrite();
			FFile.write( (uint8_t *)_Data, ASize );
			UpdateWrite();
		}

		void BeginRead()
		{
			if( ! FWriteMoved )
			{
				FWritePosition = FFile.position();
				FWriteMoved = true;
			}

			if( FReadMoved )
			{
				FFile.seek( FReadPosition );
				FReadMoved = false;
			}
		}

		inline void ResetReadPosition()
		{
			FFile.seek( FReadPosition );
		}

	protected:
		void PrepareWrite()
		{
			if( ! FReadMoved )
			{
				FReadPosition = FFile.position();
				FReadMoved = true;
			}

			if( FWriteMoved )
			{
				FFile.seek( FWritePosition );
				FWriteMoved = false;
			}
		}

		void UpdateWrite()
		{
			if( KeepFlushed )
				FFile.flush();

			if( KeepClosed )
				FFile.close();
			
			UpdateWritePosition();
		}

	public:
		bool TryOpen()
		{
			if( FFile )
				return true;

//			Serial.println( "OPEN1" );
			if( ! Enabled )
				return false;

			if( PathName == "" )
				return false;

//			Serial.println( PathName );
//			Serial.println( FHasWrite );
			if( FHasWrite )
				return FOwner.DoOpen( PathName, FILE_WRITE, FFile );

			else
				return FOwner.DoOpen( PathName, FILE_READ, FFile );
		}

	protected:
		void UpdateWritePosition()
		{
			if( WritePositionOutputPin.IsConnected() )
				WritePositionOutputPin.SendValue( FFile.position() );
		}

	protected:
		void DoCloseReceive( void *_Data )
		{
			FFile.close();
		}

		void DoFlushReceive( void *_Data )
		{
			FFile.flush();
		}

	public:
		MicroSDCardElementFile( Mitov::MicroSDCard_Intf &AOwner, bool AHasWrite ) :
			inherited( AOwner ),
			Enabled( true ),
			NewLine( true ),
			KeepFlushed( true ),
			KeepClosed( true ),
			FHasWrite( AHasWrite ),
			FReadMoved( false ),
			FWriteMoved( false )
		{
			CloseInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MicroSDCardElementFile::DoCloseReceive );
			FlushInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MicroSDCardElementFile::DoFlushReceive );
		}

	};
//---------------------------------------------------------------------------
	class MicroSDCardFileElementBasic : public OpenWire::Object
	{
		typedef OpenWire::Object inherited;

	protected:
		MicroSDCardElementFile	&FOwner;

	public:
		MicroSDCardFileElementBasic( Mitov::MicroSDCardElementFile &AOwner ) :
			FOwner( AOwner )
		{
		}

	};
//---------------------------------------------------------------------------
	class MicroSDCardFileElementBasicClocked : public MicroSDCardFileElementBasic
	{
		typedef MicroSDCardFileElementBasic inherited;

    public:
		OpenWire::SinkPin	ClockInputPin;
		OpenWire::SourcePin	OutputPin;

	protected:
		virtual void DoClockReceive( void *_Data ) = 0;

	public:
		MicroSDCardFileElementBasicClocked( Mitov::MicroSDCardElementFile &AOwner ) :
			inherited( AOwner )
		{
			ClockInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MicroSDCardFileElementBasicClocked::DoClockReceive );
		}

	};
//---------------------------------------------------------------------------
	class MicroSDCardFileElementReadTextLine : public MicroSDCardFileElementBasicClocked
	{
		typedef MicroSDCardFileElementBasicClocked inherited;

	protected:
		bool readStringLine( String &AResult )
		{
//			AResult = "";
			int c = FOwner.FFile.read();
//			Serial.println( c );
			if( c < 0 )
				return false;

//			Serial.println( "R1" );
			while (c >= 0 && c != '\n' && c != '\r' )
			{
				AResult += (char)c;
				c = FOwner.FFile.read();
			}

			while ( c >= 0 )
			{
				c = FOwner.FFile.peek();
				if( c != '\n' && c != '\r' )
					break;

				FOwner.FFile.read();
			}
/*
			while ( c >= 0 && ( c == '\n' || c == '\r' ))
				c = FOwner.FFile.read();
*/
			return true;
		}

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			if( ! FOwner.TryOpen() )
				return;

			String ALine;			
			FOwner.BeginRead();
//			Serial.println( "READ1" );
			if( readStringLine( ALine ))
				OutputPin.Notify( (void *)ALine.c_str() );

		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	template<typename T> class MicroSDCardFileElementReadTyped : public MicroSDCardFileElementBasicClocked
	{
		typedef MicroSDCardFileElementBasicClocked inherited;

	protected:
		virtual void DoClockReceive( void *_Data )
		{
			if( ! FOwner.TryOpen() )
				return;

			FOwner.BeginRead();

			T AData;
			size_t AReadSize = FOwner.FFile.readBytes( (uint8_t *)&AData, sizeof( AData ));
			if( AReadSize == sizeof( AData ) )
				OutputPin.Notify( &AData );

//			else
//				FOwner.ResetReadPosition();
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	template<typename T> class MicroSDCardFileElementWriteTyped : public MicroSDCardFileElementBasic
	{
		typedef MicroSDCardFileElementBasic inherited;

    public:
		OpenWire::SinkPin	InputPin;

	protected:
		void DoDataReceive( void *_Data )
		{
			FOwner.Write( _Data, sizeof( T ));
		}

	public:
		MicroSDCardFileElementWriteTyped( Mitov::MicroSDCardElementFile &AOwner ) :
			inherited( AOwner )
		{
			AOwner.FHasWrite = true;

			InputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MicroSDCardFileElementWriteTyped::DoDataReceive );
		}

	};
//---------------------------------------------------------------------------
	class MicroSDCardElementExists : public MicroSDCardElementClockedResult
	{
		typedef Mitov::MicroSDCardElementClockedResult inherited;

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			OutputPin.SendValue( FOwner.DoExists( PathName ));
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
//	MicroSDCardElementDirectory
	template<int PIN_NUMBER> class MicroSDCard : public Mitov::EnabledComponent, public MicroSDCard_Intf
	{
		typedef Mitov::EnabledComponent inherited;

	public:
		OpenWire::SourcePin	FailedOutputPin;
		OpenWire::SourcePin	SuccessOutputPin;

	public:
		void SetEnabled( bool AValue )
		{
			if( Enabled == AValue )
				return;

			Enabled = AValue;
			UpdateEnabled();
		}

	public:
		virtual bool DoExists( String APathName )
		{
			if( !FCard )
				return false;

//			Serial.println( "COOL1" );
//			Serial.println( APathName );
			char *ATmp = new char[ APathName.length() + 1 ];
			memcpy( ATmp, APathName.c_str(), APathName.length() + 1 );

			bool AResult = FCard->exists( ATmp );

			delete [] ATmp;

			return AResult;
		}

		virtual bool DoCreate( String APathName )
		{
			if( !FCard )
				return false;

			char *ATmp = new char[ APathName.length() + 1 ];
			memcpy( ATmp, APathName.c_str(), APathName.length() + 1 );

			bool AResult = FCard->mkdir( ATmp );

			delete [] ATmp;

			return AResult;
		}

		virtual bool DoRemove( String APathName )
		{
			if( !FCard )
				return false;

			char *ATmp = new char[ APathName.length() + 1 ];
			memcpy( ATmp, APathName.c_str(), APathName.length() + 1 );

			bool AResult = FCard->rmdir( ATmp );

			delete [] ATmp;

			return AResult;
		}

		virtual bool DoOpen( String APathName, uint8_t mode, File &AFile )
		{
//			Serial.print( "O1 :" );
			if( !FCard )
				return false;

			if( ! APathName.startsWith( "/" ))
				APathName = String( "/" ) + APathName;

			char *ATmp = new char[ APathName.length() + 1 ];
			memcpy( ATmp, APathName.c_str(), APathName.length() + 1 );

//			Serial.print( "T1 :" );
//			Serial.println( ATmp );
			AFile = FCard->open( ATmp, mode );
			delete [] ATmp;

			return AFile;
		}

	protected:
		SDClass	*FCard = nullptr;
		
	protected:
		void UpdateEnabled()
		{
			if( Enabled )
			{
//				Serial.println( "TEST1" );
				FCard = new SDClass;				
				if( FCard->begin( PIN_NUMBER ) )
					SuccessOutputPin.Notify( NULL );

				else
				{
//					Serial.println( "FAIL" );
					FailedOutputPin.Notify( NULL );
					Enabled = false;
					UpdateEnabled();
				}
//				Serial.println( "COOL" );
			}
			else
			{
				delete FCard;
				FCard = nullptr;
			}
		}

	protected:
		virtual void SystemInit()
		{
			UpdateEnabled();
			inherited::SystemInit();
		}
/*
		virtual void SystemStart() 
		{
//			FServo.attach( PIN_NUMBER );
			inherited::SystemStart();
		}
*/

/*
		virtual ~MicroSDCard()
		{
			if( FCard )
				delete FCard;
		}
*/
	};
}

#endif

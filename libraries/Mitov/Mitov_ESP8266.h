////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_ESP8266_h
#define _MITOV_ESP8266_h

#include <Mitov.h>
#include <Mitov_BasicEthernet.h>
#include <Mitov_StringPrint.h>

//#define __ESP8266__DEBUG__

namespace Mitov
{
	class ESP8266;
//---------------------------------------------------------------------------
	class ESP8266RemoteAccessPoint
	{
	public:
		bool	Enabled = true;
		String	SSID;
	    String	Password;
	};
//---------------------------------------------------------------------------
	class ESP8266Encription
	{
	public:
		bool	WPA = false;
		bool	WPA2 = false;
	};
//---------------------------------------------------------------------------
	class ESP8266AccessPoint
	{
	public:
		bool	Enabled = true;
		String	SSID;
		String	Password;

		unsigned long		Channel = 1;
		ESP8266Encription	Encription;
	};
//---------------------------------------------------------------------------
	class BasicESP8266Socket : public Mitov::BasicSocket
	{
		typedef Mitov::BasicSocket inherited;

	protected:
		ESP8266 &FModule;
		bool	FRunning = false;

	protected:
		BufferPrint	FStringPrint;

	public:
		virtual bool IsEnabled()
		{
			return Enabled && FRunning; //FModule.Enabled;
		}

		virtual Print *GetPrint()
		{
			return &FStringPrint;
		}

		virtual void BeginPacket()
		{
			FStringPrint.Value.clear();
		}

		virtual void DataReceived( int connectionId, unsigned char AData ) = 0;

	public:
		BasicESP8266Socket( ESP8266 &AModule );
	};
//---------------------------------------------------------------------------
	class ESP8266 : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		OpenWire::SourcePin	AccessPointAddressOutputPin;
		OpenWire::SourcePin	AccessPointMACOutputPin;

		OpenWire::SourcePin	StationAddressOutputPin;
		OpenWire::SourcePin	StationMACOutputPin;

	public:
		ESP8266AccessPoint	AccessPoint;
		Mitov::SimpleObjectList<ESP8266RemoteAccessPoint*>	RemoteAccessPoints;

	public:
		Mitov::SimpleList<BasicESP8266Socket*>	Sockets;

	protected:
		Mitov::BasicSerialPort &FSerial;

	protected:
		Mitov::BasicESP8266Socket	*FServerInstance;

	public:
		Mitov::BasicESP8266Socket	*FConnections[ 4 ];

		int	FReadLength = 0;
		int FReadConnectionId = 0;

	public:
		bool AssignServerID( BasicESP8266Socket *ASocket )
		{
			bool AResult = false;
			for( int i = 0; i < 4; ++i )
				if( ! FConnections[ i ] )
				{
					FConnections[ i ] = ASocket;
					FServerInstance = ASocket;
					AResult = true;
				}

			return AResult;
		}

		void ReleaseServerID( BasicESP8266Socket *ASocket )
		{
			if( FServerInstance == ASocket )
				FServerInstance = NULL;

			for( int i = 0; i < 4; ++i )
				if( FConnections[ i ] == ASocket )
					FConnections[ i ] = NULL;
		}

		bool AssignConnectionID( BasicESP8266Socket *ASocket, int &AID )
		{
			for( int i = 0; i < 4; ++i )
				if( ! FConnections[ i ] )
				{
					FConnections[ i ] = ASocket;
					AID = i;
					return true;
				}

			return false;
		}

		void ReleaseConnectionID( int AID )
		{
			FConnections[ AID ] = FServerInstance;
		}

		void SendDirect(String data )
		{
#ifdef __ESP8266__DEBUG__
			Serial.println( data );
#endif
			FSerial.GetStream().print( data );
			WaitSentOK( 2000 );
		}

		void SendDirect( uint8_t *AData, int ASize )
		{
#ifdef __ESP8266__DEBUG__
			Serial.write( AData, ASize );
#endif
			FSerial.GetStream().write( AData, ASize );
			WaitSentOK( 2000 );
		}

		void WaitSentOK( const int timeout )
		{
			String response = "";
			long int time = millis();
			 
			while( timeout > millis() - time)
			{
			  while(FSerial.GetStream().available())
			  {     
				// The esp has data so collecti it 
				char c = FSerial.GetStream().read(); // read the next character.
				response += c;
			  }  

			  if( response.indexOf( "SEND OK\r\n" ) >= 0 )
				  break;

			}
		}

		String SendData(String command, String response_key, const int timeout)
		{
			String response = "";
    
#ifdef __ESP8266__DEBUG__
			Serial.println( command );
#endif
			FSerial.GetStream().print( command + "\r\n" ); // send the read character to the esp8266
    
			long int time = millis();
    
			while( timeout > millis() - time)
			{
			  while(FSerial.GetStream().available())
			  {     
				// The esp has data so collecti it 
				char c = FSerial.GetStream().read(); // read the next character.
				response += c;
			  }  

			  if( response.indexOf( response_key ) >= 0 )
				  break;

			}
    
#ifdef __ESP8266__DEBUG__
			Serial.println( response );
#endif
			return response;
		}

		bool SendData(String command, const int timeout, String &AResponse )
		{
			AResponse = "";
    
#ifdef __ESP8266__DEBUG__
			Serial.println( command );
#endif

			FSerial.GetStream().print( command + "\r\n" ); // send the read character to the esp8266
    
			long int time = millis();
    
			while( timeout > millis() - time)
			{
				while(FSerial.GetStream().available())
				{     
					// The esp has data so collecti it 
					char c = FSerial.GetStream().read(); // read the next character.
					AResponse += c;
				}  

				if( AResponse.indexOf( "\r\nOK\r\n" ) >= 0 )
				{
#ifdef __ESP8266__DEBUG__
					Serial.println( AResponse );
#endif
					return true;
				}

				if( AResponse.indexOf( "\r\nERROR\r\n" ) >= 0 )
				{
#ifdef __ESP8266__DEBUG__
					Serial.println( AResponse );
#endif
					return false;
				}

				if( AResponse.indexOf( "\r\nFAIL\r\n" ) >= 0 )
				{
#ifdef __ESP8266__DEBUG__
					Serial.println( AResponse );
#endif
					return false;
				}

			}
    
#ifdef __ESP8266__DEBUG__
			Serial.println( AResponse );
#endif
			return false;
		}

		bool SendData( String command, const int timeout )
		{
			String AResponse;
			return SendData( command, timeout, AResponse );
		}

	protected:
		virtual void SystemStart()
		{
			inherited::SystemStart();

#ifdef __ESP8266__DEBUG__
			Serial.println( "SystemStart" );
#endif
			SendData( "AT+RST", "\r\nready\r\n", 5000 );
//			String AResponse = SendData( "AT+RST", "\r\nready\r\n", 5000 );
//			Serial.println( AResponse );

#ifdef __ESP8266__DEBUG__
			Serial.println( "INIT" );
#endif

			int AMode = 0;

			for( Mitov::SimpleObjectList<ESP8266RemoteAccessPoint*>::iterator Iter = RemoteAccessPoints.begin(); Iter != RemoteAccessPoints.end(); ++Iter )
				if((* Iter)->Enabled )
				{
					AMode = 1;
					break;
				}

			if( AMode == 0 || AccessPoint.Enabled )
				AMode |= 2; // Set it as access point so others can connect

//			AMode = 3;
			SendData( "AT+CWMODE=" + String( AMode ), 5000 );

//			Serial.println( "TTT" );
//			return;

			if( AccessPoint.Enabled )
//				if( AccessPoint.Encription )
			{
				String ASSID;
				if( AccessPoint.SSID != "" )
					ASSID = AccessPoint.SSID;

				else
					ASSID = "Arduino";

				String AEncoding;
				if( AccessPoint.Encription.WPA )
				{
					if( AccessPoint.Encription.WPA2 )
						AEncoding = "4";

					else
						AEncoding = "2";
				}

				else
				{
					if( AccessPoint.Encription.WPA2 )
						AEncoding = "3";

					else
						AEncoding = "0";
				}

				SendData( "AT+CWSAP=\"" + ASSID + "\",\"" + AccessPoint.Password + "\"," + AccessPoint.Channel + "," + AEncoding, 1000 );
			}

			for( Mitov::SimpleObjectList<ESP8266RemoteAccessPoint*>::iterator Iter = RemoteAccessPoints.begin(); Iter != RemoteAccessPoints.end(); ++Iter )
				if((* Iter)->Enabled )
					if((* Iter)->SSID != "" )
						if( SendData( "AT+CWJAP=\"" + (* Iter)->SSID + "\",\"" + (* Iter)->Password + "\"", 20000 ))
							break;	

			if( AccessPointAddressOutputPin.IsConnected() || AccessPointMACOutputPin.IsConnected() || StationAddressOutputPin.IsConnected() || StationMACOutputPin.IsConnected() )
			{
//				Serial.println( "ADDRESS:" );
				String AResponse = SendData( "AT+CIFSR", "\r\nOK\r\n",1000 );
#ifdef __ESP8266__DEBUG__
				Serial.println( AResponse );
#endif

				int AAddressPos = AResponse.indexOf( "+CIFSR:APIP,\"" );

				int AAddressEndPos;
				if( AAddressPos >= 0 )
				{
					AAddressEndPos = AResponse.indexOf( "\"", AAddressPos + 13 );
					if( AAddressEndPos >= 0 )
					{
						String Addresses = AResponse.substring( AAddressPos + 13, AAddressEndPos );
						AccessPointAddressOutputPin.Notify( (void *)Addresses.c_str() );
					}

					else
						AAddressEndPos = 0;

				}

				else
					AAddressEndPos = 0;

				AAddressPos = AResponse.indexOf( "+CIFSR:APMAC,\"", AAddressEndPos );
				if( AAddressPos >= 0 )
				{
					AAddressEndPos = AResponse.indexOf( "\"", AAddressPos + 14 );
					if( AAddressEndPos >= 0 )
					{
						String Addresses = AResponse.substring( AAddressPos + 14, AAddressEndPos );
						AccessPointMACOutputPin.Notify( (void *)Addresses.c_str() );
					}

					else
						AAddressEndPos = 0;

				}				
				else
					AAddressEndPos = 0;

				AAddressPos = AResponse.indexOf( "+CIFSR:STAIP,\"", AAddressEndPos );
				if( AAddressPos >= 0 )
				{
					AAddressEndPos = AResponse.indexOf( "\"", AAddressPos + 14 );
					if( AAddressEndPos >= 0 )
					{
						String Addresses = AResponse.substring( AAddressPos + 14, AAddressEndPos );
						StationAddressOutputPin.Notify( (void *)Addresses.c_str() );
					}

					else
						AAddressEndPos = 0;

				}				
				else
					AAddressEndPos = 0;

				AAddressPos = AResponse.indexOf( "+CIFSR:STAMAC,\"", AAddressEndPos );
				if( AAddressPos >= 0 )
				{
					AAddressEndPos = AResponse.indexOf( "\"", AAddressPos + 15 );
					if( AAddressEndPos >= 0 )
					{
						String Addresses = AResponse.substring( AAddressPos + 15, AAddressEndPos );
						StationMACOutputPin.Notify( (void *)Addresses.c_str() );
					}

					else
						AAddressEndPos = 0;

				}				
				else
					AAddressEndPos = 0;

			}

			SendData( "AT+CIPMUX=1",1000 );
		}

		virtual void SystemLoopBegin( unsigned long currentMicros ) 
		{
			if( FSerial.GetStream().available() )
			{
#ifdef __ESP8266__DEBUG__
//				Serial.println( "AVALIABLE" );
#endif
				if( ! FReadLength )
					if(FSerial.GetStream().find("+IPD,"))
					{
#ifdef __ESP8266__DEBUG__
				Serial.println( "DATA" );
#endif
//					int connectionId = FSerial.GetStream().read() - '0';
						String AIDStr = FSerial.GetStream().readStringUntil( ',' );
						FReadConnectionId = AIDStr.toInt();
#ifdef __ESP8266__DEBUG__
						Serial.println( AIDStr );
						Serial.println( FReadConnectionId );
#endif
//					FSerial.GetStream().find("," ); // Skip ','
//					FSerial.GetStream().read(); // Skip ','
						String ALengthStr = FSerial.GetStream().readStringUntil( ':' );
#ifdef __ESP8266__DEBUG__
						Serial.println( "Length = " + ALengthStr );
#endif
						FReadLength = ALengthStr.toInt();
					}

				if( FReadLength )
					if( FReadConnectionId >= 0 && FReadConnectionId < 4 )
					{
						while( FReadLength )
						{
							if( ! FSerial.GetStream().available() )
								break;

							int AData = FSerial.GetStream().read();
//#ifdef __ESP8266__DEBUG__
//							Serial.println( (char)AData );
//#endif
							if( FConnections[ FReadConnectionId ] )
								FConnections[ FReadConnectionId ]->DataReceived( FReadConnectionId, AData );

							--FReadLength;
						}
					}

			}

			inherited::SystemLoopBegin( currentMicros );
		}

	public:
		ESP8266( Mitov::BasicSerialPort &ASerial ) :
			FSerial( ASerial )
		{
			memset( FConnections, 0, 4 * sizeof( FConnections[ 0 ] ) );
		}

	};
//---------------------------------------------------------------------------
	class ESP8266TCPServerSocket : public BasicESP8266Socket
	{
		typedef BasicESP8266Socket inherited;

//		EthernetServer	*FServer;
//		EthernetClient	FClient;

		int	FClientCurrentID = -1;

	protected:
		virtual void StartSocket()
		{
#ifdef __ESP8266__DEBUG__
				Serial.println( "SERVER::StartSocket" );
#endif
//			if( FModule.AssignConnectionID( this ) )
			if( FModule.AssignServerID( this ) )
			{
				FModule.SendData( "AT+CIPSERVER=1," + String( Port ), 2000 );
				FRunning = true;
			}

		}

/*
		virtual void SystemLoopBegin( unsigned long currentMicros ) 
		{
			inherited::SystemLoopBegin( currentMicros );
		}
*/
		virtual void DataReceived( int connectionId, unsigned char AData )
		{
#ifdef __ESP8266__DEBUG__
				Serial.print( "RECEIVED: " );
				Serial.println( connectionId );
#endif
			FClientCurrentID = connectionId;
			OutputPin.Notify( &AData );
//			FClientCurrentID = -1;
		}

	public:
		virtual void EndPacket()
		{
			if( FStringPrint.Value.size() == 0 )
				return;

			if( FClientCurrentID >= 0 )
			{
				if( FModule.SendData( "AT+CIPSEND=" + String( FClientCurrentID ) + "," + String( FStringPrint.Value.size()), "\r\n> ", 2000 ))
					FModule.SendDirect( (uint8_t *)FStringPrint.Value, FStringPrint.Value.size() );

//				Serial.println( FStringPrint.Value );
			}

			else
			{
/*
				for( int i = 0; i < 4; ++ i )
					if( FModule.FConnections[ i ] == this )
					{
						if( FModule.SendData( "AT+CIPSEND=" + String( i ) + "," + String( FStringPrint.Value.size()), "\r\n> ", 2000 ))
							FModule.SendDirect( (uint8_t *)FStringPrint.Value, FStringPrint.Value.size() );

					}
*/
			}

//			FStringPrint.Value.clear();
		}

	public:
		virtual void StopSocket()
		{
			if( FRunning )
			{
				FModule.SendData( "AT+CIPSERVER=0," + String( Port ), 2000 );
				FModule.ReleaseServerID( this );
				FRunning = false;
			}
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class ESP8266TCPClientSocket : public BasicESP8266Socket
	{
		typedef BasicESP8266Socket inherited;

	public:
		String		URL;
		int			FID;

	protected:
		virtual void StartSocket()
		{
#ifdef __ESP8266__DEBUG__
				Serial.println( "CLIENT::StartSocket" );
#endif
			if( FModule.AssignConnectionID( this, FID ) )
			{
				FModule.SendData( "AT+CIPSTART=" + String( FID ) +",\"TCP\",\"" + URL + "\"," + String( Port ), 2000 );
				FRunning = true;
			}

//			Serial.println( "StartSocket" );
		}

		virtual void StopSocket()
		{
			if( FRunning )
			{
				FModule.ReleaseConnectionID( FID );
				FModule.SendData( "AT+CIPCLOSE=" + String( FID ), 2000 );
				FRunning = false;
			}
//			FClient.stop();
		}

		virtual void DataReceived( int connectionId, unsigned char AData )
		{
			OutputPin.Notify( &AData );
		}

	public:
		virtual void EndPacket()
		{
			if( FStringPrint.Value.size() == 0 )
				return;

			if( FModule.SendData( "AT+CIPSEND=" + String( FID ) + "," + String( FStringPrint.Value.size()), "\r\n> ", 2000 ))
				FModule.SendDirect( (uint8_t *)FStringPrint.Value, FStringPrint.Value.size() );

//			FStringPrint.Value.clear();
		}

	public:
/*
		virtual void SystemLoopBegin( unsigned long currentMicros ) 
		{
			inherited::SystemLoopBegin( currentMicros );
		}
*/
	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
	BasicESP8266Socket::BasicESP8266Socket( ESP8266 &AModule ) :
		FModule( AModule )
	{
		AModule.Sockets.push_back( this );
	}
//---------------------------------------------------------------------------
}

#endif

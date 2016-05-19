////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_BASIC_ETHERNET_h
#define _MITOV_BASIC_ETHERNET_h

#include <Mitov.h>
//#include <Ethernet.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class TMACAddress
	{
	public:
		byte FMacAddress[6];

	public:
		TMACAddress(
				uint8_t mac_0, 
				uint8_t mac_1, 
				uint8_t mac_2, 
				uint8_t mac_3, 
				uint8_t mac_4, 
				uint8_t mac_5 )
		{
			FMacAddress[ 0 ] = mac_0;
			FMacAddress[ 1 ] = mac_1;
			FMacAddress[ 2 ] = mac_2;
			FMacAddress[ 3 ] = mac_3;
			FMacAddress[ 4 ] = mac_4;
			FMacAddress[ 5 ] = mac_5;
		}

		TMACAddress()
		{
			memset( FMacAddress, 0, sizeof( FMacAddress ) );
		}
	};
//---------------------------------------------------------------------------
	inline String MACAdressToString( uint8_t *AMACAddress )
	{
		char AMACString[ 6 * 3 + 1 ];
		sprintf( AMACString, "%02X-%02X-%02X-%02X-%02X-%02X", AMACString[ 0 ], AMACString[ 1 ], AMACString[ 2 ], AMACString[ 3 ], AMACString[ 4 ], AMACString[ 5 ] );
		return AMACString;
//		MACOutputPin.Notify( AMACString );
	}
//---------------------------------------------------------------------------
	inline String IPAdressToString( ::IPAddress AIPAddress )
	{
		char AIPString[ 4 * 4 + 1 ];
		sprintf( AIPString, "%u.%u.%u.%u", AIPAddress[ 0 ], AIPAddress[ 1 ], AIPAddress[ 2 ], AIPAddress[ 3 ] );
		return AIPString;
	}
//---------------------------------------------------------------------------
	class BasicShieldIPAddress
	{
	public:
		bool		Enabled = false;
		::IPAddress	IP;

	};
//---------------------------------------------------------------------------
	class ShieldGatewayAddress : public BasicShieldIPAddress
	{
	public:
		BasicShieldIPAddress	Subnet;
	};
//---------------------------------------------------------------------------
	class ShieldDNSAddress : public BasicShieldIPAddress
	{
	public:
		ShieldGatewayAddress	Gateway;
	};
//---------------------------------------------------------------------------
	class ShieldIPAddress : public BasicShieldIPAddress
	{
	public:
		ShieldDNSAddress	DNS;
	};
//---------------------------------------------------------------------------
	class ShieldIPDNS2Address : public ShieldIPAddress
	{
	public:
		BasicShieldIPAddress	DNS2;
	};
//---------------------------------------------------------------------------
	class BasicSocket : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
//		OpenWire::SinkPin	InputPin;
		OpenWire::SourcePin	OutputPin;

	public:
		bool			Enabled = true;
		unsigned int	Port = 80;

	protected:
		virtual void StartSocket() = 0;

	public:
		void SetEnabled( bool AValue )
		{
            if( Enabled == AValue )
                return;

            Enabled = AValue;
			if( IsEnabled() )
				StartSocket();

			else
				StopSocket();

		}

	public:
		virtual bool IsEnabled()
		{
			return Enabled;
		}

		virtual bool CanSend()
		{
			return Enabled;
		}

		virtual void BeginPacket()
		{
		}

		virtual void EndPacket()
		{
		}

		virtual void StopSocket() = 0;

	public:
		virtual void SystemStart()
		{
//			Serial.println( Enabled );
			if( Enabled )
				StartSocket();

			inherited::SystemStart();
		}

	};
//---------------------------------------------------------------------------
	template<typename T_OWNER> class BasicEthernetSocket : public Mitov::BasicSocket
	{
		typedef Mitov::BasicSocket inherited;

	protected:
		T_OWNER &FOwner;

	public:
		virtual bool IsEnabled()
		{
			return Enabled && FOwner.Enabled;
		}

		virtual bool CanSend()
		{
			return Enabled && FOwner.Enabled;
		}

		virtual Print *GetPrint() = 0;

	public:
		BasicEthernetSocket( T_OWNER &AOwner ) :
			FOwner( AOwner )
		{
			AOwner.Sockets.push_back( this );
		}

	};
//---------------------------------------------------------------------------
	template<typename T_OWNER, typename T_CLIENT> class BasicEthernetConnectSocket : public Mitov::BasicEthernetSocket<T_OWNER>
	{
		typedef Mitov::BasicEthernetSocket<T_OWNER> inherited;

	public:
		OpenWire::TypedSourcePin<bool>	ConnectedOutputPin;

		OpenWire::SinkPin	FlushInputPin;
		OpenWire::SinkPin	DisconnectInputPin;

	protected:
		T_CLIENT	FClient;

	protected:
		void DoDisconnect( void *_Data )
		{
			FClient.flush();
			FClient.stop();
			ConnectedOutputPin.SetValue( false, true );
//			Serial.println( "DoDisconnect" );
//			Serial.println( FClient.connected() );
		}

		void DoFlush( void *_Data )
		{
			FClient.flush();
		}

	public:
		virtual Print *GetPrint()
		{
			return &FClient;
		}

	public:
		virtual void SystemInit()
		{
			ConnectedOutputPin.SetValue( false, false );
			inherited::SystemInit();
		}

	public:
		BasicEthernetConnectSocket( T_OWNER &AOwner ) :
			inherited( AOwner )
		{
			FlushInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&BasicEthernetConnectSocket::DoFlush );

			DisconnectInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&BasicEthernetConnectSocket::DoDisconnect );
		}

	};
//---------------------------------------------------------------------------
	template<typename T_OWNER, typename T_CLIENT> class TCPClientSocket : public BasicEthernetConnectSocket<T_OWNER, T_CLIENT>
	{
		typedef BasicEthernetConnectSocket<T_OWNER, T_CLIENT> inherited;

	public:
		String		URL;
		::IPAddress	IPAddress;

	protected:
		virtual void StartSocket()
		{
//			Serial.println( "StartSocket" );
			if( URL.length() )
				inherited::FClient.connect( URL.c_str(), inherited::Port );

			else
			{
//				IPAddress.printTo( Serial );
				inherited::FClient.connect( IPAddress, inherited::Port );
			}
		}

		virtual void StopSocket()
		{
			inherited::FClient.flush();
			inherited::FClient.stop();
			inherited::ConnectedOutputPin.SetValue( false, true );
		}

	public:
		virtual void SystemLoopBegin( unsigned long currentMicros ) 
		{
			if ( inherited::FClient.available() )
			{
				unsigned char AByte = inherited::FClient.read();
				inherited::OutputPin.SendValue( Mitov::TDataBlock( 1, &AByte ));
//				inherited::OutputPin.Notify( &AByte );
			}

			inherited::ConnectedOutputPin.SetValue( inherited::FClient.connected(), true );

			if (! inherited::FClient.connected()) 
			{
				inherited::FClient.stop(); // Do we need this?
				inherited::ConnectedOutputPin.SetValue( false, true );
			}

			inherited::SystemLoopBegin( currentMicros );
		}

	public:
		TCPClientSocket( T_OWNER &AOwner, ::IPAddress AIPAddress ) :
			inherited( AOwner ),
			IPAddress( AIPAddress )
		{
		}
	};
//---------------------------------------------------------------------------
	template<typename T_OWNER, typename T_SERVER, typename T_CLIENT> class TCPServerSocket : public BasicEthernetConnectSocket<T_OWNER, T_CLIENT>
	{
		typedef BasicEthernetConnectSocket<T_OWNER, T_CLIENT> inherited;

	protected:
		T_SERVER	*FServer;

	protected:
		virtual void StartSocket()
		{
//			Serial.println( "StartSockect" );
			if( FServer )
				return;

//			Serial.println( Port );
			FServer = new T_SERVER( inherited::Port );
			FServer->begin();
//			Serial.println( "Start Server Sockect" );
//			Serial.println( inherited::Port );
		}

		virtual void SystemLoopBegin( unsigned long currentMicros ) 
		{
			if( FServer )
			{
				if( ! inherited::FClient )
				{
//					Serial.println( inherited::Port );
//					Serial.println( "TRY CLIENT" );
					inherited::FClient = FServer->available();

//					if( inherited::FClient )
//						Serial.println( "CLIENT" );

//					Serial.println( "TRY CLIENT OUT" );
				}

/*
				if( inherited::FClient )
 					if (! inherited::FClient.connected()) 
					{
						Serial.println( "STOP" );
						inherited::FClient.stop(); // Do we need this?
					}
*/
				if( inherited::FClient )
				{
//					Serial.println( "CLIENT" );
					if( inherited::FClient.available() )
					{
						unsigned char AByte = inherited::FClient.read();
//				Serial.println( "RECEIVED" );
						inherited::OutputPin.SendValue( Mitov::TDataBlock( 1, &AByte ));
//						inherited::OutputPin.Notify( &AByte );
					}

 					if( ! inherited::FClient.connected() ) 
					{
//						Serial.println( "STOP" );
						inherited::FClient.stop(); // Do we need this?
						inherited::ConnectedOutputPin.SetValue( false, true );
					}
				}
			}

//			Serial.println( inherited::FClient.connected() );

			inherited::ConnectedOutputPin.SetValue( inherited::FClient.connected(), true );
//			Serial.println( "SYSTEM_LOOP" );
			inherited::SystemLoopBegin( currentMicros );
//			Serial.println( "SYSTEM_LOOP_OUT" );
		}

	public:
		virtual bool CanSend()
		{
			return inherited::Enabled && inherited::FOwner.Enabled && inherited::FClient;
		}

		virtual void StopSocket()
		{
			if( FServer )
			{
				delete FServer;
				FServer = NULL;
			}
		}

/*
		virtual void EndPacket()
		{
			Serial.println( "EndPacket" );
//			inherited::FClient.flush();
//			delay(1000);
//			inherited::FClient.stop();
		}
*/
	public:
		TCPServerSocket( T_OWNER &AOwner ) :
			inherited( AOwner ),
			FServer( NULL )
		{
		}

/*
		virtual ~TCPServerSocket()
		{
			if( FServer )
				delete FServer;
		}
*/
	};
//---------------------------------------------------------------------------
	template<typename T_OWNER, typename T_UDP> class UDPSocket : public BasicEthernetSocket<T_OWNER>
	{
		typedef BasicEthernetSocket<T_OWNER> inherited;

	public:
		OpenWire::SourcePin	RemoteIPOutputPin;
		OpenWire::SourcePin	RemotePortOutputPin;

	public:
		unsigned int	RemotePort = 80;
		::IPAddress		RemoteIPAddress;

	protected:
		T_UDP FSocket;
		String FHostName;
		bool   FResolved;

	public:
		virtual bool CanSend()
		{
			return FResolved && inherited::CanSend();
		}

	protected:
		virtual void StartSocket()
		{
			if( FHostName != "" )
			{
//				Serial.println( "TEST1" );
				FResolved = inherited::FOwner.GetIPFromHostName( FHostName, RemoteIPAddress );
			}

//			Serial.print( "StartSocket: " );
//			Serial.println( inherited::Port );
			FSocket.begin( inherited::Port );
		}

	public:
		virtual void BeginPacket()
		{
//			Serial.print( "BeginPacket: " );
//			Serial.print( RemoteIPAddress );
//			Serial.print( " " );
//			Serial.println( RemotePort );
			FSocket.beginPacket( RemoteIPAddress, RemotePort );
////  FSocket.beginPacket( RemoteIPAddress, 8888 );
//  FSocket.println( "Hello1" );
////  FSocket.endPacket();
		}

		virtual void EndPacket()
		{
//			Serial.println( "EndPacket" );
//			FSocket.println( "Hello" );
			FSocket.endPacket();
//			delay( 1000 );
		}

		virtual void StopSocket()
		{
//			Serial.println( "StopSocket" );
			FSocket.stop();
		}

		virtual Print *GetPrint()
		{
//  FSocket.println( "Hello2" );
//			return &Serial;
//			Serial.println( "GetPrint" );
			return &FSocket;
		}

	public:
		virtual void SystemLoopBegin( unsigned long currentMicros ) 
		{
			int APacketSize = FSocket.parsePacket();
			if( APacketSize )
			{
//				Serial.println( APacketSize );
				byte *Adata = new byte[ APacketSize ];
				FSocket.read( Adata, APacketSize );

				inherited::OutputPin.SendValue( Mitov::TDataBlock( APacketSize, Adata ));
				RemoteIPOutputPin.SendValue( IPAdressToString( FSocket.remoteIP() ));
				RemotePortOutputPin.SendValue( FSocket.remotePort() );

				delete []Adata;
			}
/*
			if ( FSocket.available() )
			{
				unsigned char AByte = FSocket.read();
				inherited::OutputPin.Notify( &AByte );
			}
*/
			inherited::SystemLoopBegin( currentMicros );
		}

	public:
		UDPSocket( T_OWNER &AOwner, ::IPAddress ARemoteIPAddress ) :
			inherited( AOwner ),
			RemoteIPAddress( ARemoteIPAddress ),
			FResolved( true )
		{
		}

		UDPSocket( T_OWNER &AOwner, String AHostName ) :
			inherited( AOwner ),
			FHostName( AHostName ),
			FResolved( false )
		{
		}
	};
//---------------------------------------------------------------------------
	class BasicEthernetShield : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		Mitov::SimpleList<BasicEthernetSocket<BasicEthernetShield>*>	Sockets;

	public:
		bool	Enabled = true;

	public:
		void SetEnabled( bool AValue )
		{
            if( Enabled == AValue )
                return;

            Enabled = AValue;
			if( Enabled )
				StartEthernet();

			else
				StopEthernet();

		}

	public:
		virtual bool GetIPFromHostName( String AHostName, ::IPAddress &AAdress ) = 0;

	protected:
		virtual void SystemInit()
		{
			if( Enabled )
				StartEthernet();

			inherited::SystemInit();
		}

		virtual void StopEthernet()
		{
			for( int i = 0; i < Sockets.size(); ++i )
				Sockets[ i ]->StopSocket();
		}

        void RestartEthernet()
		{
			if( ! Enabled )
				return;

//			T_SERIAL->end();
//			Ethernet.end();
			StartEthernet();
		}

		virtual void StartEthernet() = 0;

	};
//---------------------------------------------------------------------------
	template<typename T_ROOT, T_ROOT *T_INSTANCE, typename T> class EthernetSocketInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data )
		{
//			Serial.print( "DoReceive" );
			if( T_INSTANCE->CanSend() )
			{
				T_INSTANCE->BeginPacket();

//				Serial.print( "PRINT: " );
//				Serial.println( *(T*)_Data );

				T_INSTANCE->GetPrint()->println( *(T*)_Data );
				T_INSTANCE->EndPacket();
			}
		}
	};
//---------------------------------------------------------------------------
	template<typename T_ROOT, T_ROOT *T_INSTANCE> class EthernetSocketBinaryInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data )
		{
			if( T_INSTANCE->CanSend() )
			{
				T_INSTANCE->BeginPacket();
				Mitov::TDataBlock ADataBlock = *(Mitov::TDataBlock*)_Data;

//				Serial.println( ADataBlock.Size );
				T_INSTANCE->GetPrint()->write((uint8_t *) ADataBlock.Data, ADataBlock.Size );
//				T_INSTANCE->GetPrint()->write( *(unsigned char*)_Data );
				T_INSTANCE->EndPacket();
			}
		}
	};
//---------------------------------------------------------------------------
	template<typename T_ROOT, T_ROOT *T_INSTANCE> class EthernetSocketStringInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data )
		{
			if( T_INSTANCE->CanSend() )
			{
				T_INSTANCE->BeginPacket();
				T_INSTANCE->GetPrint()->println( (char*)_Data );
				T_INSTANCE->EndPacket();
			}
		}
	};
//---------------------------------------------------------------------------
	template<typename T_ROOT, T_ROOT *T_INSTANCE, typename T_OBJECT> class EthernetSocketObjectInput : public Mitov::CommonSink
	{
		typedef Mitov::CommonSink	inherited;

	protected:
		virtual void DoReceive( void *_Data )
		{
			if( T_INSTANCE->CanSend() )
			{
				T_INSTANCE->BeginPacket();
				Print *APrint = T_INSTANCE->GetPrint();
				APrint->println( ((T_OBJECT *)_Data)->ToString().c_str() );
				T_INSTANCE->EndPacket();
			}
		}
	};
//---------------------------------------------------------------------------
}	
#endif

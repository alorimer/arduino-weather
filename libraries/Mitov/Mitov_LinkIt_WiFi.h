////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_LINKIT_WIFI_h
#define _MITOV_LINKIT_WIFI_h

#include <Mitov.h>
#include <Mitov_BasicEthernet.h>
#include <LWiFi.h>
#include <LWiFiServer.h>
#include <LWiFiClient.h>
#include <LWiFiUDP.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	enum LinkItWiFiEncription { liweAuto, liweWEP, liweWPA };
//---------------------------------------------------------------------------
	class LinkItRemoteAccessPoint
	{
	public:
		bool	Enabled = true;
		String	SSID;
	    String	Password;
		LinkItWiFiEncription	Encription = liweAuto;

	public:
		bool Connect()
		{
			if( ! Enabled )
				return false;

			if( SSID == "" )
				return false;

			if( Password == "" )
				return ( LWiFi.connect( SSID.c_str() ) > 0 );


			switch( Encription )
			{
				case liweAuto:
				{
					if( LWiFi.connectWPA( SSID.c_str(), Password.c_str() ) > 0 )
						return true;

					return( LWiFi.connectWEP( SSID.c_str(), Password.c_str() ) > 0 );
				}

				case liweWEP:
					return( LWiFi.connectWEP( SSID.c_str(), Password.c_str() ) > 0 );

				case liweWPA:
					return( LWiFi.connectWPA( SSID.c_str(), Password.c_str() ) > 0 );
			}
		}
	};
//---------------------------------------------------------------------------
	class LinkItWiFiModule : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		OpenWire::SinkPin	ScanNetworksInputPin;
		OpenWire::SourcePin	FoundSignalStrengthOutputPin;
		OpenWire::SourcePin	FoundSSIDOutputPin;

		OpenWire::SourcePin	AddressOutputPin;
		OpenWire::SourcePin	MACOutputPin;

		OpenWire::SourcePin	SignalStrengthOutputPin;
		OpenWire::SourcePin	BSSIDOutputPin;
		OpenWire::SourcePin	GatewayIPOutputPin;
		OpenWire::SourcePin	SubnetMaskIPOutputPin;

		OpenWire::SinkPin	CheckSignalStrengthInputPin;

	public:
		Mitov::SimpleObjectList<LinkItRemoteAccessPoint*>	AccessPoints;
		Mitov::SimpleList<BasicEthernetSocket<LinkItWiFiModule>*>	Sockets;

		bool	Enabled = true;

	public:
		bool	IsStarted = false;

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
		virtual bool GetIPFromHostName( String AHostName, ::IPAddress &AAdress )
		{
			bool AResult = ( LWiFi.hostByName( AHostName.c_str(), AAdress ) == 1 );
			if( ! AResult )
				AAdress = INADDR_NONE;

			return AResult;
		}

	protected:
		virtual void SystemInit()
		{
			if( Enabled )
				StartEthernet();

			inherited::SystemInit();
		}

		void StopEthernet()
		{
			for( int i = 0; i < Sockets.size(); ++i )
				Sockets[ i ]->StopSocket();

			LWiFi.end();
			IsStarted = false;
		}

		void StartEthernet()
		{
/*
			if( ! AccessPoints.length() )
				return;

			if( ! Sockets.length() )
				return;
*/
			LWiFi.begin();
			for( int i = 0; i < AccessPoints.size(); ++i )
				if( AccessPoints[ i ]->Connect() )
				{
					IsStarted = true;
					if( MACOutputPin.IsConnected() )
					{
						uint8_t AMACAddress[VM_WLAN_WNDRV_MAC_ADDRESS_LEN] = {0};

						LWiFi.macAddress( AMACAddress );

						char AMACString[ VM_WLAN_WNDRV_MAC_ADDRESS_LEN * 3 + 1 ];
						sprintf( AMACString, "%02X-%02X-%02X-%02X-%02X-%02X", AMACString[ 0 ], AMACString[ 1 ], AMACString[ 2 ], AMACString[ 3 ], AMACString[ 4 ], AMACString[ 5 ] );
						MACOutputPin.Notify( AMACString );
					}

					if( AddressOutputPin.IsConnected() )
					{
						IPAddress ALocalIPAddress = LWiFi.localIP();
						char AIPString[ 4 * 4 + 1 ];
						sprintf( AIPString, "%u.%u.%u.%u", ALocalIPAddress[ 0 ], ALocalIPAddress[ 1 ], ALocalIPAddress[ 2 ], ALocalIPAddress[ 3 ] );

//						String ALocalIPAddressString = String( ALocalIPAddress[ 0 ] )  + "." + String( ALocalIPAddress[ 1 ]  + "." + ALocalIPAddress[ 2 ]  + "." + ALocalIPAddress[ 3 ];
						AddressOutputPin.Notify( AIPString );
					}

					if( SignalStrengthOutputPin.IsConnected() )
					{
						int32_t ASetrength = LWiFi.RSSI();
						SignalStrengthOutputPin.Notify( &ASetrength );
					}

					if( BSSIDOutputPin.IsConnected() )
					{
						uint8_t AMACAddress[VM_WLAN_WNDRV_MAC_ADDRESS_LEN] = {0};

						LWiFi.BSSID( AMACAddress );

						char AMACString[ VM_WLAN_WNDRV_MAC_ADDRESS_LEN * 3 + 1 ];
						sprintf( AMACString, "%02X-%02X-%02X-%02X-%02X-%02X", AMACString[ 0 ], AMACString[ 1 ], AMACString[ 2 ], AMACString[ 3 ], AMACString[ 4 ], AMACString[ 5 ] );
						BSSIDOutputPin.Notify( AMACString );
					}

					if( GatewayIPOutputPin.IsConnected() )
					{
						IPAddress ALocalIPAddress = LWiFi.gatewayIP();
						char AIPString[ 4 * 4 + 1 ];
						sprintf( AIPString, "%u.%u.%u.%u", ALocalIPAddress[ 0 ], ALocalIPAddress[ 1 ], ALocalIPAddress[ 2 ], ALocalIPAddress[ 3 ] );

//						String ALocalIPAddressString = String( ALocalIPAddress[ 0 ] )  + "." + String( ALocalIPAddress[ 1 ]  + "." + ALocalIPAddress[ 2 ]  + "." + ALocalIPAddress[ 3 ];
						AddressOutputPin.Notify( AIPString );
					}

					if( SubnetMaskIPOutputPin.IsConnected() )
					{
						IPAddress ALocalIPAddress = LWiFi.subnetMask();
						char AIPString[ 4 * 4 + 1 ];
						sprintf( AIPString, "%u.%u.%u.%u", ALocalIPAddress[ 0 ], ALocalIPAddress[ 1 ], ALocalIPAddress[ 2 ], ALocalIPAddress[ 3 ] );

//						String ALocalIPAddressString = String( ALocalIPAddress[ 0 ] )  + "." + String( ALocalIPAddress[ 1 ]  + "." + ALocalIPAddress[ 2 ]  + "." + ALocalIPAddress[ 3 ];
						SubnetMaskIPOutputPin.Notify( AIPString );
					}

					break;
				}

		}

	protected:
		void DoScanNetworks( void *_Data )
		{
			int nearbyAccessPointCount = LWiFi.scanNetworks();
			for( int i = 0; i < nearbyAccessPointCount; i ++ )
			{
				FoundSSIDOutputPin.Notify( LWiFi.SSID( i ));

				int32_t ASetrength = LWiFi.RSSI( i );
				FoundSignalStrengthOutputPin.Notify( &ASetrength );
			}
		}

		void DoCheckSignalStrength( void *_Data )
		{
			if( IsStarted )
				if( SignalStrengthOutputPin.IsConnected() )
				{
					int32_t ASetrength = LWiFi.RSSI();
					SignalStrengthOutputPin.Notify( &ASetrength );
				}
		}

	public:
		LinkItWiFiModule()
		{			
			ScanNetworksInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&LinkItWiFiModule::DoScanNetworks );
			CheckSignalStrengthInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&LinkItWiFiModule::DoCheckSignalStrength );
		}

	};
//---------------------------------------------------------------------------
	class LinkItWiFiTCPClientSocket : public TCPClientSocket<Mitov::LinkItWiFiModule,LWiFiClient>
	{
		typedef TCPClientSocket<Mitov::LinkItWiFiModule,LWiFiClient> inherited;

	public:
		virtual bool CanSend() override
		{
			return inherited::Enabled && inherited::FOwner.Enabled && FClient;
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class LinkItWiFiTCPServerSocket : public TCPServerSocket<Mitov::LinkItWiFiModule,LWiFiServer,LWiFiClient>
	{
		typedef TCPServerSocket<Mitov::LinkItWiFiModule,LWiFiServer,LWiFiClient> inherited;

	public:
		virtual bool CanSend() override
		{
			return inherited::Enabled && inherited::FOwner.Enabled && FClient;
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class LinkItWiFiUDPSocket : public UDPSocket<Mitov::LinkItWiFiModule,LWiFiUDP>
	{
		typedef UDPSocket<Mitov::LinkItWiFiModule,LWiFiUDP> inherited;

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
}

#endif

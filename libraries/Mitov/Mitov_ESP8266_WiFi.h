////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_ESP8266_WIFI_h
#define _MITOV_ESP8266_WIFI_h

#include <Mitov.h>
#include <Mitov_BasicEthernet.h>
#include <ESP8266WiFi.h>
//#include "ip_addr.h"
//#include "espconn.h"

namespace Mitov
{
	class ESP8266WiFiModule;
//---------------------------------------------------------------------------
	class ESP8266WiFiModuleOptionalDisabledElement
	{
	public:
		bool	Enabled = false;

	};
//---------------------------------------------------------------------------
	class ESP8266WiFiModuleOptionalChannel : public ESP8266WiFiModuleOptionalDisabledElement
	{
	public:
		unsigned long	Channel = 1;

	};
//---------------------------------------------------------------------------
	class ESP8266WiFiModuleOptionalIPConfig : public ESP8266WiFiModuleOptionalDisabledElement
	{
	public:
		::IPAddress	IP;

	};
//---------------------------------------------------------------------------
	class ESP8266WiFiModuleAccessPointConfig
	{
	public:
		bool		Enabled = false;

		::IPAddress	IP;
		::IPAddress	Gateway;
		::IPAddress	Subnet;

	};
//---------------------------------------------------------------------------
	class ESP8266WiFiModuleRemoteConfig : public ESP8266WiFiModuleAccessPointConfig
	{
	public:
		 ESP8266WiFiModuleOptionalIPConfig	DNS;
	};
//---------------------------------------------------------------------------
	class ESP8266WiFiModuleOptionalMacAddress : public ESP8266WiFiModuleOptionalDisabledElement
	{
	public:
		TMACAddress MacAddress;

	};
//---------------------------------------------------------------------------
	class ESP8266ModuleRemoteAccessPoint
	{
	public:
		bool	Enabled = true;
		String	SSID;
	    String	Password;
		ESP8266WiFiModuleOptionalChannel	Channel;
		ESP8266WiFiModuleOptionalMacAddress	MacAddress;
		ESP8266WiFiModuleRemoteConfig		Config;

//	protected:
//		ESP8266WiFiModule &FOwner;

	public:
		bool Connect()
		{
			if( ! Enabled )
				return false;

			if( SSID == "" )
				return false;

			const char *APassword;
			if( Password == "" )
				APassword = NULL;

			else
				APassword = Password.c_str();

			int32_t channel;

			if( Channel.Enabled )
				channel = Channel.Channel;

			else
				channel = 0;

			const uint8_t* bssid;

			if( MacAddress.Enabled )
				bssid = MacAddress.MacAddress.FMacAddress;

			else
				bssid = NULL;

//			Serial.println( SSID );
//			Serial.println( APassword );

			if( Config.Enabled )
			{
				if( Config.DNS.Enabled )
					WiFi.config( Config.IP, Config.Gateway, Config.Subnet, Config.DNS.IP );

				else
					WiFi.config( Config.IP, Config.Gateway, Config.Subnet);
			}

			WiFi.begin( (char *)SSID.c_str(), APassword, channel, bssid );

			for(;;)
			{
//				Serial.print( "." ) ;
				int ARes = WiFi.status();
				if( ARes == WL_CONNECTED )
					return true;

				if( ARes == WL_CONNECT_FAILED )
					return false;

				delay(500);
			}

/*
			while (WiFi.status() != WL_CONNECTED) {
			WL_CONNECT_FAILED

			return ( ARes == WL_CONNECTED );
*/
//			return ( WiFi.begin( (char *)SSID.c_str(), APassword, channel, bssid ) == WL_CONNECTED );


//			return ( WiFi.begin( (char *)SSID.c_str() ) == WL_CONNECTED );


/*
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
*/
		}

	protected:
		void RegisterIn( ESP8266WiFiModule &AOwner );

	public:
		ESP8266ModuleRemoteAccessPoint( ESP8266WiFiModule &AOwner, TMACAddress AMacAddress ) :
			ESP8266ModuleRemoteAccessPoint( AOwner )
		{
			MacAddress.MacAddress = AMacAddress;
		}

		ESP8266ModuleRemoteAccessPoint( ESP8266WiFiModule &AOwner )
		{
			RegisterIn( AOwner );
		}

	};
//---------------------------------------------------------------------------
	class ESP8266WiFiModuleAccessPoint
	{
	protected:
		ESP8266WiFiModule *FOwner;

	public:
		bool	Enabled = true;
		String	SSID;
		String	Password;

		unsigned long	Channel = 1;
		bool	IsHidden = false;

		ESP8266WiFiModuleAccessPointConfig	Config;

	public:
		void SetEnabled( bool AValue )
		{
			if( Enabled == AValue )
				return;

			Enabled = AValue;
			if( Enabled )
				TryStart();

			else
				WiFi.softAPdisconnect();

		}

	public:
		void TryStart();

	public:
		ESP8266WiFiModuleAccessPoint( ESP8266WiFiModule * AOwner ) :
			FOwner( AOwner )
		{
		}
	};
//---------------------------------------------------------------------------
	class ESP8266ModuleScanNetworksOperation : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		OpenWire::SinkPin	ScanInputPin;

		OpenWire::SourcePin	CountOutputPin;

		OpenWire::SourcePin	SignalStrengthOutputPin;
		OpenWire::SourcePin	SSIDOutputPin;
		OpenWire::SourcePin	ChannelOutputPin;
		OpenWire::SourcePin	EncryptionOutputPin;
		OpenWire::SourcePin	IsHiddenOutputPin;
		OpenWire::SourcePin	MACAddressOutputPin;

		OpenWire::TypedSourcePin<bool>	ScanningOutputPin;
		OpenWire::SourcePin	FailedOutputPin;
		OpenWire::SourcePin	FoundNetworkOutputPin;

	protected:
		bool	FScanRequest = false;

	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			inherited::SystemLoopBegin( currentMicros );
			ScanningOutputPin.SetValue( FScanRequest, true );
			if( FScanRequest )
			{
				int8_t AComplete = WiFi.scanComplete();
				if( AComplete == WIFI_SCAN_FAILED )
				{
					FailedOutputPin.Notify( NULL );
					CountOutputPin.SendValue( 0 );
					FScanRequest = false;
				}

				else if( AComplete >= 0 )
				{
					CountOutputPin.SendValue( (int)AComplete );
					for( int i = 0; i < AComplete; i ++ )
					{
						String ssid;
						uint8_t encryptionType;
						int32_t ASetrength;
						uint8_t* BSSID;
						int32_t channel;
						bool isHidden;
						if( WiFi.getNetworkInfo( i, ssid, encryptionType, ASetrength, BSSID, channel, isHidden ))
						{
							String BSSIDStr = WiFi.BSSIDstr( i );

							SSIDOutputPin.Notify( (void *)ssid.c_str() );
							SignalStrengthOutputPin.Notify( &ASetrength );
							EncryptionOutputPin.Notify( &encryptionType );
							MACAddressOutputPin.Notify( (void *)BSSIDStr.c_str() );
							ChannelOutputPin.Notify( &channel );
							IsHiddenOutputPin.Notify( &isHidden );
						}

						FoundNetworkOutputPin.Notify( NULL );
					}

					FScanRequest = false;
				}
			}
		}

	protected:
		void DoScanNetworks( void *_Data )
		{
			if( WiFi.scanComplete() != WIFI_SCAN_RUNNING )
			{
				WiFi.scanNetworks( true );
				FScanRequest = true;
			}
/*
			for( int i = 0; i < nearbyAccessPointCount; i ++ )
			{
				FoundSSIDOutputPin.Notify( LWiFi.SSID( i ));

				int32_t ASetrength = LWiFi.RSSI( i );
				FoundSignalStrengthOutputPin.Notify( &ASetrength );
			}
*/
		}

	public:
		ESP8266ModuleScanNetworksOperation()
		{			
			ScanInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ESP8266ModuleScanNetworksOperation::DoScanNetworks );
		}
	};
//---------------------------------------------------------------------------
	class ESP8266WiFiModule : public Mitov::BasicEthernetShield
	{
		typedef Mitov::BasicEthernetShield inherited;

	public:
		OpenWire::SourcePin	AddressOutputPin;
		OpenWire::SourcePin	MACOutputPin;

		OpenWire::SourcePin	BSSIDOutputPin;
		OpenWire::SourcePin	GatewayIPOutputPin;
		OpenWire::SourcePin	SubnetMaskIPOutputPin;

		OpenWire::TypedSourcePin<bool>	RemoteConnectedOutputPin;

	public:
		ESP8266WiFiModuleAccessPoint	AccessPoint;
		Mitov::SimpleObjectList<ESP8266ModuleRemoteAccessPoint*>	AccessPoints;

	public:
		bool	AutoReconnect = true;
		String	HostName;

	public:
		bool	IsStarted = false;

	public:
		virtual bool GetIPFromHostName( String AHostName, ::IPAddress &AAdress )
		{
			bool AResult = ( WiFi.hostByName( AHostName.c_str(), AAdress ) == 1 );
			if( ! AResult )
				AAdress = INADDR_NONE;

			return AResult;
		}

	protected:
		void StopEthernet()
		{
			inherited::StopEthernet();

			WiFi.disconnect( true );
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
//			Serial.println( "TRY CONNECT" );
			if( HostName != "" )
			{
				WiFi.hostname( HostName );
//				espconn_mdns_set_hostname( (char *) HostName.c_str() ); 
			}


			bool AConnected = false;
			for( int i = 0; i < AccessPoints.size(); ++i )
				if( AccessPoints[ i ]->Connect() )
				{
					AConnected = true;
//					Serial.println( "CONNECT" );

					if( AddressOutputPin.IsConnected() )
					{
						String IPAddress = IPAdressToString( WiFi.localIP());
						AddressOutputPin.Notify( (void *)IPAddress.c_str() );
					}

					IsStarted = true;
//						MACOutputPin;
					break;
				}

			if( ! AConnected )
				WiFi.begin();

			RemoteConnectedOutputPin.SetValue( AConnected, true );

			WiFi.setAutoReconnect( AutoReconnect );

			AccessPoint.TryStart();

			if( BSSIDOutputPin.IsConnected() )
				BSSIDOutputPin.SendValue( WiFi.BSSIDstr() );

			if( GatewayIPOutputPin.IsConnected() )
				GatewayIPOutputPin.SendValue( IPAdressToString( WiFi.gatewayIP() ));

			if( SubnetMaskIPOutputPin.IsConnected() )
				SubnetMaskIPOutputPin.SendValue( IPAdressToString( WiFi.subnetMask() ));


		}

	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			if( RemoteConnectedOutputPin.IsConnected() )
				RemoteConnectedOutputPin.SetValue( WiFi.isConnected(), true );

			inherited::SystemLoopBegin( currentMicros );
		}

/*
	void DoCheckSignalStrength( void *_Data )
		{
			if( IsStarted )
				if( SignalStrengthOutputPin.IsConnected() )
				{
					int32_t ASetrength = WiFi.RSSI();
					SignalStrengthOutputPin.Notify( &ASetrength );
				}

		}
*/
	public:
		ESP8266WiFiModule() :
			AccessPoint( this )
		{			
//			ScanNetworksInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ESP8266WiFiModule::DoScanNetworks );
//			CheckSignalStrengthInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&ESP8266WiFiModule::DoCheckSignalStrength );
		}

	};
//---------------------------------------------------------------------------
	class ESP8266ModuleClockedOperation : public Mitov::ClockingSupport
	{
	protected:
		ESP8266WiFiModule &FOwner;

	public:
		ESP8266ModuleClockedOperation( ESP8266WiFiModule &AOwner ) :
			FOwner( AOwner )
		{
		}
	};
//---------------------------------------------------------------------------
	class ESP8266ModuleSignalStrengthOperation : public ESP8266ModuleClockedOperation
	{
		typedef ESP8266ModuleClockedOperation	inherited;

	public:
		OpenWire::SourcePin	SignalStrengthOutputPin;

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			if( inherited::FOwner.IsStarted )
				if( SignalStrengthOutputPin.IsConnected() )
				{
					int32_t ASetrength = WiFi.RSSI();
					SignalStrengthOutputPin.Notify( &ASetrength );
				}

		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	class ESP8266ModuleReconnectOperation : public ESP8266ModuleClockedOperation
	{
		typedef ESP8266ModuleClockedOperation	inherited;

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			WiFi.reconnect();
		}

	public:
		using inherited::inherited;

	};
//---------------------------------------------------------------------------
	void ESP8266WiFiModuleAccessPoint::TryStart()
	{
		if( FOwner->Enabled )
			if( Enabled )
				if( SSID != "" )
				{
//Serial.println( "TEST1" );
//Serial.println( SSID );
//Serial.println( IsHidden );
					if( Config.Enabled )
						WiFi.softAPConfig( Config.IP, Config.Gateway, Config.Subnet );

					if( Password != "" )
						WiFi.softAP( SSID.c_str(), Password.c_str(), Channel, IsHidden );

					else
//						WiFi.softAP( SSID.c_str() );
						WiFi.softAP( SSID.c_str(), NULL, Channel, IsHidden );

					WiFi.softAPIP();
				}

	}
//---------------------------------------------------------------------------
	void ESP8266ModuleRemoteAccessPoint::RegisterIn( ESP8266WiFiModule &AOwner )
	{
		AOwner.AccessPoints.push_back( this );
	}
//---------------------------------------------------------------------------
}

#endif

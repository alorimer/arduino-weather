////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_ETHERNET_SHIELD_h
#define _MITOV_ETHERNET_SHIELD_h

#include <Mitov.h>
#include <Mitov_BasicEthernet.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class EthernetShield : public Mitov::BasicEthernetShield
	{
		typedef Mitov::BasicEthernetShield inherited;

	protected:
		TMACAddress FMacAddress;

	public:
		ShieldIPAddress	IPAddress;
		bool	UseDHCP;

	protected:
		virtual void StartEthernet()
		{
//			int AIndex = ((int)Parity) * 2 * 4 + ( StopBits - 1 ) + ( DataBits - 5);
//			T_SERIAL->begin( Speed );

//			Serial.println( "StartEthernet" );

			if( ! IPAddress.Enabled )
				Ethernet.begin( FMacAddress.FMacAddress );

			else
			{
				if( UseDHCP )
					if( Ethernet.begin( FMacAddress.FMacAddress ))
						return;

				if( ! IPAddress.DNS.Enabled )
				{
/*
					Serial.println( "StartEthernet IP" );
					Serial.print( FMacAddress.FMacAddress[ 0 ] );
					Serial.print( FMacAddress.FMacAddress[ 1 ] );
					Serial.print( FMacAddress.FMacAddress[ 2 ] );
					Serial.print( FMacAddress.FMacAddress[ 3 ] );
					Serial.println();
					IPAddress.IP.printTo( Serial );
					Serial.println();
*/
					Ethernet.begin( FMacAddress.FMacAddress, IPAddress.IP );
				}

				else
				{
					if( ! IPAddress.DNS.Gateway.Enabled )
						Ethernet.begin( FMacAddress.FMacAddress, IPAddress.IP, IPAddress.DNS.IP );

					else
					{
						if( ! IPAddress.DNS.Gateway.Subnet.Enabled )
							Ethernet.begin( FMacAddress.FMacAddress, IPAddress.IP, IPAddress.DNS.IP, IPAddress.DNS.Gateway.IP );

						else
							Ethernet.begin( FMacAddress.FMacAddress, IPAddress.IP, IPAddress.DNS.IP, IPAddress.DNS.Gateway.IP, IPAddress.DNS.Gateway.Subnet.IP );

					}
				}
			}
		}

	public:
		virtual bool GetIPFromHostName( String AHostName, ::IPAddress &AAdress )
		{
			DNSClient ADNSClient;

			ADNSClient.begin( Ethernet.dnsServerIP() );

			bool AResult = ( ADNSClient.getHostByName( AHostName.c_str(), AAdress ) == 1 );
			if( ! AResult )
				AAdress = INADDR_NONE;

			return AResult;
		}

	public:
		EthernetShield( TMACAddress AMacAddress ) :
			FMacAddress( AMacAddress ),
			UseDHCP( false )
		{
		}

		EthernetShield( TMACAddress AMacAddress, bool UseDHCP, ::IPAddress local_ip) :
			FMacAddress( AMacAddress ),
			UseDHCP( UseDHCP )
		{
			IPAddress.Enabled = true;
			IPAddress.IP = local_ip;
		}

		EthernetShield( TMACAddress AMacAddress, bool UseDHCP, ::IPAddress local_ip, ::IPAddress dns_server) :
			FMacAddress( AMacAddress ),
			UseDHCP( UseDHCP )
		{
			IPAddress.Enabled = ! UseDHCP;
			IPAddress.IP = local_ip;
			IPAddress.DNS.Enabled = true;
			IPAddress.DNS.IP = dns_server;
		}

		EthernetShield( TMACAddress AMacAddress, bool UseDHCP, ::IPAddress local_ip, ::IPAddress dns_server, ::IPAddress gateway) :
			FMacAddress( AMacAddress ),
			UseDHCP( UseDHCP )
		{
			IPAddress.Enabled = ! UseDHCP;
			IPAddress.IP = local_ip;
			IPAddress.DNS.Enabled = true;
			IPAddress.DNS.IP = dns_server;
			IPAddress.DNS.Gateway.Enabled = true;
			IPAddress.DNS.Gateway.IP = dns_server;
		}

		EthernetShield( TMACAddress AMacAddress, bool UseDHCP, ::IPAddress local_ip, ::IPAddress dns_server, ::IPAddress gateway, ::IPAddress subnet) :
			FMacAddress( AMacAddress ),
			UseDHCP( UseDHCP )
		{
			IPAddress.Enabled = ! UseDHCP;
			IPAddress.IP = local_ip;
			IPAddress.DNS.Enabled = true;
			IPAddress.DNS.IP = dns_server;
			IPAddress.DNS.Gateway.Enabled = true;
			IPAddress.DNS.Gateway.IP = dns_server;
			IPAddress.DNS.Gateway.Subnet.Enabled = true;
			IPAddress.DNS.Gateway.Subnet.IP = dns_server;
		}
	};
//---------------------------------------------------------------------------
}

#endif

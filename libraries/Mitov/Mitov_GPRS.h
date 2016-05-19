////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_GPRS_h
#define _MITOV_GPRS_h

#include <Mitov.h>
#include <Mitov_BasicEthernet.h>
#include <Mitov_GSMShield.h>
#include <GSM.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class GPRSModule : public Mitov::GSMModule
	{
		typedef Mitov::GSMModule inherited;

	public:
		Mitov::SimpleList<BasicEthernetSocket<GPRSModule>*>	Sockets;

	public:
		bool	Enabled = true;

		Mitov::SimpleObjectList<GPRSAccessPoint *>	AccessPoints;

	protected:
		GSMShield &FOwner;

	protected:
		GPRS	FGprs;
		bool	FConnected = false;

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

	protected:
		virtual void StartModule()
		{
			if( Enabled )
				StartEthernet();

			inherited::StartModule();
		}

		void StopEthernet()
		{
			for( int i = 0; i < Sockets.size(); ++i )
				Sockets[ i ]->StopSocket();

//			LWiFi.end();
			IsStarted = false;
		}

		void StartEthernet()
		{
			if( ! FOwner.IsStarted )
				return;
/*
			if( ! AccessPoints.length() )
				return;

			if( ! Sockets.length() )
				return;
*/
			TryConnect( true, 0 );
//			if( AccessPoint != "" )
//				IsStarted = FGprs.attachGPRS( (char *)AccessPoint.c_str(), (char *)UserName.c_str(), (char *)Password.c_str() );

//			else
//				AConnected = FGprs.attachGPRS();

/*
			if( AConnected )
			{
			}
*/
		}

		void TryConnect( bool FromStart, unsigned long currentMicros )
		{
			for( int i = 0; i < AccessPoints.size(); i ++ )
				if( AccessPoints[ i ]->Enabled )
				{
					if( ! FromStart )
						if( ! AccessPoints[ i ]->CanRetry( currentMicros ) )
							continue;

					FConnected = FGprs.attachGPRS( (char *)AccessPoints[ i ]->AccessPoint.c_str(), (char *)AccessPoints[ i ]->UserName.c_str(), (char *)AccessPoints[ i ]->Password.c_str() );
					if( FConnected )
						break;

					AccessPoints[ i ]->FLastTime = currentMicros;
					++ AccessPoints[ i ]->FRetryCount;
				}

		}

		virtual void SystemLoopBegin( unsigned long currentMicros )
		{
			if( ! FConnected )
				TryConnect( false, currentMicros );

			inherited::SystemLoopBegin( currentMicros );
		}

	public:
		GPRSModule( GSMShield &AOwner ) :
			FOwner( AOwner )
		{			
			AOwner.Modules.push_back( this );
		}

	};
//---------------------------------------------------------------------------
/*
	class LinkItWiFiTCPClientSocket : public TCPClientSocket<Mitov::LinkItWiFiModule,LWiFiClient>
	{
		typedef TCPClientSocket<Mitov::LinkItWiFiModule,LWiFiClient> inherited;

	public:
		virtual bool CanSend()
		{
			return inherited::Enabled && inherited::FOwner.Enabled && FClient;
		}

	public:
		LinkItWiFiTCPClientSocket( LinkItWiFiModule &AOwner, ::IPAddress AIPAddress ) :
			inherited( AOwner, AIPAddress )
		{
		}

	};
//---------------------------------------------------------------------------
	class LinkItWiFiTCPServerSocket : public TCPServerSocket<Mitov::LinkItWiFiModule,LWiFiServer,LWiFiClient>
	{
		typedef TCPServerSocket<Mitov::LinkItWiFiModule,LWiFiServer,LWiFiClient> inherited;

	public:
		virtual bool CanSend()
		{
			return inherited::Enabled && inherited::FOwner.Enabled && FClient;
		}

	public:
		LinkItWiFiTCPServerSocket( LinkItWiFiModule &AOwner ) :
			inherited( AOwner )
		{
		}
	};
//---------------------------------------------------------------------------
	class LinkItWiFiUDPSocket : public UDPSocket<Mitov::LinkItWiFiModule,LWiFiUDP>
	{
		typedef UDPSocket<Mitov::LinkItWiFiModule,LWiFiUDP> inherited;

	public:
		LinkItWiFiUDPSocket( LinkItWiFiModule &AOwner, ::IPAddress ARemoteIPAddress ) :
			inherited( AOwner, ARemoteIPAddress )
		{
		}
	};
//---------------------------------------------------------------------------
*/
}

#endif

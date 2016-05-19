////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_PS2_CONTROLLER_BASIC_h
#define _MITOV_PS2_CONTROLLER_BASIC_h

#include <Mitov.h>

namespace Mitov
{
	class PS2BasicController : public Mitov::EnabledComponent, public Mitov::ClockingSupport
	{
		typedef Mitov::EnabledComponent inherited;

	public:
		virtual	bool	ReadDigital( unsigned int AIndex ) = 0;
		virtual	float	ReadAnalog( unsigned int AIndex ) = 0;

	public:
		class BaseSensor : public OpenWire::SourcePin
		{
		protected:
			unsigned	int		FIndex;

		public:
			virtual	void	Process( PS2BasicController *AOwner ) = 0;
			virtual	void	StartProcess( PS2BasicController *AOwner ) = 0;

		public:
			BaseSensor( unsigned int AIndex ) :
				FIndex( AIndex )
			{
			}

		};

		class DigitalSensor : public BaseSensor
		{
			typedef BaseSensor inherited;

		protected:
			bool	FValue;

		public:
			virtual	void Process( PS2BasicController *AOwner ) override
			{
				bool AValue = AOwner->ReadDigital( FIndex );

				if( FValue == AValue )
					return;

				FValue = AValue;
				Notify( &FValue );
			}

			virtual	void StartProcess( PS2BasicController *AOwner ) override
			{
				bool AValue = AOwner->ReadDigital( FIndex );
				FValue = AValue;
				Notify( &FValue );
			}

		public:
			using inherited::inherited;

		};

		class AnalogSensor : public BaseSensor
		{
			typedef BaseSensor inherited;

		protected:
			float	FValue;

		public:
			virtual	void	Process( PS2BasicController *AOwner ) override
			{
				float	AValue = AOwner->ReadAnalog( FIndex );

				if( FValue == AValue )
					return;

				FValue = AValue;
				Notify( &FValue );				
			}

			virtual	void StartProcess( PS2BasicController *AOwner ) override
			{
				float	AValue = AOwner->ReadAnalog( FIndex );

				FValue = AValue;
				Notify( &FValue );				
			}

		public:
			using inherited::inherited;

		};

	public:
		Mitov::SimpleList<Mitov::PS2BasicController::BaseSensor *> Sensors;

	protected:
		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			if( ! ClockInputPin.IsConnected() )
				ReadData();

			inherited::SystemLoopBegin( currentMicros );
		}

		virtual void SystemStart() override
		{
			ReadController();
			for( int i = 0; i < Sensors.size(); ++i )
				Sensors[ i ]->StartProcess( this );

			inherited::SystemStart();
		}

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			ReadData();
		}

		void ReadData()
		{
			if( ! Enabled )
				return;

			ReadController();

			for( int i = 0; i < Sensors.size(); ++i )
				Sensors[ i ]->Process( this );
		}

		virtual void ReadController() = 0;

/*
		virtual ~PS2BasicController()
		{
			for( int i = 0; i < Sensors.size(); ++i )
				delete Sensors[ i ];
		}
*/
	};
//---------------------------------------------------------------------------
}

#endif

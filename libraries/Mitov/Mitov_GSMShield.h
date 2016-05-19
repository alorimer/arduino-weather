////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_GSM_SHIELD_h
#define _MITOV_GSM_SHIELD_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	class GSMModule : public OpenWire::Component
	{
	public:
		virtual void StartModule()
		{
		}

		virtual void StopModule()
		{
		}
	};
//---------------------------------------------------------------------------
	class GSMShield : public OpenWire::Component
	{
		typedef OpenWire::Component inherited;

	public:
		bool	Enabled = true;

		String	PIN;
		bool	Restart = true;

	public:
		bool	IsStarted = false;

	public:
		Mitov::SimpleList<GSMModule*>	Modules;

	protected:
		GSM	FGsm;

	public:
		void SetEnabled( bool AValue )
		{
            if( Enabled == AValue )
                return;

            Enabled = AValue;
			if( Enabled )
				StartShield();

			else
				StopShield();

		}

	protected:
		virtual void SystemInit()
		{
			if( Enabled )
				StartShield();

			inherited::SystemInit();
		}

		void StopShield()
		{
			for( int i = 0; i < Modules.size(); ++i )
				Modules[ i ]->StopModule();

			FGsm.shutdown();
			IsStarted = false;
		}

/*
        void RestartShield()
		{
			if( ! Enabled )
				return;

			StartShield();
		}
*/
		void StartShield()
		{
			if( PIN == "" )
			{
//				if( ! Restart )
					IsStarted = ( FGsm.begin( 0, Restart ) == GSM_READY );

//				else
//					IsStarted = ( FGsm.begin() == GSM_READY );
			}

			else
				IsStarted = ( FGsm.begin( (char *)PIN.c_str(), Restart ) == GSM_READY );

			for( int i = 0; i < Modules.size(); ++i )
				Modules[ i ]->StartModule();
		}

	};
//---------------------------------------------------------------------------
}

#endif

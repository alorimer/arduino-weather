////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_HEAT_INDEDX_h
#define _MITOV_HEAT_INDEDX_h

#include <Mitov.h>

namespace Mitov
{
	class HeatIndex : public Mitov::CommonEnableSource
	{
		typedef Mitov::CommonEnableSource inherited;

	public:
		OpenWire::SinkPin	TemperatureInputPin;
		OpenWire::SinkPin	HumidityInputPin;

	public:
		float	InitialTemperature = 0.0f;
		float	InitialHumidity = 0.0f;
		bool	InFahrenheit = false;

	protected:
		void DoTemperatureReceive( void *_Data )
		{
			float AValue = *(float*)_Data;
			if( InitialTemperature == AValue )
				return;

			InitialTemperature = AValue;
			CalculateOutput();
		}

		void DoHumidityReceive( void *_Data )
		{
			float AValue = *(float*)_Data;
			if( InitialHumidity == AValue )
				return;

			InitialHumidity = AValue;
			CalculateOutput();
		}

		void CalculateOutput()
		{
			// Using both Rothfusz and Steadman's equations
			// http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml

			float temperature = InitialTemperature;
			if ( ! InFahrenheit )
				temperature = ConvertCtoF(temperature);

			float hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (InitialHumidity * 0.094));

			if( hi > 79 )
			{
				hi = -42.379 +
							 2.04901523 * temperature +
							10.14333127 * InitialHumidity +
							-0.22475541 * temperature * InitialHumidity +
							-0.00683783 * pow(temperature, 2) +
							-0.05481717 * pow(InitialHumidity, 2) +
							 0.00122874 * pow(temperature, 2) * InitialHumidity +
							 0.00085282 * temperature * pow(InitialHumidity, 2) +
							-0.00000199 * pow(temperature, 2) * pow(InitialHumidity, 2);

				if((InitialHumidity < 13) && (temperature >= 80.0) && (temperature <= 112.0))
					hi -= ((13.0 - InitialHumidity) * 0.25) * sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

				else if((InitialHumidity > 85.0) && (temperature >= 80.0) && (temperature <= 87.0))
					hi += ((InitialHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
			}

			if( ! InFahrenheit )
				hi = ConvertFtoC( hi );

			inherited::OutputPin.Notify( &hi );
		}

	protected:
		virtual void SystemStart() 
		{
			inherited::SystemStart();
			CalculateOutput();
		}

	public:
		HeatIndex()
		{
			TemperatureInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&HeatIndex::DoTemperatureReceive );
			HumidityInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&HeatIndex::DoHumidityReceive );
		}

	};
}

#endif

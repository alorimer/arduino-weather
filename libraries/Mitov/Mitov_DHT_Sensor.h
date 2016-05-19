////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_DHT_SENSOR_h
#define _MITOV_DHT_SENSOR_h

#include <Mitov.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	template<int PIN_NUMBER> class BasicDHTSensor : public OpenWire::Component, public Mitov::ClockingSupport
	{
		typedef OpenWire::Component inherited;

		const uint32_t MIN_INTERVAL = 2000;

	public:
		OpenWire::SourcePin	TemperatureOutputPin;
		OpenWire::SourcePin	HumidityOutputPin;

	public:
		bool	InFahrenheit = false;

	protected:
		uint8_t data[5];
		uint32_t _lastreadtime = -MIN_INTERVAL;
		uint32_t _maxcycles;
		bool _lastresult = false;

#ifdef __AVR
		// Use direct GPIO access on an 8-bit AVR so keep track of the port and bitmask
		// for the digital pin connected to the DHT.  Other platforms will use digitalRead.
		uint8_t _bit, _port;
#endif

//		DHT	*FSensor;

	protected:
		virtual void SystemInit() override
		{
			pinMode( PIN_NUMBER, INPUT_PULLUP );
//			FSensor = new DHT( PIN_NUMBER, SENSOR_TYPE );
//			FSensor->begin();
			inherited::SystemInit();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			if( ! ClockInputPin.IsConnected() )
				ReadSensor();

			inherited::SystemLoopBegin( currentMicros );
		}

	protected:
		virtual void ReadSensor() = 0;

		uint32_t expectPulse(bool level) 
		{
			uint32_t count = 0;
			// On AVR platforms use direct GPIO port access as it's much faster and better
			// for catching pulses that are 10's of microseconds in length:
#ifdef __AVR
			uint8_t portState = level ? _bit : 0;
			while ((*portInputRegister(_port) & _bit) == portState) 
			// Otherwise fall back to using digitalRead (this seems to be necessary on ESP8266
			// right now, perhaps bugs in direct port access functions?).
#else
			while (digitalRead(PIN_NUMBER) == level) 
#endif
			{
				if (count++ >= _maxcycles) 
					return 0; // Exceeded timeout, fail.
			}

			return count;
		}

		bool TryRead()
		{
			uint32_t currenttime = millis();
			if ( (currenttime - _lastreadtime) < 2000 ) 
				return _lastresult; // return last correct measurement

			_lastreadtime = currenttime;

			// Reset 40 bits of received data to zero.
			data[0] = data[1] = data[2] = data[3] = data[4] = 0;

			// Send start signal.  See DHT datasheet for full signal diagram:
			//   http://www.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf

			// Go into high impedence state to let pull-up raise data line level and
			// start the reading process.
			digitalWrite( PIN_NUMBER, HIGH);
			delay(250);

			// First set data line low for 20 milliseconds.
			pinMode( PIN_NUMBER, OUTPUT);
			digitalWrite( PIN_NUMBER, LOW);
			delay(20);

			uint32_t cycles[80];
			{
				// Turn off interrupts temporarily because the next sections are timing critical
				// and we don't want any interruptions.
				InterruptLock lock;

				// End the start signal by setting data line high for 40 microseconds.
				digitalWrite( PIN_NUMBER, HIGH);
				delayMicroseconds(40);

				// Now start reading the data line to get the value from the DHT sensor.
				pinMode( PIN_NUMBER, INPUT_PULLUP );
				delayMicroseconds(10);  // Delay a bit to let sensor pull data line low.

				// First expect a low signal for ~80 microseconds followed by a high signal
				// for ~80 microseconds again.
				if (expectPulse(LOW) == 0) 
				{
//					DEBUG_PRINTLN(F("Timeout waiting for start signal low pulse."));
					_lastresult = false;
					return _lastresult;
				}
				if (expectPulse(HIGH) == 0) 
				{
//					DEBUG_PRINTLN(F("Timeout waiting for start signal high pulse."));
					_lastresult = false;
					return _lastresult;
				}

				// Now read the 40 bits sent by the sensor.  Each bit is sent as a 50
				// microsecond low pulse followed by a variable length high pulse.  If the
				// high pulse is ~28 microseconds then it's a 0 and if it's ~70 microseconds
				// then it's a 1.  We measure the cycle count of the initial 50us low pulse
				// and use that to compare to the cycle count of the high pulse to determine
				// if the bit is a 0 (high state cycle count < low state cycle count), or a
				// 1 (high state cycle count > low state cycle count). Note that for speed all
				// the pulses are read into a array and then examined in a later step.
				for (int i=0; i<80; i+=2) 
				{
					cycles[i]   = expectPulse(LOW);
					cycles[i+1] = expectPulse(HIGH);
				}

			} // Timing critical code is now complete.

			// Inspect pulses and determine which ones are 0 (high state cycle count < low
			// state cycle count), or 1 (high state cycle count > low state cycle count).
			for (int i=0; i<40; ++i) 
			{
				uint32_t lowCycles  = cycles[2*i];
				uint32_t highCycles = cycles[2*i+1];
				if ((lowCycles == 0) || (highCycles == 0)) 
				{
//					DEBUG_PRINTLN(F("Timeout waiting for pulse."));
					_lastresult = false;
					return _lastresult;
				}

				data[i/8] <<= 1;
				// Now compare the low and high cycle times to see if the bit is a 0 or 1.
				if (highCycles > lowCycles) 
					// High cycles are greater than 50us low cycle count, must be a 1.
					data[i/8] |= 1;

				// Else high cycles are less than (or equal to, a weird case) the 50us low
				// cycle count so this must be a zero.  Nothing needs to be changed in the
				// stored data.
			}

/*
  DEBUG_PRINTLN(F("Received:"));
  DEBUG_PRINT(data[0], HEX); DEBUG_PRINT(F(", "));
  DEBUG_PRINT(data[1], HEX); DEBUG_PRINT(F(", "));
  DEBUG_PRINT(data[2], HEX); DEBUG_PRINT(F(", "));
  DEBUG_PRINT(data[3], HEX); DEBUG_PRINT(F(", "));
  DEBUG_PRINT(data[4], HEX); DEBUG_PRINT(F(" =? "));
  DEBUG_PRINTLN((data[0] + data[1] + data[2] + data[3]) & 0xFF, HEX);
*/
			// Check we read 40 bits and that the checksum matches.
			if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) 
			{
				_lastresult = true;
				return _lastresult;
			}

			else 
			{
	//			DEBUG_PRINTLN(F("Checksum failure!"));
				_lastresult = false;
				return _lastresult;
			}
		}

	protected:
		void DoClockReceive( void *_Data ) override
		{
			ReadSensor();
		}

	public:
		BasicDHTSensor()
		{
  #ifdef __AVR
			_bit = digitalPinToBitMask( PIN_NUMBER );
			_port = digitalPinToPort( PIN_NUMBER );
  #endif
			_maxcycles = microsecondsToClockCycles(1000);  // 1 millisecond timeout for
                                                 // reading pulses from DHT sensor.
		}
	};
//---------------------------------------------------------------------------
	template<int PIN_NUMBER> class DHT11Sensor : public BasicDHTSensor<PIN_NUMBER>
	{
		typedef BasicDHTSensor<PIN_NUMBER> inherited;

	protected:
		virtual void ReadSensor() override
		{
			if( ! inherited::TryRead())
				return;

			if( inherited::TemperatureOutputPin.IsConnected() )
			{
				float   AValue = inherited::data[2];
				if( inherited::InFahrenheit )
					AValue = ConvertCtoF( AValue );

				inherited::TemperatureOutputPin.Notify( &AValue );
			}

			if( inherited::HumidityOutputPin.IsConnected() )
			{
				float   AValue = inherited::data[0];
				inherited::HumidityOutputPin.Notify( &AValue );
			}
		}
	};
//---------------------------------------------------------------------------
	template<int PIN_NUMBER> class DHT22Sensor : public BasicDHTSensor<PIN_NUMBER>
	{
		typedef BasicDHTSensor<PIN_NUMBER> inherited;

	protected:
		virtual void ReadSensor() override
		{
			if( ! inherited::TryRead())
				return;

			if( inherited::TemperatureOutputPin.IsConnected() )
			{
				float   AValue = inherited::data[2] & 0x7F;
				AValue *= 256;
				AValue += inherited::data[3];
				AValue *= 0.1;
				if( inherited::data[2] & 0x80 ) 
					AValue *= -1;
				
				if( inherited::InFahrenheit )
					AValue = ConvertCtoF( AValue );

				inherited::TemperatureOutputPin.Notify( &AValue );
			}

			if( inherited::HumidityOutputPin.IsConnected() )
			{
				float   AValue = inherited::data[0];
				AValue *= 256;
				AValue += inherited::data[1];
				AValue *= 0.1;
				inherited::HumidityOutputPin.Notify( &AValue );
			}
		}
	};
//---------------------------------------------------------------------------
}

#endif

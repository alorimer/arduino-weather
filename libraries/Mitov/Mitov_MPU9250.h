////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//     This software is supplied under the terms of a license agreement or    //
//     nondisclosure agreement with Mitov Software and may not be copied      //
//     or disclosed except in accordance with the terms of that agreement.    //
//         Copyright(c) 2002-2016 Mitov Software. All Rights Reserved.        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _MITOV_MPU9250_h
#define _MITOV_MPU9250_h

#include <Mitov.h>
#include <Wire.h> //I2C Arduino Library
#include <Mitov_Basic_I2C.h>
#include <Mitov_Compass_AK8963.h>

namespace Mitov
{
//---------------------------------------------------------------------------
	namespace MPU9250Const
	{
		const byte Adresses[ 2 ] = { 0x68, 0x68 };

		const byte MPU9150_RA_MAG_ADDRESS	= 0x0C;

		const byte MPU9150_RA_MAG_ST1		= 0x02;
		const byte MPU9150_RA_MAG_XOUT_L		= 0x03;
		const byte MPU9150_RA_MAG_XOUT_H		= 0x04;
		const byte MPU9150_RA_MAG_YOUT_L		= 0x05;
		const byte MPU9150_RA_MAG_YOUT_H		= 0x06;
		const byte MPU9150_RA_MAG_ZOUT_L		= 0x07;
		const byte MPU9150_RA_MAG_ZOUT_H		= 0x08;
		const byte MPU9150_RA_MAG_ST2		= 0x09;
		const byte MPU9150_RA_MAG_CNTL1		= 0x0A;
		const byte MPU9150_RA_MAG_CNTL2		= 0x0B;

		const byte MPU9150_RA_MAG_ASAX		= 0x10;
		const byte MPU9150_RA_MAG_ASAY		= 0x11;
		const byte MPU9150_RA_MAG_ASAZ		= 0x12;
	};
//---------------------------------------------------------------------------
	enum MPU9250ClockSource { mcsAutoSelect = 6, mcsInternal = 0, mcsGyroX = 1, mcsGyroY = 2, mcsGyroZ = 3, mcsExt32K = 4, mcsExt19M = 5, mcsReset = 7 };
//---------------------------------------------------------------------------
	class MPU9250BasicSensor : public OpenWire::Object
	{
	public:
		OpenWire::SourcePin	OutputPins[ 3 ];

	};
//---------------------------------------------------------------------------
	class MPU9250OptionalSensor
	{
	public:
		bool Enabled : 1;
		bool Queue : 1;
		bool SelfTest : 1; // Added to save space as bitfield

	public:
		MPU9250OptionalSensor() :
			Enabled( true ),
			Queue( false ),
			SelfTest( false )
		{
		}

	};
//---------------------------------------------------------------------------
	typedef MPU9250OptionalSensor MPU9250OptionalSelfTestSensor;
//---------------------------------------------------------------------------
	class MPU9250OptionalAxesSensor : public MPU9250BasicSensor
	{
	public:
		MPU9250OptionalSelfTestSensor	X;
		MPU9250OptionalSelfTestSensor	Y;
		MPU9250OptionalSelfTestSensor	Z;
	};
//---------------------------------------------------------------------------
	enum ArduinoMPU9250AccelerometerRange : uint8_t { ar2g, ar4g, ar8g, ar16g };
//---------------------------------------------------------------------------
	class MPU9250Accelerometer : public MPU9250OptionalAxesSensor
	{
	public:
		ArduinoMPU9250AccelerometerRange FullScaleRange = ar2g;

	};
//---------------------------------------------------------------------------
	enum TArduinoMPU9250GyroscopeRange : uint8_t { gr250dps, gr500dps, gr1000dps, gr2000dps };
//---------------------------------------------------------------------------
	class MPU9250Gyroscope : public MPU9250OptionalAxesSensor
	{
	public:
		TArduinoMPU9250GyroscopeRange FullScaleRange = gr250dps;

	};
//---------------------------------------------------------------------------
	class MPU9250Compass : public MPU9250BasicSensor
	{
	public:
		CompassAK8963Mode	Mode : 2;
		bool Enabled : 1;
		bool HighResolution : 1;

	public:
		OpenWire::SinkPin	ResetInputPin;
		OpenWire::SourcePin OverflowOutputPin;

	protected:
		void DoResetReceive( void *_Data )
		{
			I2C::WriteByte( MPU9250Const::MPU9150_RA_MAG_ADDRESS, MPU9250Const::MPU9150_RA_MAG_CNTL2, 0b00000001 );
		}

	public:
		MPU9250Compass() :
			Mode( cm100Hz ),
			Enabled( true ),
			HighResolution( true )
		{
			ResetInputPin.SetCallback( this, (OpenWire::TOnPinReceive)&MPU9250Compass::DoResetReceive );
		}
	};
//---------------------------------------------------------------------------
	class MPU9250Thermometer : public MPU9250OptionalSensor
	{
	public:
		OpenWire::SourcePin	OutputPin;
	};
//---------------------------------------------------------------------------
	enum MPU9250GyroscopeThermometerFilter
	{
		gtf_GB_8800Hz_GF_32KHz_TB_4000Hz,
		gtf_GB_3600Hz_GF_32KHz_TB_4000Hz,
		gtf_GB_250Hz_GF_8KHz_TB_4000Hz,
		gtf_GB_184Hz_GF_1KHz_TB_188Hz,
		gtf_GB_92Hz_GF_1KHz_TB_98Hz,
		gtf_GB_41Hz_GF_1KHz_TB_42Hz,
		gtf_GB_20Hz_GF_1KHz_TB_20Hz,
		gtf_GB_10Hz_GF_1KHz_TB_10Hz,
		gtf_GB_5Hz_GF_1KHz_TB_5Hz,
		gtf_GB_3600Hz_GF_8KHz_TB_4000Hz
	};
//---------------------------------------------------------------------------
	enum MPU9250FrameSynchronizationLocation { fslDisabled, fslThermometer, fslGyroscopeX, fslGyroscopeY, fslGyroscopeZ, fslAccelerometerX, fslAccelerometerY, fslAccelerometerZ };
//---------------------------------------------------------------------------
	class MPU9250FrameSynchronization
	{
	public:
		MPU9250FrameSynchronizationLocation Location : 3;
		bool	EnableInterrupt : 1;
		bool	InterruptOnLowLevel : 1;

	public:
		MPU9250FrameSynchronization() :
			Location( fslDisabled ),
			EnableInterrupt( false ),
			InterruptOnLowLevel( false )
		{
		}
	};
//---------------------------------------------------------------------------
	class MPU9250Interrupt
	{
	public:
		bool	Inverted : 1;
		bool	OpenDrain : 1;
		bool	Latch : 1;

	public:
		MPU9250Interrupt() :
			Inverted( false ),
			OpenDrain( false ),
			Latch( true )
		{
		}
	};
//---------------------------------------------------------------------------
	class MPU9250Queue
	{
	public:
		bool BlockOnFull = false;
	};
//---------------------------------------------------------------------------
	class MPU9250I2C : public Mitov::EnabledComponent, public Mitov::ClockingSupport
	{
		typedef	Mitov::EnabledComponent	inherited;

	protected:
		static const byte MPU9250_RA_SMPLRT_DIV     = 0x19;
		static const byte MPU9250_RA_CONFIG			= 0x1A;
		static const byte MPU9250_RA_GYRO_CONFIG	= 0x1B;
		static const byte MPU9250_RA_ACCEL_CONFIG	= 0x1C;
		static const byte MPU9250_RA_INT_PIN_CFG	= 0x37;
		static const byte MPU9250_RA_ACCEL_XOUT_H	= 0x3B;
		static const byte MPU9250_RA_ACCEL_XOUT_L	= 0x3C;
		static const byte MPU9250_RA_ACCEL_YOUT_H	= 0x3D;
		static const byte MPU9250_RA_ACCEL_YOUT_L	= 0x3E;
		static const byte MPU9250_RA_ACCEL_ZOUT_H	= 0x3F;
		static const byte MPU9250_RA_ACCEL_ZOUT_L	= 0x40;
		static const byte MPU9250_RA_PWR_MGMT_1		= 0x6B;
		static const byte MPU9250_RA_PWR_MGMT_2		= 0x6C;

	public:
		OpenWire::SinkPin	ResetInputPin;

	public:
		bool	Address : 1;
		bool	Standby : 1;
		MPU9250ClockSource	ClockSource : 3;
		MPU9250GyroscopeThermometerFilter GyroscopeThermometerFilter : 4;
		uint8_t	SampleRateDivider = 0;

	public:
		MPU9250Accelerometer		Accelerometer;
		MPU9250Gyroscope			Gyroscope;
		MPU9250Compass				Compass;
		MPU9250Thermometer			Thermometer;
		MPU9250FrameSynchronization	FrameSynchronization;
		MPU9250Queue				Queue;
		MPU9250Interrupt			Interrupt;

	protected:
		float CompassAdjustmentValues[ 3 ];

	protected:
		virtual void DoClockReceive( void *_Data ) override
		{
			ReadSensors();
		}

		void DoResetReceive( void *_Data )
		{
		}

	protected:
		void ReadSensors()
		{
//			Serial.println( "ReadSensors" );
			const float AccelerometerCoefficients [] = 
			{ 
				2.0f / 32768, 
				4.0f / 32768, 
				8.0f / 32768, 
				16.0f / 32768 
			};

			const float GyroscopeCoefficients [] = 
			{ 
				250.0f / 32768, 
				500.0f / 32768, 
				1000.0f / 32768, 
				2000.0f / 32768 
			};

			const float CompassCoefficients[] =
			{
				10.0f *4219.0f / 8190.0f,
				10.0f *4219.0f / 32760.0f
			};

			uint8_t AIntValues[ 7 * 2 ];

			if( ReadBytes( MPU9250_RA_ACCEL_XOUT_H, sizeof( AIntValues ), AIntValues ))
			{
				for( int i = 0; i < 3; ++i )
				{
					float AValue = (( ((int16_t)AIntValues[ i * 2 ] ) << 8 ) | AIntValues[ i * 2 + 1 ] ) * AccelerometerCoefficients[ Accelerometer.FullScaleRange ];
					Accelerometer.OutputPins[ i ].Notify( &AValue );
				}

				for( int i = 0; i < 3; ++i )
				{
					float AValue = (((int16_t)AIntValues[ ( i + 4 ) * 2 ] ) << 8 | AIntValues[ ( i + 4 ) * 2 + 1 ] ) * GyroscopeCoefficients[ Gyroscope.FullScaleRange & 0x11 ];
					Gyroscope.OutputPins[ i ].Notify( &AValue );
				}

				float AValue = ((((int16_t)AIntValues[ 3 * 2 ] ) << 8 ) | AIntValues[ 3 * 2 + 1 ] ) / 333.87 + 21.0;
				Thermometer.OutputPin.Notify( &AValue );
			}

			if( I2C::ReadBytes( MPU9250Const::MPU9150_RA_MAG_ADDRESS, MPU9250Const::MPU9150_RA_MAG_ST1, 1, AIntValues ))
			{
				Compass.OverflowOutputPin.SendValue( AIntValues[ 0 ] & 0b00000010 );
				if( AIntValues[ 0 ] & 0b00000001 )
					if( I2C::ReadBytes( MPU9250Const::MPU9150_RA_MAG_ADDRESS, MPU9250Const::MPU9150_RA_MAG_XOUT_L, 7, AIntValues ))
						for( int i = 0; i < 3; ++i )
						{
							float AValue = (( ((int16_t)AIntValues[ i * 2 + 1 ] ) << 8 ) | AIntValues[ i * 2 ] ) * CompassCoefficients[ Compass.HighResolution & 1 ] * CompassAdjustmentValues[ i ];
							Compass.OutputPins[ i ].Notify( &AValue );
						}
			}

		}

	protected:
		inline void WriteTo( byte ARegister, byte AData )
		{
			I2C::WriteByte( MPU9250Const::Adresses[ Address ], ARegister, AData );
		}

		bool ReadBytes( uint8_t regAddr, uint8_t length, void *data )
		{
			return I2C::ReadBytes( MPU9250Const::Adresses[ Address ], regAddr, length, data );
		}

	protected:
		void UpdatePowerManagementReg1()
		{
			uint8_t AValue;
			if( ClockSource = mcsAutoSelect )
			{
				if( Gyroscope.X.Enabled )
					AValue = mcsGyroX;

				else if( Gyroscope.Y.Enabled )
					AValue = mcsGyroY;

				else if( Gyroscope.Z.Enabled )
					AValue = mcsGyroZ;

				else
					AValue = mcsInternal;
			}

			else
				AValue = ClockSource;

			AValue |=	( Enabled				? 0 :	0b01000000 ) |
						( Standby				?		0b00010000 : 0 ) |
						( Thermometer.Enabled	? 0 :	0b00001000 );

			WriteTo( MPU9250_RA_PWR_MGMT_1, AValue );
		}

		void UpdatePowerManagementReg2()
		{
			uint8_t AValue =	( Accelerometer.X.Enabled	? 0 : 0b00100000 ) |
								( Accelerometer.Y.Enabled	? 0 : 0b00010000 ) |
								( Accelerometer.Z.Enabled	? 0 : 0b00001000 ) |
								( Gyroscope.X.Enabled		? 0 : 0b00000100 ) |
								( Gyroscope.Y.Enabled		? 0 : 0b00000010 ) |
								( Gyroscope.Z.Enabled		? 0 : 0b00000001 );

			WriteTo( MPU9250_RA_PWR_MGMT_2, AValue );
		}

		void UpdateConfigReg()
		{
			uint8_t AValue =	(( GyroscopeThermometerFilter - 2 ) & 0b111 ) |
								( Queue.BlockOnFull ? 0b01000000 : 0 ) |
								(( FrameSynchronization.Location & 0b111 ) << 4 );

			WriteTo( MPU9250_RA_CONFIG, AValue );
		}

		void UpdateGyroConfigReg()
		{
			uint8_t AValue;
			switch( GyroscopeThermometerFilter )
			{
				case gtf_GB_8800Hz_GF_32KHz_TB_4000Hz: AValue = 0b11; break;
				case gtf_GB_3600Hz_GF_32KHz_TB_4000Hz: AValue = 0b10; break;
				default : AValue = 0b00;
			}

			AValue |=	(( Gyroscope.FullScaleRange & 0b11 ) << 3 ) |
						( Gyroscope.X.SelfTest ? 0b10000000 : 0 ) |
						( Gyroscope.Y.SelfTest ? 0b01000000 : 0 ) |
						( Gyroscope.Z.SelfTest ? 0b00100000 : 0 );

			WriteTo( MPU9250_RA_GYRO_CONFIG, AValue );
		}

		void UpdateAccelerometerConfigReg()
		{
			byte AValue =	(( Accelerometer.FullScaleRange & 0b11 ) << 3 ) |
							( Accelerometer.X.SelfTest ? 0b10000000 : 0 ) |
							( Accelerometer.Y.SelfTest ? 0b01000000 : 0 ) |
							( Accelerometer.Z.SelfTest ? 0b00100000 : 0 );

			WriteTo( MPU9250_RA_ACCEL_CONFIG, AValue );
		}

		void UpdatenterruptPinAndBypassConfigReg( bool ADirectCompassAccess )
		{
			byte AValue =	( Interrupt.Inverted								?		0b10000000 : 0 ) |
							( Interrupt.OpenDrain								?		0b01000000 : 0 ) |
							( Interrupt.Latch									?		0b00100000 : 0 ) |
							( FrameSynchronization.InterruptOnLowLevel			?		0b00001000 : 0 ) |
							( FrameSynchronization.EnableInterrupt				?		0b00000100 : 0 ) |
							( Thermometer.Queue & ( ! ADirectCompassAccess )	? 0 :	0b00000010 );

//			Serial.print( "UpdatenterruptPinAndBypassConfigReg: " ); Serial.println( AValue, BIN );

			WriteTo( MPU9250_RA_INT_PIN_CFG, AValue );
		}

		void UpdateSampleRateDividerReg()
		{
			WriteTo( MPU9250_RA_SMPLRT_DIV, SampleRateDivider );
		}

		void UpdateCompassControlReg()
		{
			byte AValue;
			if( Compass.Enabled )
				AValue = CompassAK8963Const::CompassModes[ Compass.Mode ];

			else
				AValue = 0;

			AValue |=	( Compass.HighResolution	? 0b00010000 : 0 );

			I2C::WriteByte( MPU9250Const::MPU9150_RA_MAG_ADDRESS, MPU9250Const::MPU9150_RA_MAG_CNTL1, AValue );
		}

		void ReagCompassAdjustmentValues()
		{
			uint8_t AValues[ 3 ];

			I2C::ReadBytes( MPU9250Const::MPU9150_RA_MAG_ADDRESS, MPU9250Const::MPU9150_RA_MAG_ASAX, sizeof( AValues ), AValues );
			for( int i = 0; i < 3; ++i )
				CompassAdjustmentValues[ i ] = (((float) AValues[ i ] ) - 128.0f) / 256.0f + 1.0f;

		}

	protected:
		virtual void SystemStart() override
		{
			UpdatePowerManagementReg1();
			UpdatePowerManagementReg2();
			UpdateConfigReg();
			UpdateGyroConfigReg();
			UpdateAccelerometerConfigReg();
			UpdateSampleRateDividerReg();
			UpdatenterruptPinAndBypassConfigReg( true );
			UpdateCompassControlReg();
			ReagCompassAdjustmentValues();
			UpdatenterruptPinAndBypassConfigReg( false );

			inherited::SystemStart();
		}

		virtual void SystemLoopBegin( unsigned long currentMicros ) override
		{
			if( Enabled )
				if( ! ClockInputPin.IsConnected() )
					ReadSensors();

			inherited::SystemLoopBegin( currentMicros );
		}

	public:
		MPU9250I2C() :
			ClockSource( mcsAutoSelect ),
			GyroscopeThermometerFilter( gtf_GB_250Hz_GF_8KHz_TB_4000Hz ),
			Address( false ),
			Standby( false )
		{
			ResetInputPin.SetCallback( MAKE_CALLBACK( MPU9250I2C::DoResetReceive ));
		}
	};
//---------------------------------------------------------------------------
}

#endif

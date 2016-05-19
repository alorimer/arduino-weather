/*
* bmp180.h - Library for communicating with BMP180 sensor
* Created by Andrew Lorimer, original code by Leo Nutz (http://www.ALTDuino.de)
*/

#ifndef bmp180_h
#define bmp180_h

  #include "Arduino.h"
  #include <OneWire.h>      // Used for BMP180
  #include <Wire.h>         // Used for BMP180 (I2C communication)

  class bmp180 {
    public:
      bmp180(int pin);
      void readTemperature();
      void readPressure();

    private:
      int _pin;
      const unsigned char _OSS = 0;  // Oversampling Setting
      int _ac1; // Calibration values
      int _ac2;
      int _ac3;
      unsigned int _ac4;
      unsigned int _ac5;
      unsigned int _ac6;
      int _b1;
      int _b2;
      int _mb;
      int _mc;
      int _md;
      long _b5;
  }

#endif

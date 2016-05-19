#include "Arduino.h"
#include "bmp180.h"

bmp180::bmp180(int pin) {
  pinMode(pin, OUTPUT);
  _pin = pin;
  Wire.begin();
  calibrate();
}

//Calabration of Barometic sensor
void bmp180::calibrate() {
  _ac1 = readInt(0xAA);
  _ac2 = readInt(0xAC);
  _ac3 = readInt(0xAE);
  _ac4 = readInt(0xB0);
  _ac5 = readInt(0xB2);
  _ac6 = readInt(0xB4);
  _b1 = readInt(0xB6);
  _b2 = readInt(0xB8);
  _mb = readInt(0xBA);
  _mc = readInt(0xBC);
  _md = readInt(0xBE);
}



// Read 1 byte from BMP180
char bmp180::read(unsigned char address) {

  unsigned char data;

  Wire.beginTransmission(bmppin);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(bmppin, 1);
  while(!Wire.available());

  return Wire.read();
}

// Read 2 bytes from the BMP180. First byte from 'address', second from 'address'+1
int bmp180::readInt(unsigned char address) {

  unsigned char msb, lsb;

  Wire.beginTransmission(bmppin);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(bmppin, 2);
  while(Wire.available()<2)
    ;
  msb = Wire.read();
  lsb = Wire.read();

  return (int) msb<<8 | lsb;
}

// Read the compensated temperature value
unsigned int bmp180::readTemperature(){
  unsigned int ut;

  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(bmppin);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();

  // Wait at least 4.5ms
  delay(5);

  // Read two bytes from registers 0xF6 and 0xF7
  ut = readInt(0xF6);

  long x1, x2;

  x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
  x2 = ((long)mc << 11)/(x1 + md);
  b5 = x1 + x2;

  float temp = ((b5 + 8)>>4);
  temp = temp /10;

  return temp;
}






// Calculate pressure given up (Uncompensated Pressure)
// calibration values must be known
// b5 is also required so bmp180GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.

// Read the compensated pressure value
unsigned long bmp180::readPressure(){

  unsigned char msb, lsb, xlsb;
  unsigned long up = 0;

  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(bmppin);
  Wire.write(0xF4);
  Wire.write(0x34 + (OSS<<6));
  Wire.endTransmission();

  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));

  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  msb = read(0xF6);
  lsb = read(0xF7);
  xlsb = read(0xF8);

  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS); // Calculate uncompensated pressure


  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;

  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;

  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;

  b7 = ((unsigned long)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;

  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;

  long pressure = p;
  return pressure;
}

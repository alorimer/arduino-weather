/*
 *      Arduino Weather Station - Transmitter Unit
 *      Code by Andrew Lorimer (http://lorimer.id.au)
 *      Credit to @bram2202 (Instructables) and Leo Nutz (http://altduino.de)
 */


// Libraries
#include <Wire.h>         // Used for BMP180
#include <VirtualWire.h>  // Used for 433 MHz transmitter
#include "DHT.h"          // Adafruit DHTxx lib


// Sensor setup

DHT dht(2, DHT22);            // Set up DHT22 object on digital pin 2
#define bmppin 0x77           // I2C address of BMP180
const unsigned char OSS = 0;  // Oversampling Setting for BMP180

// Calibration values for BMP180
int ac1, ac2, ac3, b1, b2, mb, mc, md;
unsigned int ac4, ac5, ac6;
long b5; 

// Transmitter setup
#define INTEGER_MAX (powx(2,31)-1)
#define E_MAX (pow(10, 7))
#define E_MIN (pow(10, -6))
#define EPSILON 0.000000119209
int txpin = 12;     // Transmit pin of 433 MHz transmitter
int linenumber = 0; // Tracks how many lines of data sent (0-3)



void setup()
{
  Serial.begin(9600);
  Serial.println("Entering setup routine...");

  // DHT22 (humidity & temp)
  dht.begin();
  Serial.println("> Successfully started DHT22");
  
  //Pressure sensor
  Wire.begin();
  CalibratePressure();
  Serial.println("> Successfully started BMP180");

  // Setup transmitter
  pinMode('D12', OUTPUT);
  vw_set_tx_pin(txpin); 
  vw_setup(2000); //
  Serial.println("> Successfully started 433 MHz transmitter");

  Serial.println("Finished setup routine.\n");
}




void loop() {

  // Humidity (DHT22)
  float humidity = dht.readHumidity(); // Read humidity
  if (isnan(humidity)) {Serial.println("ERROR: Failed to read humidity from DHT22.");}
  SendData("#H"+ ((String)humidity)); // Send humidity data
  Serial.println("   Humidity: " + String(humidity) + " %     (sent)");

  // Temperature (BMP180)
  float temperature = GetTemp(); // Read temperature
  if (isnan(temperature)) {Serial.println("ERROR: Failed to read temperature from BMP180.");}
  SendData("#C"+(String(temperature,2))); // Send temperature data
  Serial.println("Temperature: " + String(temperature) + " " + char(176) + "C    (sent)");

  // Barometric pressure (BMP180)
  float pressure = GetPressure(); // Read pressure
  if (isnan(pressure)) {Serial.println("ERROR: Failed to read pressure from BMP180.");}
  SendData("#P"+(String(pressure/100,2)));
  Serial.println("   Pressure: " + String(temperature) + " hPa   (sent)");

  Serial.println();
  
  delay(5000); //Wait for next loop
  
}




void SendData(String data) { // Transmit data over 433 MHz  
  const char* rawdata = data.c_str(); // Convert input string to char array
  digitalWrite(13, true); // Flash onboard TX light
  vw_send((uint8_t *)rawdata, strlen(rawdata)); //Send Data
  vw_wait_tx(); // Wait until the whole message is gone
  digitalWrite(13, false); 
}




void CalibratePressure() {
  ac1 = BMP180ReadInt(0xAA);
  ac2 = BMP180ReadInt(0xAC);
  ac3 = BMP180ReadInt(0xAE);
  ac4 = BMP180ReadInt(0xB0);
  ac5 = BMP180ReadInt(0xB2);
  ac6 = BMP180ReadInt(0xB4);
  b1 = BMP180ReadInt(0xB6);
  b2 = BMP180ReadInt(0xB8);
  mb = BMP180ReadInt(0xBA);
  mc = BMP180ReadInt(0xBC);
  md = BMP180ReadInt(0xBE);
}




float GetTemp(){ // Calculate temperature (deg C) based on calibration data

  unsigned int ut;

  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(bmppin);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();

  delay(5); // Must be >4.5 ms
  
  ut = BMP180ReadInt(0xF6); // Read two bytes from registers 0xF6 and 0xF7
  
  long x1, x2;
  x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
  x2 = ((long)mc << 11)/(x1 + md);
  b5 = x1 + x2;
  float temp = ((b5 + 8)>>4);
  temp = temp /10;

  return temp;
}




long GetPressure(){ // Calculate pressure based on calibration data
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
  msb = BMP180Read(0xF6);
  lsb = BMP180Read(0xF7);
  xlsb = BMP180Read(0xF8);

  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);

  
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

  long temp = p;
  return temp;
}




char BMP180Read(unsigned char address) { // Read 1 byte
  unsigned char data;

  Wire.beginTransmission(bmppin);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(bmppin, 1);
  while(!Wire.available());

  return Wire.read();
}

int BMP180ReadInt(unsigned char address) { // Read 2 bytes - first from 'address', second from 'address' + 1
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

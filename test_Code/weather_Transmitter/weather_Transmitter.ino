#include <bmp180.h>

/*
 *      Arduino Weather Station
 *      Transmitter Unit
 *      
 *      Code by Andrew Lorimer, 2016
 */



// Libraries
#include <OneWire.h>      // Used for BMP180
#include <Wire.h>         // Used for BMP180 (I2C communication)
#include <VirtualWire.h>  // Used for 433 MHz transmitter
#include "DHT.h"          // Adafruit DHTxx lib

// Set up DHT22
#define dhtpin 2          // Digital connection to DHT22
DHT dht(dhtpin, DHT22);   // Create DHT22 object


// Set up BMP180
#define bmppin 0x77       // I2C address of BMP180
const unsigned char OSS = 0;  // Oversampling Setting
int ac1; // Calibration values
int ac2;
int ac3;
unsigned int ac4;
unsigned int ac5;
unsigned int ac6;
int b1;
int b2;
int mb;
int mc;
int md;
long b5; //

// Set up 433 MHz transmitter
#define INTEGER_MAX (pow(2,31)-1)
#define E_MAX (pow(10, 7))
#define E_MIN (pow(10, -6))
#define EPSILON 0.000000119209
OneWire oneWire(10);
int RF_TX_PIN = 12;





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

    // Setup transmit pin
  vw_set_tx_pin(RF_TX_PIN); 
  vw_setup(2000); //
  Serial.println("> Successfully started 433 MHz transmitter");

  //For Debug
  Serial.println("Finished setup routine.\n");
}







void loop() {

  // Humidity (DHT22)
  float humidity = dht.readHumidity(); // Read humidity
  if (isnan(humidity)) {Serial.println("ERROR: Failed to read humidity from DHT22.");}
  SendData("#H"+ ((String)humidity)); // Send humidity data

  // Temperature (BMP180)
  float temperature = readTemperature(); // Read temperature
  if (isnan(temperature)) {Serial.println("ERROR: Failed to read temperature from BMP180.");}
  SendData("#C"+(String(temperature,2))); // Send temperature data

  // Barometric pressure (BMP180)
  float pressure = readPressure(); // Read pressure
  if (isnan(pressure)) {Serial.println("ERROR: Failed to read pressure from BMP180.");}
  SendData("#P"+(String(pressure/100,2)));
  
  delay(5000); //Wait for next loop
  
}







void SendData(String data) {
  
  Serial.println("Sending " + data);
  
  const char* rawdata = data.c_str(); // Conver input string to char array
  digitalWrite(13, true); // Flash onboard TX light
  vw_send((uint8_t *)rawdata, strlen(rawdata)); //Send Data
  vw_wait_tx(); // Wait until the whole message is gone
  digitalWrite(13, false);
  
}



















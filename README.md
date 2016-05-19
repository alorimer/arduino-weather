# arduino-weather
> *Simple wireless weather station built on Arduino Nano boards.*

For my engineering class at school I built a simple wireless weather station using two Arduino boards. It currently measures temperature, humidity and pressure, and transmits this data over 433 MHz for displaying on an LCD display. For more details on this project, read the report I wrote for school over here - **************.

## Receiver
The receiver code uses two libraries:
- [VirtualWire](https://www.pjrc.com/teensy/td_libs_VirtualWire.html) is used for interfacing with the 433 MHz receiver. Essentially it differentiates between signal and noise and outputs a string. Note that VirtualWire is [no longer supported](http://www.airspayce.com/mikem/arduino/VirtualWire/) and the developer is recommending use of [RadioHead](http://www.airspayce.com/mikem/arduino/RadioHead/), however there are no real disadvantages to using VirtualWire, and the newer library RadioHead is much more complex to implement.
- [Visuino Liquid Crystal](https://www.visuino.com/wiki/index.php?title=Liquid_Crystal_Display_(LCD)_-_I2C) library is used to display information on the 20*4 char LCD display. It acts like a driver, interfacing with the breakout board of the LCD.

The code is heavily commented so it's not too hard to see what each bit does. However, here is a short description of how it works.
- `setup()` shows the splash screen on the LCD and initialises the libraries. Note that it is essential to set the pin mode - it will not work reliably without this. Also, be sure that the pin numbers are all correct if you are using this code for your own project.
- `loop()` turns on the LCD backlight if the PIR motion sensor is high, displays the max and min values if the buttons are pressed, and checks if data has been received. If data has been received `decode(char*)` is called...
- `decode(char*)` splits the data received into the three values. It then compares the values against the min and max values and replaces these if necessary. It then calls `displayCurrent()`...
- `displayCurrent()` then calls the Visuino LCD library which displays the characters in the correct locations.
- `displayMax()` and `displayMin()` work in the same way as `displayCurrent()`, and they are triggered by the two buttons on A2 and A3.

## Transmitter
The transmitter uses some different libraries for the sensors:
- [VirtualWire](https://www.pjrc.com/teensy/td_libs_VirtualWire.html) interfaces with the 433 MHz transmitter. It converts text to binary signals in a format the transmitter can read. Same library is used in the receiver, just a different part of it.
- [Wire](https://www.arduino.cc/en/Reference/Wire) allows communication with I2C devices (i.e. devices with SDA and SCL lines). This library is used to get data from the BMP180 pressure/temperature sensor which communicates over I2C. Note that the BMP180 only facilitates communication, not calibration or interpretation. These are done in the main sketch.
- [Adafruit DHTxx Library](https://learn.adafruit.com/dht) is used to read digital signals from the DHT22 humidity sensor (can also be used with DHT11).

Here is a short description of the code:
- `setup()` initialises the sensors and calibrates the BMP180 pressure/temperature sensor.
- `loop()` is run every 5 seconds. It reads data from the sensors, does a simple verification of the data, and then sends it off to `sendData(str)`...
- `sendData(str)` is very simple.. it just flashes a light to show it is transmitting, and then calls the VirtualWire library.
- The other methods (`CalibratePressure()`, `GetTemp()`, `GetPressure()`, `BMP180Read()` and `BMP180ReadInt()` are a bit more complicated to explain, but they're fairly easy to understand. Essentially they are involved in reading data from the sensors and then calculating a value in human-readable units.

## Credits
This project is an original idea but was inspired by [this Instructable tutorial](http://www.instructables.com/id/Arduino-weather-station/?ALLSTEPS).
Credits to developers of all the libraries - Mike McAuley (VirtualWire), Visuino (LCD Display library), Arduino community (Wire), Adafruit (DHTxx library).

Thanks also to my engineering teacher, Mr. Wayne Young, and to Alan Patterson for help on this project.

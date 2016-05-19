/*
 *      Arduino Weather Station - Receiver Unit
 *      Code by Andrew Lorimer (http://lorimer.id.au)
 *      Credit to @bram2202 (Instructables) and Mike McCauley (mikem@airspayce.com)
 */


// Libraries
#include <VirtualWire.h>    // Communicates with 433 MHz receiver
#include <LiquidCrystal.h>  // Displays text on LCD

// Objects and pins
LiquidCrystal lcd(7, 8, 5, 4, 3, 2); //LCD Pins
#define pir A0        // Pin for PIR input
#define backpin A1    // Pin for backlight output

// Data received ([0] = current;  [1] = min; [2] = max)
String TempC[3];     // Temp (deg C)
String Humidity[3];  // Humidity (%)
String Pressure[3];  // Pressure (Pa)






void setup() {
  
  Serial.begin(9600);
  Serial.println("Starting setup routine...");

  // Motion sensor
  pinMode(pir, INPUT);
  Serial.println("> Successfully started motion sensor");

  // LCD Display
  pinMode(backpin, OUTPUT);          // Set backlight pin
  lcd.begin(20, 4);                  // Set resolution

  // Show splash
  lcd.setCursor(0, 1);               // Set cursor at first line
  lcd.print("       arduino      "); // Print splash
  lcd.setCursor(0, 2);               // Set cursor at second line
  lcd.print("       weather      "); // Print splash
  lcd.display(); 
  Serial.println("> Successfully started LCD display");

  // Wireless receiver
  pinMode(11, INPUT);
  vw_set_rx_pin(11);
  vw_set_ptt_inverted(true);  // Required for DR3100
  vw_setup(2000);             // Bits per sec
  vw_rx_start();              // Start the receiver PLL running
  Serial.println("> Successfully started 433 MHz receiver");
  Serial.println("Finished setup routine.\n");

  // Buttons
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  delay(1500);
}




void loop() {
  
  analogWrite(backpin, analogRead(pir)); // If PIR is high, LCD backlight goes on. Duration & sensitivity adjusted on sensor.
  Serial.println(analogRead(pir));
  // Receive data
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  
  if (vw_get_message(buf, &buflen)) {
    digitalWrite(13, true);  // Flash onboard LED 
    char Chars[buflen]; 
    for (int i = 0; i < buflen; i++) {Chars[i]  = char(buf[i]);} // From buffer to char array
    Decode(Chars); // Send to deconder
    digitalWrite(13, false); // Turn off onboard LED
  }

  if (analogRead(A2) > 1020) {
    DisplayMin();
    delay(7000);
    DisplayCurrent();  
  }
  if (analogRead(A3) > 1020) {
    DisplayMax();
    delay(7000);
    DisplayCurrent();  
  }
}




void Decode(char* Raw) { // Filter wireless signals and put data into strings
  
  String Code = (String)Raw; // From char array to string
  
  if (Code.startsWith("#C")) {
    TempC[0] = Code.substring(2,7);     // Get chars 2-7
    if (TempC[1] == "") {TempC[1] = TempC[0]; TempC[2] = TempC[0];}
    if (TempC[0] < TempC[1]) {TempC[1] = TempC[0];}
    if (TempC[0] > TempC[2]) {TempC[2] = TempC[0];}
    Serial.println("Current Temp: " + TempC[0]);
    Serial.println("Min Temp: " + TempC[1]);
    Serial.println("Max Temp: " + TempC[2] + "\n");
  }
  
  if (Code.startsWith("#H")) {
    Humidity[0] = Code.substring(2,4);  // Get chars 2-4
    if (Humidity[1] == "") {Humidity[1] = Humidity[0]; Humidity[2] = Humidity[0];}
    if (Humidity[0] < Humidity[1]) {Humidity[1] = Humidity[0];}
    if (Humidity[0] > Humidity[2]) {Humidity[2] = Humidity[0];}
    Serial.println("Current Humidity: " + Humidity[0]);
    Serial.println("Min Humidity: " + Humidity[1]);
    Serial.println("Max Humidity: " + Humidity[2] + "\n");
  }
  
  if (Code.startsWith("#P")) {
    Pressure[0] =  Code.substring(2,8); // Get chars 2-8
    if (Pressure[1] == "") {Pressure[1] = Pressure[0]; Pressure[2] = Pressure[0];}
    if (Pressure[0] < Pressure[1]) {Pressure[1] = Pressure[0];}
    if (Pressure[0] > Pressure[2]) {Pressure[2] = Pressure[0];}
    Serial.println("Current Pressure: " + Pressure[0]);
    Serial.println("Min Pressure: " + Pressure[1]);
    Serial.println("Max Pressure: " + Pressure[2] + "\n");
  }

  DisplayCurrent();
  
}




void DisplayCurrent() { // Show text on screen
  
  lcd.clear();      // Clear LCD
  lcd.begin(20, 4); // Set resolution

  if (TempC[0] != "") {
    lcd.setCursor(0, 0);                        // Set cursor position to origin
    lcd.print(((String)" Temp. " + TempC[0] + " oC"));  // Print received data
  }

  if (Humidity[0] != "") {
    lcd.setCursor(0, 1);                           // Set cursor position to second line
    lcd.print(((String)"Humid. " + Humidity[0] +  " %"));  // Print received data
  }

  if (Pressure[0] != "") {
    lcd.setCursor(0, 2);                             // Set cursor position to third line
    lcd.print(((String)"Press. " + Pressure[0] +  " hPa"));  // Print received data
  }
  
  lcd.display();    // Display data in buffer

}




void DisplayMin() { // Show text on screen
  
  lcd.clear();      // Clear LCD
  lcd.begin(20, 4); // Set resolution

  if (TempC[1] != "") {
    lcd.setCursor(0, 0);                        // Set cursor position to origin
    lcd.print(((String)" Temp. " + TempC[1] + " oC"));  // Print received data
  }

  if (Humidity[1] != "") {
    lcd.setCursor(0, 1);                           // Set cursor position to second line
    lcd.print(((String)"Humid. " + Humidity[1] +  " %"));  // Print received data
  }

  if (Pressure[1] != "") {
    lcd.setCursor(0, 2);                             // Set cursor position to third line
    lcd.print(((String)"Press. " + Pressure[1] +  " hPa"));  // Print received data
  }
  
  lcd.display();    // Display data in buffer

}




void DisplayMax() { // Show text on screen
  
  lcd.clear();      // Clear LCD
  lcd.begin(20, 4); // Set resolution

  if (TempC[2] != "") {
    lcd.setCursor(0, 0);                        // Set cursor position to origin
    lcd.print(((String)" Temp. " + TempC[2] + " oC"));  // Print received data
  }

  if (Humidity[2] != "") {
    lcd.setCursor(0, 1);                           // Set cursor position to second line
    lcd.print(((String)"Humid. " + Humidity[2] +  " %"));  // Print received data
  }

  if (Pressure[2] != "") {
    lcd.setCursor(0, 2);                             // Set cursor position to third line
    lcd.print(((String)"Press. " + Pressure[2] +  " hPa"));  // Print received data
  }
  
  lcd.display();    // Display data in buffer

}

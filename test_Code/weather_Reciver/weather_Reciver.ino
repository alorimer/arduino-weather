// receiver.pde
//
// Simple example of how to use VirtualWire to receive messages
// Implements a simplex (one-way) receiver with an Rx-B1 module
//
// See VirtualWire.h for detailed API docs
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2008 Mike McCauley
// $Id: receiver.pde,v 1.3 2009/03/30 00:07:24 mikem Exp $
#include <VirtualWire.h>
#include <stdio.h>
#include <string.h>
#include <LiquidCrystal.h>

double Temp;

LiquidCrystal lcd(12, 10, 5, 4, 3, 2); //LCD Pins
int LCDpin =9;  //LCD backlight Pin
int Humid; // Value
#define pir A0 //Pin for the PIR input

//Varaible to store the recoverd values
String TempC; //Temp in C
String Vocht; //humidity
String Druk;  //pressure

void setup()
{
  Serial.begin(9600); // Debugging only
  Serial.println("setup");


  ////Setting pins for backlight
  pinMode(pir, INPUT);
  pinMode(LCDpin, OUTPUT);

  //LCD
  lcd.begin(20, 4); //Setting rows an colums

  lcd.setCursor(0, 0); //Set Cursor at begining
  lcd.print("####################"); //Drawing

  lcd.setCursor(0, 1); //Set cursor at next line
  lcd.print("#      Bram's      #"); //Drawim 

  lcd.setCursor(0, 2); //Set cursor at next line
  lcd.print("# Weather  station #"); //Drawim 

  lcd.setCursor(0, 3); //Set cursor at next line
  lcd.print("#######-V1.0-#######"); //Drawim 

  //Show on Screen, if this isnt called, screen would be empty
  lcd.display(); 

  //Set back color to full brightness
  digitalWrite(LCDpin,255);

  //Wait a seccond
  delay(1000);

  //For Receiving  Data
  // Initialise the IO and ISR
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000);      // Bits per sec
  vw_rx_start();       // Start the receiver PLL running
}



void loop()
{
  //IF PIR sensor if HIGH, LCD backlight is on
  //The Sensitivity  and duration is adjusted ON the PIR
  digitalWrite(LCDpin, digitalRead(pir));

  ////Receiving  DATA
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    int i;
    digitalWrite(13, true);  //onboard LED ON 
    char Chars[buflen]; 
    for (i = 0; i < buflen; i++)
    {
      Chars[i]  = char(buf[i]); //from buffer to Char Arry
    }

    Serial.println(Chars); //Debug only
    Decode(Chars); //Send to deconder
    digitalWrite(13, false); //onboard LED OFF 
  }
}

void Decode(char* Raw)
{

  String Code = (String)Raw; //From Char Array to string
  if (Code.startsWith("#C")) //Looks if Code starts With #C
  {
    TempC = Code.substring(2,7); //get 2 to 7 char and put in int TempC string
  }

  if (Code.startsWith("#H")) //Looks if Code starts With #H
  {
    Vocht = Code.substring(2,4);//get 2 to 4 char and put in int Vocht string
  }

  if (Code.startsWith("#P")) //Looks if Code starts With #P
  {
    Druk =  Code.substring(2,8);//get 2 to 8 char and put in int Druk string
  }

  SetScreen();  //Call Funtion
}

//Herbouw scherm
void SetScreen()
{

  lcd.clear(); //Cleas LCD
  lcd.begin(20, 4); //set screen res

  if (TempC != "")
  {
    lcd.setCursor(0, 0); //Set Cursos
    lcd.print(((String)"  Temp = " + TempC + " oC")); //Writes text with received value
  }

  if (Vocht != "")
  {
    lcd.setCursor(0, 1); //Set Cursos
    lcd.print(((String)"    Vocht = " + Vocht +  "%")); //Writes text with received value
  }

  if (Druk != "")
  {
    lcd.setCursor(0, 2); //Set Cursos
    lcd.print(((String)"  Druk = " + Druk +  " HPA"));//Writes text with received value
  }
  lcd.display();
}

































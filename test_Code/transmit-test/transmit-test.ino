#define rfTransmitPin 4  //RF Transmitter pin = digital pin 4

 void setup(){
   pinMode(rfTransmitPin, OUTPUT);     
   pinMode(13, OUTPUT);    
 }

 void loop(){
   for(int i=4000; i>5; i=i-(i/3)){
     digitalWrite(rfTransmitPin, HIGH);     //Transmit a HIGH signal
     Serial.println("HIGH");
     delay(2000);                           //Wait for 1 second
     
//     digitalWrite(rfTransmitPin,LOW);      //Transmit a LOW signal
//     Serial.println("low");
//     delay(i);                            //Variable delay
   }
 }

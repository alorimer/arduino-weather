#define rfReceivePin A0  //RF Receiver pin = Analog pin 0

 unsigned int data = 0;   // variable used to store received data
 const unsigned int upperThreshold = 50;  //upper threshold value
 const unsigned int lowerThreshold = 30;  //lower threshold value

 void setup(){
   pinMode(13, OUTPUT);
   Serial.begin(9600);
 }

 void loop(){
   data=analogRead(rfReceivePin);    //listen for data on Analog pin 0
   
    if(data>upperThreshold){
     Serial.println("low");
     Serial.println(data);
   }
   
   if(data<lowerThreshold){
     Serial.println("HIGH----------");
     Serial.println(data);
   }
 }


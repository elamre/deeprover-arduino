#include <Wire.h>  
#define SLAVE_ADDRESS 0x40   // Define the i2c address
#define trigPin 12
#define echoPin 11

 int  distance;
 
 void setup()
 {
 Serial.begin(9600);
 Wire.begin(SLAVE_ADDRESS); 
 Wire.onRequest(requestCallback);
 pinMode(trigPin, OUTPUT);
 pinMode(echoPin, INPUT);
}

void loop() 
{
long duration;
  
  digitalWrite(trigPin, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;

//Wire.onRequest(distance); 
Serial.println(distance);
 delay(300);
}

void requestCallback()
{
//  // Contrived example - transmit a value from an analogue pin.
//  int input = analogRead(AnalogueInputPin);
Wire.write(distance);
}

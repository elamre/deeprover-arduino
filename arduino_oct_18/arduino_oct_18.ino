#include <Wire.h>  
#define SLAVE_ADDRESS 0x40   // Define the i2c address
#define trigPin 12
#define echoPin 11

// [messageid][dataamount][n^data][cksum][cksum]

#define PROTOCOL_MAX_DATA 8
#define PROTOCOL_CHCKSUM 2
#define PROTOCOL_MSG_ID 1
#define PROTOCOL_MSG_LEN 1
#define PROTOCOL_MAX_SIZE PROTOCOL_MAX_DATA + PROTOCOL_CHCKSUM + PROTOCOL_MSG_ID + PROTOCOL_MSG_LEN

#define PROTOCOL_ID_RESTART 0
#define PROTOCOL_ID_DISTANCE 1
#define PROTOCOL_ID_LED 2
#define PROTOCOL_ID_MOTOR 3

#define MOTOR_LEFT_PIN 6
#define MOTOR_RIGHT_PIN 5

uint8_t sendLength = 0;
uint8_t protocolBuffer[PROTOCOL_MAX_SIZE];
uint8_t receiveBuffer[PROTOCOL_MAX_SIZE];

void requestCallback();
 int  distance;

 int currentSpeed = 255;
 
 void setup()
 {
 Serial.begin(9600);
 Wire.begin(SLAVE_ADDRESS); 
 Wire.onRequest(requestCallback);
 pinMode(trigPin, OUTPUT);
 pinMode(echoPin, INPUT);
 pinMode(MOTOR_LEFT_PIN, OUTPUT);
 pinMode(MOTOR_RIGHT_PIN, OUTPUT);
}

void turnLeft(){
  analogWrite(MOTOR_LEFT_PIN, currentSpeed/4);
  analogWrite(MOTOR_RIGHT_PIN, currentSpeed);
}

void straight(){
  analogWrite(MOTOR_LEFT_PIN, currentSpeed);
  analogWrite(MOTOR_RIGHT_PIN, currentSpeed);
}

void turnRight(){
  analogWrite(MOTOR_RIGHT_PIN, currentSpeed/4);
  analogWrite(MOTOR_LEFT_PIN, currentSpeed);
}

void stopDriving(){
    analogWrite(MOTOR_LEFT_PIN, 0);
  analogWrite(MOTOR_RIGHT_PIN, 0);
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
 delay(1000);
 testDrive();
}


int state = 0;
void testDrive(){
state = (state+1) % 4;
if(state == 0){
  straight();
}else if (state == 1){
  turnLeft();
} else if (state == 2){
  turnRight();
}else if (state == 3){
  stopDriving();
}
}

int receiveBufferIndex = 0;
int receivedLength = 0;


void requestCallback()
{
  while(Wire.available()>1 && receivedLength != 0){
    receiveBuffer[receiveBufferIndex] = Wire.read();
    if(receiveBufferIndex == 1){
      receivedLength = receiveBuffer[receiveBufferIndex];
    }
    receiveBufferIndex++;
  }
  if(receiveBufferIndex < receivedLength){
    delay(10);
  }
  receiveBufferIndex = 0;
  switch(receiveBufferIndex){
    case PROTOCOL_ID_RESTART:
    break;
    case PROTOCOL_ID_DISTANCE:
      // [messageid][dataamount][n^data][cksum][cksum]
      
    break;
    case PROTOCOL_ID_LED:
    break;
    case PROTOCOL_ID_MOTOR:
    break;
  }

  if(sendLength>0){
    Wire.write(protocolBuffer,sendLength);
  }
  
  // switch messageid
  //
//  // Contrived example - transmit a value from an analogue pin.
//  int input = analogRead(AnalogueInputPin);
//Wire.write(distance);
}

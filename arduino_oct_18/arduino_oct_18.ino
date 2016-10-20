#include <Wire.h>  
#define SLAVE_ADDRESS               0x40   // Define the i2c address
#define trigPin                     12
#define echoPin                     11

// [messageid][dataamount][n^data][cksum][cksum]

#define PROTOCOL_BYTE_ID            0
#define PROTOCOL_BYTE_LEN           PROTOCOL_MSG_ID
#define PROTOCOL_BYTE_DATA          PROTOCOL_MSG_ID + PROTOCOL_MSG_LEN

#define PROTOCOL_MSG_ID             1
#define PROTOCOL_MSG_LEN            1
#define PROTOCOL_MAX_DATA           8
#define PROTOCOL_CHCKSUM            2
#define PROTOCOL_MAX_SIZE           PROTOCOL_MAX_DATA + PROTOCOL_CHCKSUM + PROTOCOL_MSG_ID + PROTOCOL_MSG_LEN

#define PROTOCOL_ID_RESTART         0
#define PROTOCOL_ID_DISTANCE        1
#define PROTOCOL_ID_LED             2
#define PROTOCOL_ID_MOTOR           3

#define PROTOCOL_CALCULATE_LEN(data) data + PROTOCOL_CHCKSUM + PROTOCOL_MSG_ID + PROTOCOL_MSG_LEN;

#define PROTOCOL_MOTOR_LEFT_BIT     0x01
#define PROTOCOL_MOTOR_RIGHT_BIT    0x02
#define PROTOCOL_MOTOR_STRAIGHT_BIT 0x03
#define PROTOCOL_MOTOR_STOP_BIT     0x04

#define MOTOR_LEFT_PIN              6
#define MOTOR_RIGHT_PIN             5

uint8_t sendLength = 0;
uint8_t sendBuffer[PROTOCOL_MAX_SIZE];
uint8_t receiveBuffer[PROTOCOL_MAX_SIZE];

void requestCallback();
int  distance;

int currentSpeed = 255;

 void bufferWriteInt(uint8_t * buf, int value){
     buf[0] = (value & 0xFF000000)>>24;
     buf[1] = (value & 0x00FF0000)>>16;
     buf[2] = (value & 0x0000FF00)>>8;
     buf[3] = (value & 0x000000FF);
 }

 void calculateChecksum(uint8_t * protoBuffer, uint8_t dataAmount){
    uint8_t i = 0;
    uint16_t checksum = 0;
    for(;i<dataAmount;i++){
        checksum += protoBuffer[i + PROTOCOL_MSG_ID + PROTOCOL_MSG_LEN];
    }
    protoBuffer[PROTOCOL_MSG_ID + PROTOCOL_MSG_LEN+dataAmount] = (checksum& 0xFF00)>>8;
    protoBuffer[PROTOCOL_MSG_ID + PROTOCOL_MSG_LEN+dataAmount + 1] = (checksum& 0x00FF);
 }
 
 void setup() {
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

void loop(){
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


void requestCallback() {
  while(Wire.available()>1 && receivedLength != 0){
    receiveBuffer[receiveBufferIndex] = Wire.read();
    if(receiveBufferIndex == PROTOCOL_BYTE_LEN){
      receivedLength = receiveBuffer[receiveBufferIndex];
    }
    receiveBufferIndex++;
  }
  if(receiveBufferIndex < receivedLength){
    delay(10);
    //TODO check again, throw a timeout
  }
  receiveBufferIndex = 0;
  
  switch(receiveBuffer[PROTOCOL_BYTE_ID]){
    case PROTOCOL_ID_RESTART:
    break;
    case PROTOCOL_ID_DISTANCE:
      // [1][0][cksum][cksum]                         -- raspberry
      // [1][4][d][d][d][d][cksum][cksum]             -- arduino
      sendBuffer[PROTOCOL_BYTE_ID] = PROTOCOL_ID_DISTANCE;
      sendLength = sendBuffer[PROTOCOL_BYTE_LEN] = PROTOCOL_CALCULATE_LEN(4);
      bufferWriteInt(&sendBuffer[PROTOCOL_MSG_ID + PROTOCOL_MSG_LEN],distance);
      calculateChecksum(sendBuffer,4);
    break;
    case PROTOCOL_ID_LED:
    
    break;
    case PROTOCOL_ID_MOTOR:
      // [1][2][dir][speed][cksum][cksum]             -- raspberry
      uint8_t control = receiveBuffer[PROTOCOL_BYTE_DATA];
      uint8_t newSpeed = receiveBuffer[PROTOCOL_BYTE_DATA + 1];
      currentSpeed = newSpeed;
      if(control & PROTOCOL_MOTOR_LEFT_BIT){
        analogWrite(MOTOR_LEFT_PIN,   currentSpeed);
      }
      if(control & PROTOCOL_MOTOR_RIGHT_BIT){
        analogWrite(MOTOR_RIGHT_PIN,  currentSpeed);
      }
      if(control & PROTOCOL_MOTOR_STRAIGHT_BIT){
        analogWrite(MOTOR_LEFT_PIN,   currentSpeed);
        analogWrite(MOTOR_RIGHT_PIN,  currentSpeed);
      }
      if(control & PROTOCOL_MOTOR_STOP_BIT){
        analogWrite(MOTOR_LEFT_PIN,   0);
        analogWrite(MOTOR_RIGHT_PIN,  0);
      }
    break;
  }

  if(sendLength>0){
    Wire.write(sendBuffer,sendLength);
    sendLength = 0;
  }
}

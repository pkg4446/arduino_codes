#include <Arduino.h>
#include <Wire.h>
#include "PCA9555.h"

#define UPDATE_INTERVAL 1000L
unsigned long prevUpdateTime = 0L;

#define COMMAND_LENGTH  32
#define TOTAL_PINOUT    6
#define TOTAL_RELAY     16
PCA9555 ioport(0x20);

const uint8_t pinout[TOTAL_PINOUT] = {12,27,26,15,2,4};

char    command_buf[COMMAND_LENGTH];
uint8_t  command_num;

struct stepmoter {
  uint8_t stepPin;
  uint8_t enablePin;
  uint8_t dirPin;
};

stepmoter moter[TOTAL_PINOUT] = {
  {12,0,1},
  {27,2,3},
  {26,4,5},
  {15,10,11},
  {2,12,13},
  {4,14,15}
};

////--------------------- String_slice ----------------////
String String_slice(uint8_t *check_index, String text, char check_char){
  String response = "";
  for(uint8_t index_check=*check_index; index_check<text.length(); index_check++){
    if(text[index_check] == check_char || text[index_check] == 0x00){
      *check_index = index_check+1;
      break;
    }
    response += text[index_check];
  }
  return response;
}
////--------------------- String_slice ----------------////
void serial_command_help(){
  Serial.println("Available Commands:");
  Serial.println("reboot");
  Serial.println("run <motor_num> <direction> <delay> <pulse>");
  Serial.println("pul <motor_num1> <motor_num2> <delay> <pulse>");
}

void command_service(){
  uint8_t check_index = 0;
  String cmd_text  = String_slice(&check_index, command_buf, 0x20);
  ////cmd start
  if(cmd_text=="reboot"){
    ESP.restart();
  }else if(cmd_text=="run"){
    uint8_t moternum = String_slice(&check_index, command_buf, 0x20).toInt();
    bool direction = String_slice(&check_index, command_buf, 0x20).toInt()==0?false:true;
    uint16_t delay = String_slice(&check_index, command_buf, 0x20).toInt();
    uint16_t pulse = String_slice(&check_index, command_buf, 0x20).toInt();
    Serial.print("Motor: ");
    Serial.print(moternum);
    Serial.print(", Direction: ");
    Serial.print(direction);
    Serial.print(", Delay: ");
    Serial.print(delay);
    Serial.print(", Pulse: ");
    Serial.println(pulse);
    stepperMotorControl(moter[moternum], direction, delay, pulse);
  }else if(cmd_text=="pul"){
    uint8_t moternum1 = String_slice(&check_index, command_buf, 0x20).toInt();
    uint8_t moternum2 = String_slice(&check_index, command_buf, 0x20).toInt();
    bool direction = String_slice(&check_index, command_buf, 0x20).toInt()==0?false:true;
    uint16_t delay = String_slice(&check_index, command_buf, 0x20).toInt();
    uint16_t pulse = String_slice(&check_index, command_buf, 0x20).toInt();
    Serial.print("Motor: ");
    Serial.print(moternum1);
    Serial.print(", Motor2: ");
    Serial.print(moternum2);
    Serial.print(", Direction: ");
    Serial.print(direction);
    Serial.print(", Delay: ");
    Serial.print(delay);
    Serial.print(", Pulse: ");
    Serial.println(pulse);
    control_pair(moter[moternum1], moter[moternum2], direction, delay, pulse);
  }else{
    serial_command_help();
  }
  Serial.println(".");
}

void command_process(char ch) {
  if(ch=='\n'){
    command_buf[command_num] = 0x00;
    command_num = 0;
    command_service();
    memset(command_buf, 0x00, COMMAND_LENGTH);
  }else if(ch!='\r'){
    command_buf[command_num++] = ch;
    command_num %= COMMAND_LENGTH;
  }
}

void setup() {
  Serial.begin(115200);
  ioport.begin();
  ioport.setClock(400000);
  // initialize digital pin LED_BUILTIN as an output.

  for(uint8_t index=0; index<TOTAL_PINOUT; index++){
    pinMode(moter[index].stepPin, OUTPUT);
    ioport.pinMode(moter[index].dirPin, OUTPUT);
    ioport.pinMode(moter[index].enablePin, OUTPUT);
  }
  for(uint8_t index=0; index<TOTAL_PINOUT; index++){
    digitalWrite(moter[index].stepPin, false);
    ioport.digitalWrite(moter[index].dirPin, false);
    ioport.digitalWrite(moter[index].enablePin, true);
  }
  Serial.println("System Ready.");
}

// the loop function runs over and over again forever
void loop() {
  if(Serial.available()){
    command_process(Serial.read());
  }
}

void stepperMotorControl(stepmoter motor, bool direction, uint16_t stepDelay, uint16_t steps) {
  ioport.digitalWrite(motor.enablePin, false); // Enable the motor driver
  ioport.digitalWrite(motor.dirPin, direction); // Set direction
  delayMicroseconds(10);

  for (uint16_t i = 0; i < steps; i++) {
    digitalWrite(motor.stepPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(motor.stepPin, LOW);
    delayMicroseconds(stepDelay);
  }

  ioport.digitalWrite(motor.enablePin, true); // Disable the motor driver
}

void control_pair(stepmoter motor1, stepmoter motor2, bool direction, uint16_t stepDelay, uint16_t steps) {
  ioport.digitalWrite(motor1.enablePin, false); // Enable the motor driver
  ioport.digitalWrite(motor2.enablePin, false); // Enable the motor driver
  ioport.digitalWrite(motor1.dirPin, direction); // Set direction
  ioport.digitalWrite(motor2.dirPin, !direction); // Set direction
  delayMicroseconds(10);

  for (uint16_t i = 0; i < steps; i++) {
    digitalWrite(motor1.stepPin, HIGH);
    digitalWrite(motor2.stepPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(motor1.stepPin, LOW);
    digitalWrite(motor2.stepPin, LOW);
    delayMicroseconds(stepDelay);
  }

  ioport.digitalWrite(motor1.enablePin, true); // Disable the motor driver
  ioport.digitalWrite(motor2.enablePin, true); // Disable the motor driver
}
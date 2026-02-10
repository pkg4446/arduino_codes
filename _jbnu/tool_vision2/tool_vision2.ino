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
  Serial.println("run <motor_num> <delay> <pulse>");
  Serial.println("pin <pin_num> <state>");
}

void command_service(){
  uint8_t check_index = 0;
  String cmd_text  = String_slice(&check_index, command_buf, 0x20);
  ////cmd start
  if(cmd_text=="reboot"){
    ESP.restart();
  }else if(cmd_text=="run"){
    uint8_t moternum = String_slice(&check_index, command_buf, 0x20).toInt();
    uint16_t delay = String_slice(&check_index, command_buf, 0x20).toInt();
    uint16_t pulse = String_slice(&check_index, command_buf, 0x20).toInt();
    pinOnOff(pinout[moternum]);
    Serial.print("motor ");
    Serial.print(moternum);
    Serial.print(" pin: ");
    Serial.println(pinout[moternum]);
    control_run(pinout[moternum], delay, pulse);
  }else if(cmd_text=="pin"){
    uint8_t pin = String_slice(&check_index, command_buf, 0x20).toInt();
    bool state = String_slice(&check_index, command_buf, 0x20).toInt()==0?false:true;
    Serial.print("pin ");
    Serial.print(pin);
    Serial.print(" state: ");
    Serial.println(state);
    ioport_con(pin, state);
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
    pinMode(pinout[index], OUTPUT);
  }
  for (uint8_t index = 0; index < TOTAL_RELAY; index++)
  {
    ioport.pinMode(index, OUTPUT);
  }

  for (uint8_t index = 0; index < TOTAL_PINOUT; index++)
  {
    pinOnOff(pinout[index]);
  }
  for (uint8_t index = 0; index < TOTAL_RELAY; index++)
  {
    relayOnOff(index);
  }

  Serial.println("System Ready.");
}

// the loop function runs over and over again forever
void loop() {
  if(Serial.available()){
    command_process(Serial.read());
  }
}

void pinOnOff(int8_t pinNumber) {
  Serial.print("pin number ");
  Serial.println(pinNumber);
  digitalWrite(pinNumber, true);
  delay(100);
  digitalWrite(pinNumber, false);
}

void relayOnOff(int8_t pinNumber) {
  Serial.print("pin number ");
  Serial.println(pinNumber);
  ioport.digitalWrite(pinNumber, true);
  delay(100);
  ioport.digitalWrite(pinNumber, false);
}

void ioport_con(uint8_t pin, bool state) {
  ioport.digitalWrite(pin, state);
}

void control_run(uint16_t motor, uint16_t stepDelay, uint16_t steps) {
  for (uint16_t i = 0; i < steps; i++) {
    digitalWrite(motor, HIGH);
    delayMicroseconds(10);
    digitalWrite(motor, LOW);
    delayMicroseconds(stepDelay);
  }
}
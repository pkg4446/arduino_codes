#include "HardwareSerial.h"
#include "control.h"

MOTOR::MOTOR(){}
MOTOR::~MOTOR(){}

void MOTOR::begin(uint8_t num){
  Serial.print(num);
}
//#include "pin_setup"
#include "control.h"
#include "pin_setup.h"

MOTOR::MOTOR(STEP_ts moter_pins){
    step_moter = moter_pins;
    Position   = 0;
    Zero       = false;
}
MOTOR::~MOTOR(){}

void MOTOR::init(){
  Serial.print(step_moter.DIR);
  Serial.print(step_moter.PWM);
}
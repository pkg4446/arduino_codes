#include "arduino.h"

#include "moter_control.h"
#include "pin_setup.h"
#include "shift_regs.h"

MOTOR::MOTOR(){
  Position   = 0;
  Zero_set   = false;
  accel      = 0;
  decel      = 0;
  dla_s      = 10;
  dla_l      = 10;
}
MOTOR::~MOTOR(){}

void MOTOR::init(STEP_ts moter_pins){
  step_moter = moter_pins;
  pinMode(step_moter.DIR, OUTPUT);
  pinMode(step_moter.PWM, OUTPUT);
  digitalWrite(step_moter.DIR, false);
  digitalWrite(step_moter.PWM, false);
}

void MOTOR::set_config(uint16_t v_accel, uint16_t v_decel, uint16_t v_dla_s, uint16_t v_dla_l){
  accel = v_accel;
  decel = v_decel;
  dla_s = v_dla_s;
  dla_l = v_dla_l;
}

void MOTOR::status(){
  Serial.print("DIR: ");
  Serial.print(step_moter.DIR);
  Serial.print(" , PWM: ");
  Serial.print(step_moter.PWM);
  Serial.print(", Position: ");
  Serial.print(Position);
  Serial.print(", zero set: ");
  Serial.print(Zero_set);
  Serial.print(", accel_step: ");
  Serial.print(accel);
  Serial.print(", decel_step: ");
  Serial.print(decel);
  Serial.print(", delay_short: ");
  Serial.print(dla_s);
  Serial.print(", delay_long: ");
  Serial.println(dla_l);
}

void MOTOR::pos_update(bool direction){
  if(direction) Position++;
  else Position--;
}

uint32_t MOTOR::get_pos(){return Position;}
bool     MOTOR::get_zero_set(){return Zero_set;}
uint16_t MOTOR::accel_step(){return accel;}
uint16_t MOTOR::decel_step(){return decel;}
uint16_t MOTOR::delay_short(){return dla_s;}
uint16_t MOTOR::delay_long(){return dla_l;}

void MOTOR::run_drive(bool direction, uint8_t limit_sw, uint32_t step, uint8_t acceleration, uint8_t deceleration, uint8_t speed_max, uint16_t speed_min, uint32_t hight_max){
  boolean celerations = false;
  float  speed_change_ac = 0.0;
  float  speed_change_de = 0.0;

  if(acceleration < 1){acceleration = 1;}
  else if(acceleration > 45){acceleration = 45;}  
  if(deceleration < 1){deceleration = 1;}
  else if(deceleration > 45){deceleration = 45;}

  if(speed_max < 1){speed_max = 1;}
  const float percent_ac = acceleration/100.0;
  const float section_ac = step*percent_ac;
  const float percent_de = deceleration/100.0;
  const float section_de = step*percent_de;

  if(speed_min > speed_max){
    celerations  = true;
    speed_change_ac = ((speed_min - speed_max) / section_ac);
    speed_change_de = ((speed_min - speed_max) / section_de);
  }

  float speed          = speed_min;
  uint32_t distance_ac = uint32_t(section_ac)+1;
  uint32_t distance_de = uint32_t(section_de)+1;
  uint16_t adjust      = speed_min;

  if(direction){ //up
    for (uint32_t index=0; index < step; index++) {
      //speed change
      if(Zero_set && (Position < hight_max)){
        if(celerations){
          if(index < distance_ac){
            if(speed > speed_max){speed -= speed_change_ac;}
            else{speed = speed_max;}
            adjust = uint16_t(speed);
          }else if(index > (step - distance_de)){
            speed += speed_change_de;
            adjust = uint16_t(speed);
          }else{
            adjust = uint16_t(speed); //for delay (move smoth)
          }
        }else{
          speed = speed_max;
        }
      }else{
        adjust = 0;
        //---------check this**********----------- limit sw pin
        if(swich_values(limit_sw, read_shift_regs())){
          Zero_set = true;
          Position = hight_max;
          break; //when push the limit sw, stop
        }
      }
      //---------check this**********---------- max hight
      //if(swich_values(limit_sw, read_shift_regs())) break; //when push the limit sw, stop
      //if(Position > 1000) break; //if Position is higher than maximum hight, stop
      digitalWrite(step_moter.PWM, true);
      Position += 1;
      digitalWrite(step_moter.PWM, false);
      delayMicroseconds(adjust);
    }
  }else if(!direction){  //down
    for (uint32_t index=0; index<step; index++) {
      //speed change
      if(Zero_set){
        if(celerations){
          if(index < distance_ac){
            if(speed >speed_max){speed -= speed_change_ac;}
            else{speed = speed_max;}
            adjust = uint16_t(speed);
          }else if(index > (step - distance_de)){
            speed += speed_change_de;
            adjust = uint16_t(speed);
          }else{
            adjust = uint16_t(speed); //for delay (move smoth)
          }
        }else{
          speed = speed_max;
        }
      }else{
        adjust = 0;
        //---------check this**********----------- limit sw pin
        if(swich_values(limit_sw, read_shift_regs())){
          Zero_set = true;
          Position = 0;
          break; //when push the limit sw, stop
        }
      }
      digitalWrite(step_moter.DIR, true);
      if(Position > 0){
        Position -= 1;
      }else{
        Zero_set = false;
      }
      digitalWrite(step_moter.DIR, false);
      delayMicroseconds(adjust);
    }
  }
}
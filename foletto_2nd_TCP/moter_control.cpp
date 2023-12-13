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
  pinMode(moter_pins.DIR, OUTPUT);
  pinMode(moter_pins.PWM, OUTPUT);
  digitalWrite(moter_pins.DIR, false);
  digitalWrite(moter_pins.PWM, false);
  Serial.print("DIR: ");
  Serial.print(moter_pins.DIR);
  Serial.print(" ,PWM: ");
  Serial.println(moter_pins.PWM);
}

void MOTOR::set_config(uint16_t v_accel, uint16_t v_decel, uint16_t v_dla_s, uint16_t v_dla_l){
  accel = v_accel;
  decel = v_decel;
  dla_s = v_dla_s;
  dla_l = v_dla_l;
}

void MOTOR::status(){
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
  else if(Position > 0) Position--;
}

uint32_t MOTOR::get_pos(){return Position;}
bool     MOTOR::get_zero_set(){return Zero_set;}
uint16_t MOTOR::accel_step(){return accel;}
uint16_t MOTOR::decel_step(){return decel;}
uint16_t MOTOR::delay_short(){return dla_s;}
uint16_t MOTOR::delay_long(){return dla_l;}

void MOTOR::run_drive(STEP_ts moter_pins, bool direction, uint8_t limit_sw, bool sensor_on, uint32_t step, uint32_t hight_max, uint8_t brake, uint32_t extra){
  if(limit_sw>15) Zero_set = true;
  boolean celerations = false;
  float  speed_change_ac = 0.0;
  float  speed_change_de = 0.0;
  Serial.print("PWM:");
  Serial.print(moter_pins.PWM);
  Serial.print(" ,DIR:");
  Serial.println(moter_pins.DIR);

  if(this->dla_l > this->dla_s){
    celerations  = true;
    speed_change_ac = (((this->dla_l - this->dla_s)*1.00) / (this->accel*1.00));
    speed_change_de = (((this->dla_l - this->dla_s)*1.00) / (this->decel*1.00));
  }

  float speed          = this->dla_l;
  uint32_t distance_ac = uint32_t(this->accel)+1;
  uint32_t distance_de = uint32_t(this->decel)+1;
  uint16_t adjust      = this->dla_l;

  if(brake !=0 && brake < 8) digitalWrite(relay_pin[brake-1], true);  //브레이크 풀기

  if(direction){ //up
    for (uint32_t index=0; index < step; index++) {
      //speed change
      if(Zero_set && (Position < hight_max)){
        if(celerations){
          if(index < distance_ac){
            if(speed > this->dla_s){speed -= speed_change_ac;}
            else{speed = this->dla_s;}
            adjust = uint16_t(speed);
          }else if(index > (step - distance_de)){
            speed += speed_change_de;
            adjust = uint16_t(speed);
          }else{
            adjust = uint16_t(speed); //for delay (move smoth)
          }
        }else{
          speed = this->dla_s;
        }
      }else{
        adjust = 0;
        //---------check this**********----------- limit sw pin
        if(swich_values(limit_sw, read_shift_regs(), sensor_on)){
          Zero_set = true;
          Position = hight_max;
          if(extra-- <= 1) break; //when push the limit sw, stop
        }
      }
      //---------check this**********---------- max hight
      //if(swich_values(limit_sw, read_shift_regs())) break; //when push the limit sw, stop
      //if(Position > 1000) break; //if Position is higher than maximum hight, stop
      digitalWrite(moter_pins.PWM, true);
      Position += 1;
      digitalWrite(moter_pins.PWM, false);
      delayMicroseconds(adjust);
    }
  }else if(!direction){  //down
    for (uint32_t index=0; index<step; index++) {
      //speed change
      if(Zero_set){
        if(celerations){
          if(index < distance_ac){
            if(speed >this->dla_s){speed -= speed_change_ac;}
            else{speed = this->dla_s;}
            adjust = uint16_t(speed);
          }else if(index > (step - distance_de)){
            speed += speed_change_de;
            adjust = uint16_t(speed);
          }else{
            adjust = uint16_t(speed); //for delay (move smoth)
          }
        }else{
          speed = this->dla_s;
        }
      }else{
        adjust = 0;
        //---------check this**********----------- limit sw pin
        if(swich_values(limit_sw, read_shift_regs(), sensor_on)){
          Zero_set = true;
          Position = 0;
          if(extra-- <= 1) break; //when push the limit sw, stop
        }
      }
      digitalWrite(moter_pins.DIR, true);
      if(Position > 0){
        Position -= 1;
      }else{
        Zero_set = false;
      }
      digitalWrite(moter_pins.DIR, false);
      delayMicroseconds(adjust);
    }
  }

  if(brake !=0 && brake < 8) digitalWrite(relay_pin[brake-1], false);  //브레이크 잠금
}


void MOTOR::run_moter(STEP_ts moter_pins, uint8_t motor_number, bool direction, uint8_t limit_sw, bool sensor_on, uint32_t step, uint32_t hight_max, uint8_t brake){
  if(limit_sw>15) Zero_set = true;
  boolean celerations = false;
  float  speed_change_ac = 0.0;
  float  speed_change_de = 0.0;
  Serial.print("PWM:");
  Serial.print(moter_pins.PWM);
  Serial.print(" ,DIR:");
  Serial.println(moter_pins.DIR);

  if(this->dla_l > this->dla_s){
    celerations  = true;
    speed_change_ac = (((this->dla_l - this->dla_s)*1.00) / (this->accel*1.00));
    speed_change_de = (((this->dla_l - this->dla_s)*1.00) / (this->decel*1.00));
  }

  float speed          = this->dla_l;
  uint32_t distance_ac = uint32_t(this->accel)+1;
  uint32_t distance_de = uint32_t(this->decel)+1;
  uint16_t adjust      = this->dla_l;

  digitalWrite(BUITIN_EN, true);
  if(brake !=0 && brake < 8) digitalWrite(relay_pin[brake-1], true);  //브레이크 풀기

  if(motor_number == 2 || motor_number == 3){
    digitalWrite(moter_pins.DIR, direction);
  }else{
    if(direction){
      if(motor_number == 0){
        PORTE |= 0b01000000; //on
      }else if(motor_number == 1){
        PORTE |= 0b10000000; //on
      }else if(motor_number == 4){
        PORTE |= 0b00000100; //on
      }else if(motor_number == 5){
        PORTH |= 0b10000000; //on
      }
    }else{
      if(motor_number == 0){
        PORTE &= 0b10111111; //off
      }else if(motor_number == 1){
        PORTE &= 0b01111111; //off
      }else if(motor_number == 4){
        PORTE &= 0b11111011; //off
      }else if(motor_number == 5){
        PORTH &= 0b01111111; //off
      }
    }
  }

  if(direction){ //up
    for (uint32_t index=0; index < step; index++) {
      //speed change      
      if(Zero_set && (Position < hight_max)){
        if(celerations){
          if(index < distance_ac){
            if(speed > this->dla_s){speed -= speed_change_ac;}
            else{speed = this->dla_s;}
            adjust = uint16_t(speed);
          }else if(index > (step - distance_de)){
            speed += speed_change_de;
            adjust = uint16_t(speed);
          }else{
            adjust = uint16_t(speed); //for delay (move smoth)
          }
        }else{
          speed = this->dla_s;
        }
      }else{
        adjust = 0;
        //---------check this**********----------- limit sw pin
        if(swich_values(limit_sw, read_shift_regs(), sensor_on)){
          Zero_set = true;
          Position = hight_max;
          break; //when push the limit sw, stop
        }
      }
      //---------check this**********---------- max hight
      //if(swich_values(limit_sw, read_shift_regs())) break; //when push the limit sw, stop
      //if(Position > 1000) break; //if Position is higher than maximum hight, stop
      Position += 1;
      digitalWrite(moter_pins.PWM, true);
      delayMicroseconds(10);
      digitalWrite(moter_pins.PWM, false);
      delayMicroseconds(adjust);
    }
  }else if(!direction){  //down
    for (uint32_t index=0; index<step; index++) {
      //speed change
      if(Zero_set){
        if(celerations){
          if(index < distance_ac){
            if(speed >this->dla_s){speed -= speed_change_ac;}
            else{speed = this->dla_s;}
            adjust = uint16_t(speed);
          }else if(index > (step - distance_de)){
            speed += speed_change_de;
            adjust = uint16_t(speed);
          }else{
            adjust = uint16_t(speed); //for delay (move smoth)
          }
        }else{
          speed = this->dla_s;
        }
      }else{
        adjust = 0;
        //---------check this**********----------- limit sw pin
        if(swich_values(limit_sw, read_shift_regs(), sensor_on)){
          Zero_set = true;
          Position = 0;
          break; //when push the limit sw, stop
        }
      }
      if(Position > 0){
        Position -= 1;
      }else{
        Zero_set = false;
      }
      digitalWrite(moter_pins.PWM, true);
      delayMicroseconds(10);
      digitalWrite(moter_pins.PWM, false);
      delayMicroseconds(adjust);
    }
  }

  if(brake !=0 && brake < 8) digitalWrite(relay_pin[brake-1], false);  //브레이크 잠금
  digitalWrite(BUITIN_EN, false);
}
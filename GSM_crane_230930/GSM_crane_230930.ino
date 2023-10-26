#include <EEPROM.h>
#include "HX711.h"
#define  SERIAL_MAX  10

boolean  load_view = true;

const uint8_t eeprom_1st = 1;
const uint8_t eeprom_2nd = 2;
uint16_t      load_max   = 65535;

HX711 loadcell;
const uint8_t HX711_dataPin  = 50;
const uint8_t HX711_clockPin = 52;

uint16_t    load_weght = 0;
float       load_raw   = 0.00;
const float spring_k   = 0.001;
const float offset_spr = 0.00;

unsigned long Update_time               = 0UL;
unsigned long prevUpdate_load_monit     = 0UL;
unsigned long prevUpdate_load_display   = 0UL;
unsigned long prevUpdate_warn_reel      = 0UL;
unsigned long prevUpdate_warn_load      = 0UL;
unsigned long prevUpdate_lever_reel     = 0UL;
unsigned long prevUpdate_lever_cylinder = 0UL;
char Serial_buf[SERIAL_MAX];
uint8_t Serial_num = 0;

//35,47,37/49,33,43/31,41,39/29
const uint8_t over_reel             = 29;
const uint8_t lever_reel_up         = 35;
const uint8_t lever_reel_down       = 37;
const uint8_t lever_cylinder_up     = 49;
const uint8_t lever_cylinder_down   = 43;

const uint8_t relay_reel_up         = 22;
const uint8_t relay_reel_down       = 24;
const uint8_t relay_cylinder_up     = 26;
const uint8_t relay_cylinder_down   = 28;

const uint8_t relay_bulb[4]   = {14,15,18,19};

boolean state_reel_up       = false;
boolean state_reel_down     = false;
boolean state_cylinder_up   = false;
boolean state_cylinder_down = false;

bool flage_l = false;
bool flage_m = false;

void Serial_process() {
  char ch;
  ch = Serial2.read();
  switch ( ch ) {
    case 0x23:
      Serial_buf[9] = '#';
      Serial_service();
      Serial_num = 0;
      break;
    case 0x40:
      Serial_buf[0] = '@';
      Serial_num = 1;
      break;
    default :
      Serial_buf[ Serial_num ++ ] = ch;
      Serial_num %= SERIAL_MAX;
      break;
  }
}

void Serial_service() {
  if(Serial_buf[0]=='@'){
    if(Serial_buf[1]=='I'&&Serial_buf[2]=='N'&&Serial_buf[3]=='I'&&Serial_buf[4]=='T'){
      send2Nextion("flage_r.val=1");
      Serial.println("flage_r");
    }else if(Serial_buf[1]=='P'&&Serial_buf[2]=='A'&&Serial_buf[3]=='G'&&Serial_buf[4]=='E'){
      if(int(Serial_buf[5]) == 0){
        load_view = true;
      }else{
        load_view = false;
      }
    }else if(Serial_buf[1]=='M'&&Serial_buf[2]=='O'&&Serial_buf[3]=='D'&&Serial_buf[4]=='I'){
      load_max = Serial_buf[5] + Serial_buf[6]*256;
      EEPROM.write(eeprom_1st,Serial_buf[5]);
      EEPROM.write(eeprom_2nd,Serial_buf[6]);
      send2Nextion("kgf.val=1");
      Serial.println("kgf_set");
    }
  }  
}//Serial_service

void setup() {
  Serial.begin(115200);
  Serial2.begin(57600);
  loadcell.begin(HX711_dataPin, HX711_clockPin);

  load_max = EEPROM.read(eeprom_1st) + EEPROM.read(eeprom_2nd)*255;

  pinMode(over_reel,INPUT_PULLUP);
  pinMode(lever_reel_up,INPUT_PULLUP);
  pinMode(lever_reel_down,INPUT_PULLUP);
  pinMode(lever_cylinder_up,INPUT_PULLUP);
  pinMode(lever_cylinder_down,INPUT_PULLUP);

  pinMode(relay_reel_up,OUTPUT);
  pinMode(relay_reel_down,OUTPUT);
  pinMode(relay_cylinder_up,OUTPUT);
  pinMode(relay_cylinder_down,OUTPUT);
  for (int8_t index=0; index<4; index++) {
  pinMode(relay_bulb[index],OUTPUT);
  }
  
  Serial.print("max load: ");
  Serial.println(load_max);
}
void loop() {
  Update_time = millis();
  // put your main code here, to run repeatedly:
  DisplayConnect();  
  load_monit(Update_time);
  warning(Update_time);
  lever(Update_time);
}//loop()

void load_monit(unsigned long Update_time) {
  if(Update_time > prevUpdate_load_monit + 20){
    prevUpdate_load_monit = Update_time;
    load_raw   = (2*load_raw+loadcell.read())/3;
    if(load_raw<0){load_weght = 0;}
    else{load_weght = int(spring_k*load_raw+offset_spr);}
  }//Update_time
  if(Update_time > prevUpdate_load_display + 400){
    prevUpdate_load_display = Update_time;
    Display("kgfv",load_weght);
  }//Update_time
}//warning()

void warning(unsigned long Update_time) {
  if((Update_time > prevUpdate_warn_reel + 500)){
    prevUpdate_warn_reel = Update_time;
    if(!digitalRead(over_reel)){
      if(state_reel_up){
        state_reel_up = false;
        digitalWrite(relay_reel_up,false);
      }
      if(load_view){
        flage_l = true;
        send2Nextion("flage_w.val=1");
        send2Nextion("flage_l.val=1");
        digitalWrite(relay_bulb[2],true);
      }
    }else if(load_view){
      flage_l = false;
      send2Nextion("flage_l.val=0");
      digitalWrite(relay_bulb[2],false);
      if(!flage_l && !flage_m)  send2Nextion("flage_w.val=0");
    }
  }
  //Update_time
  if((Update_time > prevUpdate_warn_load + 500)){
    if((load_weght > load_max)){
      prevUpdate_warn_load = Update_time;
      if(load_view){
        flage_m = true;
        send2Nextion("flage_w.val=1");
        send2Nextion("flage_m.val=1");    
        digitalWrite(relay_bulb[3],true);
      }
    }else if(load_view){
      flage_m = false;
      send2Nextion("flage_m.val=0");
      digitalWrite(relay_bulb[3],false);
      if(!flage_l && !flage_m)  send2Nextion("flage_w.val=0");
    }
  }//Update_time
}//warning()

void lever(unsigned long Update_time) {
  if(Update_time > prevUpdate_lever_reel + 10){
    prevUpdate_lever_reel = Update_time;
    if(!digitalRead(lever_reel_up) && digitalRead(over_reel)){
      if(state_reel_down){
        state_reel_down = false;
        digitalWrite(relay_reel_down,false);
      }else if(!state_reel_up){
        state_reel_up = true;
        digitalWrite(relay_reel_up,true);
      }
    }else if(!digitalRead(lever_reel_down)){
      if(state_reel_up){
        state_reel_up = false;
        digitalWrite(relay_reel_up,false);
      }else if(!state_reel_down){
        state_reel_down = true;
        digitalWrite(relay_reel_down,true);
      }
    }else{
      if(state_reel_up){
        state_reel_up = false;
        digitalWrite(relay_reel_up,false);
      }
      if(state_reel_down){
        state_reel_down = false;
        digitalWrite(relay_reel_down,false);
      }
    }
  }
  ///////////////////////////////////////////
  if(Update_time > prevUpdate_lever_cylinder + 10){
    prevUpdate_lever_cylinder = Update_time;
    if(!digitalRead(lever_cylinder_up)){
      if(state_cylinder_down){
        state_cylinder_down = false;
        digitalWrite(relay_cylinder_down,false);
      }else if(!state_cylinder_up){
        state_cylinder_up = true;
        digitalWrite(relay_cylinder_up,true);
      }
    }else if(!digitalRead(lever_cylinder_down)){
      if(state_cylinder_up){
        state_cylinder_up = false;
        digitalWrite(relay_cylinder_up,false);
      }else if(!state_cylinder_down){
        state_cylinder_down = true;
        digitalWrite(relay_cylinder_down,true);
      }
    }else{
      if(state_cylinder_up){
        state_cylinder_up = false;
        digitalWrite(relay_cylinder_up,false);
      }
      if(state_cylinder_down){
        state_cylinder_down = false;
        digitalWrite(relay_cylinder_down,false);
      }
    }
  }//Update_time
}//lever()

void DisplayConnect() {
  if (Serial2.available()) {
    Serial_process();
  }
}

void Display(String IDs, uint16_t values) {
  String cmd;
  char buf[8] = {0};
  sprintf(buf, "%d", values);
  cmd = IDs + ".val=";
  cmd += buf;
  send2Nextion(cmd);
}

void send2Nextion(String cmd) {
  Serial2.print(cmd);
  Serial2.write(0xFF);
  Serial2.write(0xFF);
  Serial2.write(0xFF);
}
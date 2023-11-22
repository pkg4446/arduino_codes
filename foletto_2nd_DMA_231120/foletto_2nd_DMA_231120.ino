#include "pin_setup.h"
#include "shift_regs.h"
#include "moter_control.h"

#include <EEPROM.h>
#include <SPI.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <Ethernet.h>
#include <EthernetClient.h>
#include <Dns.h>
#include <Dhcp.h>

#include <ArduinoJson.h>

#define JSON_STACK 144

#define DEBUG
#define DEBUG_MQTT
//#define DEBUG_STEP
//#define DEBUG_SHIFT_REGS

/************************* Ethernet Client Setup *****************************/
//byte mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, byte(random(0, 255))};
byte mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x00};

IPAddress ip(192, 168, 0, random(0, 255));
IPAddress myDns(192, 168, 0, 1);
EthernetClient client;
/************************* Mqtt Server Setup *********************************/
#define AIO_SERVER      "mqtt.kro.kr"
#define AIO_SERVER_PORT  1883
#define AIO_USERNAME    "test"
#define AIO_KEY         "test"

const char* AIO_Publish = "foletto_pub_hub";
char AIO_Subscribe[24]  = {0x00,};
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVER_PORT, AIO_USERNAME, AIO_KEY);
// You don't need to change anything below this line!
// Setup a mqtt 
Adafruit_MQTT_Publish   response = Adafruit_MQTT_Publish(&mqtt,   AIO_Publish); //Adafruit_MQTT_Publish(&mqtt,   AIO_Publish, 1);
Adafruit_MQTT_Subscribe request  = Adafruit_MQTT_Subscribe(&mqtt, AIO_Subscribe); //Adafruit_MQTT_Subscribe(&mqtt, AIO_Subscribe, 1);
/************************* Mqtt End *********************************/

/************************* values *********************************/
MOTOR driver[4];
MOTOR builtin[6];
unsigned long relay_start_time[7] = {0UL,};
unsigned long relay_end_time[7]   = {0UL,};
bool relay_state[7];
bool online = false;
/************************* values *********************************/

/******
{
    "ctrl":  "motor",
    "cmd":   "set",
    "opt":   1,   //0내장 드라이버, 1외장드라이버(MD5-HD14)
    "num":   1,   // motor type e.g. 1번 모터 // 0 to 255
    "accel": 10,  //가속 구간 스탭 수 unsigned int16 - 0 to 65535
    "decel": 5,   //감속 구간 스탭 수 unsigned int16 - 0 to 65535
    "dla_s": 10,  //unsigned int 16, 모터 스탭간격 micro sec, delay_short
    "dla_l": 1000 //unsigned int 16, 모터 스탭간격 micro sec, delay_long > delay_short    
}

step motor ctrl request (사용 전, 후, relay status change로 반드시 브레이크 off, on)
{
    "ctrl":  "motor",
    "cmd":   "run",
    "opt":   1,   //0내장 드라이버, 1외장드라이버(MD5-HD14)
    "num":   1,   // motor type e.g. 1번 모터 // 0 to 255
    "step":  10,  //스탭 수 (n 스탭당 1회전) unsigned int32 - 0 to 4294967295        
    "dir":   1,   //step motor direction (CW/CCW), 0:CW, 1:CCW
    "limit": 4,   //모터 진행방향 리미트센서 번호 = sensor_read 번호 
    "max":   1000 //unsigned int 36, 가동 최대 거리 //리미트 한쪽에만 있을 경우, 리미트 인식 후 pos가 0이 되고, 이후 최대 max 까지만 회전
}
*******/
/*************/
uint8_t  shift_read       = 0;
bool     builtin_run[6]   = {false,};
bool     builtin_dir[6]   = {false,};
bool     builtin_pulse_swich[6] = {false,};
uint32_t builtin_pulse[6] = {0,};
uint32_t builtin_pulse_start[6] = {0,};
uint32_t builtin_pulse_end[6]   = {0,};
uint8_t  builtin_limit[6] = {16,};
uint16_t builtin_speed[6] = {0L,};
float  builtin_speed_f[6] = {0.00f,};
float  builtin_speed_accel[6] = {0.00f,};
float  builtin_speed_decel[6] = {0.00f,};
unsigned long builtin_interval[6] = {0UL,};

void builtin_stepper(){
  BYTES_VAL_T pinValues;
  bool builtin_progress = false;
  for(uint8_t index=0; index<6; index++){
    if(builtin_run[index]) builtin_progress = true;
  }
  if(builtin_progress){
    digitalWrite(BUITIN_EN, true);
    unsigned long builtin_time = micros();
    if(shift_read++ > 160){
      pinValues   = read_shift_regs();
      shift_read  = 0;
    }

    for(uint8_t index=0; index<6; index++){
      if(builtin_run[index] && (builtin_time - builtin_interval[index] > builtin_speed[index] + 200)){
        builtin_interval[index] = builtin_time;
        if(!swich_values(builtin_limit[index], pinValues)){
          digitalWrite(stepMotor[index].PWM, true);
          builtin_pulse_swich[index] = true;
          #ifdef DEBUG_STEP
          Serial.print(builtin_speed_f[index]);
          Serial.print(" = ");
          Serial.println(builtin_speed[index]);
          #endif
        }else{
          builtin_run[index] = false;
          response_moter_status("motor", "run", 0, index +1, builtin[index].get_zero_set(), builtin[index].get_pos());
          //종료
        }
      }
    }
    for(uint8_t index=0; index<6; index++){
      if(builtin_pulse_swich[index] && builtin_run[index]){
        builtin_pulse_swich[index] = false;
        digitalWrite(stepMotor[index].PWM, false);
        builtin[index].pos_update(builtin_dir[index]);
        builtin_pulse[index]--;
        if(builtin_pulse[index] == 0){
          builtin_run[index] = false;
          response_moter_status("motor", "run", 0, index +1, builtin[index].get_zero_set(), builtin[index].get_pos());
        }else if(builtin_pulse[index] >= builtin_pulse_start[index]){
          builtin_speed_f[index] -= builtin_speed_accel[index];
          builtin_speed[index] = builtin_speed_f[index] + 0.01;
        }else if(builtin_pulse[index] <= builtin_pulse_end[index]){
          builtin_speed_f[index] += builtin_speed_decel[index];
          builtin_speed[index] = builtin_speed_f[index] + 0.01;
        }
      }
    }
  }else{
    digitalWrite(BUITIN_EN, false);
  }
}
/***************/
/***************/
/***************/
/***************/

void init_port_base(){
  DDRE |= 0b11000100;
  DDRH |= 0b10000000;
  /*
  PORTE &= 0b00111011; //off
  
  PORTH &= 0b01111111; //off
  
  delay(1000);
  PORTE |= 0b11000100; //on
  PORTH |= 0b10000000; //on
  delay(1000);
  */
  for (uint8_t index=0 ; index<7; index++) {
    pinMode(relay_pin[index],OUTPUT);
    digitalWrite(relay_pin[index], false);
    relay_state[index] = false;
  }
}

char    Serial_buf[144];
uint8_t Serial_num = 0;

void Serial_process() {
  char ch;
  ch = Serial.read();
  if(ch=='}'){
    Serial_buf[Serial_num] = '}';
    Serial_buf[Serial_num+1] = 0x00;
    command_pros(Serial_buf);
    Serial_num = 0;
  }else if(ch=='{'){
    Serial_num = 0;
    Serial_buf[Serial_num++]='{';
  }else{
    Serial_buf[ Serial_num++ ] = ch;
    Serial_num %= 144;
  }
}

//**********MQTT**********//
void MQTT_connect() {
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  Ethernet.maintain();
  int8_t ret = mqtt.connect();
  Serial.println("Connecting to MQTT.");
  unsigned long interval_mqtt_retry = 0L;
  while (ret != 0) { // connect will return 0 for connected
    unsigned long mqtt_retry = millis();
    if(Serial.available()) Serial_process();
    if(mqtt_retry - interval_mqtt_retry > 1000){
      Serial.println(mqtt.connectErrorString(ret));
      mqtt.disconnect();
      interval_mqtt_retry = mqtt_retry;
      Serial.println("Retrying MQTT connection.");
      ret = mqtt.connect();
    }    
  }
  Serial.println("MQTT Connected!");
}

void mqtt_requeset(){
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(1))) {
    if (subscription == &request) {
      String receive = (char *)request.lastread;
      command_pros(receive);
    }//subscription
  }//while
}//mqtt_requeset()

void command_pros(String receive){
  #ifdef DEBUG_MQTT
    Serial.print("MQTT: ");
    Serial.println(receive);
  #endif
  StaticJsonDocument<JSON_STACK> json;
  deserializeJson(json, receive);
  String control = json["ctrl"];
  String command = json["cmd"];

  if(control.equalsIgnoreCase("mqtt_sub")){    
    String mqtt_sub_cmd = json["cmd"];
    for(uint8_t index=0; index<mqtt_sub_cmd.length(); index++){
      byte sub_char = mqtt_sub_cmd[index];
      EEPROM.write(index+1, sub_char);
    }
    EEPROM.write(mqtt_sub_cmd.length()+1, 0x00);
    Serial.print(mqtt_sub_cmd);
  }else if(control.equalsIgnoreCase("macaddr")){
    uint8_t macaddr = json["cmd"];
    EEPROM.write(0, macaddr);
  }

  mqtt_receive(control,command);
  /************************************************/
  if(control.equalsIgnoreCase("relay")){
    /**********************************************/
    int8_t relay_number = json["num"];
    if(relay_number<1 || relay_number>7){
      mqtt_err_msg(control,"select wrong");
    }else if(command.equalsIgnoreCase("onoff")){
      relay_number -= 1;
      int16_t duration  = json["opt"];
      relay_state[relay_number] = true;
      digitalWrite(relay_pin[relay_number], true);
      unsigned long sys_time = millis();
      relay_start_time[relay_number] = sys_time;
      relay_end_time[relay_number]   = sys_time + duration;
    }else if(command.equalsIgnoreCase("hold")){
      relay_number -= 1;
      bool status = json["opt"];
      digitalWrite(relay_pin[relay_number], status);
      relay_status_change(relay_number+1, status);
    }else{mqtt_err_msg(control,"command null");}
    /**********************************************/
  }else if(control.equalsIgnoreCase("sensor")){
    /**********************************************/
    if(command.equalsIgnoreCase("read")){
      read_pin_values();
    }else{mqtt_err_msg(control,"command null");}
    /**********************************************/
  }else if(control.equalsIgnoreCase("motor")){
    uint8_t motor_number = json["num"];
    bool    drive        = json["opt"];
    if(drive){ //(MD5-HD14)
      if(motor_number<1 || motor_number>4){
        mqtt_err_msg(control,"select wrong");
      }else{
        motor_number -= 1;
        if(command.equalsIgnoreCase("set")){
          driver[motor_number].set_config(json["accel"], json["decel"], json["dla_s"], json["dla_l"]);
          response_moter_config(control,command,drive,motor_number+1,driver[motor_number].accel_step(),driver[motor_number].decel_step(),driver[motor_number].delay_short(),driver[motor_number].delay_long());
        }else if(command.equalsIgnoreCase("run")){
          bool relay_busy = false;
          for(uint8_t index = 0; index < 7; index++){
            if(relay_state[index]) relay_busy = true;
          }
          if(relay_busy){
            mqtt_err_msg(control,"relay is busy");
          }else{
            driver[motor_number].run_drive(stepDriver[motor_number],json["dir"],json["limit"],json["step"],json["max"]);
            response_moter_status("motor", "run", drive, motor_number +1, driver[motor_number].get_zero_set(), driver[motor_number].get_pos());
          }
        }else if(command.equalsIgnoreCase("status")){
          Serial.print("driver");
          Serial.print(motor_number);
          Serial.print(":");
          driver[motor_number].status();
        }else if(command.equalsIgnoreCase("config")){
          
        }else{mqtt_err_msg(control,"command null");}
      }
    }else{ //builtin driver
      if(motor_number<1 || motor_number>6){
        mqtt_err_msg(control,"select wrong");
      }else{
        motor_number -= 1;
        if(command.equalsIgnoreCase("set")){
          builtin[motor_number].set_config(json["accel"], json["decel"], json["dla_s"], json["dla_l"]);
          response_moter_config(control,command,drive,motor_number+1,builtin[motor_number].accel_step(),builtin[motor_number].decel_step(),builtin[motor_number].delay_short(),builtin[motor_number].delay_long());
        }else if(command.equalsIgnoreCase("run")){
          builtin_dir[motor_number] = json["dir"];
          if(motor_number == 2 || motor_number == 3){
            digitalWrite(stepMotor[motor_number].DIR, builtin_dir[motor_number]);
          }else{
            if(builtin_dir[motor_number]){
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

          builtin_pulse[motor_number] = json["step"];
          builtin_limit[motor_number] = json["limit"];
          builtin_run[motor_number]   = true;
          uint16_t spd_gap = builtin[motor_number].delay_long() - builtin[motor_number].delay_short();
          builtin_speed[motor_number]   = builtin[motor_number].delay_long();
          builtin_speed_f[motor_number] = builtin[motor_number].delay_long();
          builtin_pulse_start[motor_number] = builtin[motor_number].accel_step()+1;
          builtin_pulse_end[motor_number]   = builtin[motor_number].decel_step()+1;
          builtin_speed_accel[motor_number] = (spd_gap*1.00)/(builtin_pulse_start[motor_number]*1.00);
          builtin_speed_decel[motor_number] = (spd_gap*1.00)/(builtin_pulse_end[motor_number]*1.00);
          #ifdef DEBUG
            Serial.print("spd_gap="); Serial.println(spd_gap);
            Serial.print("accel step="); Serial.print(builtin_pulse_start[motor_number]);
            Serial.print(",accel="); Serial.println(builtin_speed_accel[motor_number]);
            Serial.print("decel step="); Serial.print(builtin_pulse_end[motor_number]);
            Serial.print(",decel="); Serial.println(builtin_speed_decel[motor_number]);
            Serial.print(",total_step="); Serial.println(builtin_pulse[motor_number]);
          #endif

          builtin_pulse_start[motor_number] = builtin_pulse[motor_number] - builtin_pulse_start[motor_number];
          //json["max"];
        }else if(command.equalsIgnoreCase("status")){
          Serial.print("builtin");
          Serial.print(motor_number);
          Serial.print(":");
          driver[motor_number].status();
        }else if(command.equalsIgnoreCase("config")){
          
        }else{mqtt_err_msg(control,"command null");}
      }
    }
  }else{
    Serial.println("nope");
    mqtt_err_msg("null","command error");
  }
}//mqtt_requeset()

void mqtt_response(const char* send_data){
  if (online && !response.publish(send_data)) {
    mqtt_response(send_data);
  }else{
    Serial.print("mqtt response ok: ");
    Serial.println(send_data);
  }
}

void mqtt_err_msg(String type, String error_msg){
  DynamicJsonDocument res(JSON_STACK);
  res["id"]   = AIO_Subscribe;
  res["ctrl"] = "error";
  res["cmd"]  = type;
  res["err"]  = error_msg;
  
  String json="";
  serializeJson(res, json);
  char buffer[json.length() + 1];
  json.toCharArray(buffer, json.length() + 1);
  mqtt_response(buffer);
}

void mqtt_receive(String control, String command){
  DynamicJsonDocument res(JSON_STACK);
  res["id"]   = AIO_Subscribe;
  res["ctrl"] = "receive";
  res["cmd"]  = control;
  res["opt"]  = command;
  
  String json="";
  serializeJson(res, json);
  char buffer[json.length() + 1];
  json.toCharArray(buffer, json.length() + 1);
  mqtt_response(buffer);
}
//**********End Of MQTT**********//
void response_moter_config(String control, String command, bool drive, uint8_t motor_number, uint16_t v_accel, uint16_t v_decel, uint16_t v_dla_s, uint16_t v_dla_l){
  DynamicJsonDocument res(JSON_STACK);
  res["id"]    = AIO_Subscribe;
  res["ctrl"]  = control;
  res["cmd"]   = command;
  res["opt"]   = drive;
  res["num"]   = motor_number;
  res["accel"] = v_accel;
  res["decel"] = v_decel;
  res["dla_s"] = v_dla_s;
  res["dla_l"] = v_dla_l;

  String json="";
  serializeJson(res, json);
  char buffer[json.length() + 1];
  json.toCharArray(buffer, json.length() + 1);
  mqtt_response(buffer);
}

void response_moter_status(String ctrl, String command, bool drive, uint8_t number, bool zero, uint32_t pos){
  DynamicJsonDocument res(JSON_STACK);
  res["id"]   = AIO_Subscribe;
  res["ctrl"] = ctrl;
  res["cmd"]  = command;
  res["opt"]  = drive;
  res["num"]  = number;
  res["zero"] = zero;
  res["pos"]  = pos;

  String json="";
  serializeJson(res, json);
  char buffer[json.length() + 1];
  json.toCharArray(buffer, json.length() + 1);
  mqtt_response(buffer);
}

void read_pin_values(){
    BYTES_VAL_T pinValues = read_shift_regs();

    String json = "{\"id\":\"";
    json += AIO_Subscribe;
    json += "\",\"ctrl\":\"sensor_read\",\"sensor\":[";

    for(int i = 0; i < DATA_WIDTH; i++)
    {
      json += (pinValues >> i) & 1;
      if(i != DATA_WIDTH-1) json += ",";
    }
    json += "]}";
    
    char buffer[json.length() + 1];
    json.toCharArray(buffer, json.length() + 1);
    mqtt_response(buffer);
}

void relay_status_change(uint8_t index, bool status){
  DynamicJsonDocument res(JSON_STACK);
  res["id"]   = AIO_Subscribe;
  res["ctrl"] = "relay_hold";
  res["cmd"]  = index;
  res["opt"]  = status;

  String json="";
  serializeJson(res, json);
  char buffer[json.length() + 1];
  json.toCharArray(buffer, json.length() + 1);
  mqtt_response(buffer);
}

void relay_off_awiat(){
  unsigned long sys_time = millis();  
  for (uint8_t index = 0; index<7 ; index++) {
    if(relay_state[index] && (relay_end_time[index] <= sys_time)){
      digitalWrite(relay_pin[index], false);
      relay_state[index] = false;

      DynamicJsonDocument res(JSON_STACK);
      res["id"]   = AIO_Subscribe;
      res["ctrl"] = "relay";
      res["cmd"]  = index+1;
      res["opt"]  = uint16_t(sys_time - relay_start_time[index]);

      String json="";
      serializeJson(res, json);
      char buffer[json.length() + 1];
      json.toCharArray(buffer, json.length() + 1);
      mqtt_response(buffer);
    }
  }
}

//********** setup**********//
void setup() {
  Serial.begin(115200);
  Serial.println("System boot... wait a few seconds.");

  mac[5] = byte(EEPROM.read(0));
  for (uint8_t index = 1; index < 25; index++)
  {
    AIO_Subscribe[index-1] = EEPROM.read(index);
  }
  request  = Adafruit_MQTT_Subscribe(&mqtt, AIO_Subscribe);

  Ethernet.init(53);

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    }
    // try to configure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    online = true;
  }
  shift_regs_init();

  pinMode(BUITIN_EN, OUTPUT);
  for (uint8_t index=0 ; index<4; index++) {
    driver[index].init(stepDriver[index]);
  }
  for (uint8_t index=0 ; index<6; index++) {
    builtin[index].init(stepMotor[index]);
  }
  init_port_base();
  
  if(online){
    Serial.print(Ethernet.maintain());
    Serial.print(" : ");
    Serial.print(mac[5]);
    Serial.print(" : DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
    mqtt.subscribe(&request);
    Serial.print("mqtt_subscribe:");  Serial.println(AIO_Subscribe);
  }
  //online = false;
}//********** End Of Setup() **********//

unsigned long mqtt_req    = 0UL;
//********** loop **********//
void loop() {
  //if need asynchronous, add something
  //unsigned long test1 = micros();
  if(online){
    MQTT_connect();
    unsigned long mqtt_run = millis();
    if(mqtt_run - mqtt_req > 1000){
      mqtt_req = mqtt_run;
      mqtt_requeset();
    }
  }
  if (Serial.available()) Serial_process();

  //Serial.println(micros() - test1);

  relay_off_awiat();
  builtin_stepper();
  #ifdef DEBUG_SHIFT_REGS
    display_pin_values();
  #endif
}//**********End Of loop()**********//

#ifdef DEBUG_SHIFT_REGS
unsigned long interval_74HC165 = 0L;
void display_pin_values()
{
    if(millis() > interval_74HC165 + 1000){
      BYTES_VAL_T pinValues = read_shift_regs();
      interval_74HC165 = millis();
      Serial.print("Pin States:\r\n");
      for(int i = 0; i < DATA_WIDTH; i++)
      {
          Serial.print("  Pin-");
          Serial.print(i);
          Serial.print(": ");
          if((pinValues >> i) & 1)
              Serial.print("HIGH");
          else
              Serial.print("LOW");
          Serial.print("\r\n");
      }
      Serial.print("done.\r\n");
    }
}
#endif
/******************************** ver 1.1.0  ********************************/
/******************************** 2023-12-10 ********************************/
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

#define DRIVER_O  4
#define DRIVER_I  6

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
MOTOR driver[DRIVER_O];
MOTOR builtin[DRIVER_I];
unsigned long relay_start_time[7] = {0UL,};
unsigned long relay_end_time[7]   = {0UL,};
bool relay_state[7];
bool online = false;
BYTES_VAL_T pinValues;
/************************* values *********************************/
uint32_t HIGHT_MAX_O[DRIVER_O] = {9999,};
uint32_t HIGHT_MAX_I[DRIVER_I] = {9999,};
uint8_t  BRAKE_O[DRIVER_O] = {0,};
uint8_t  BRAKE_I[DRIVER_I] = {0,};
bool     SENSOR_ON[DATA_WIDTH+1] = {false,};
/************************* values *********************************/
uint8_t  shift_read       = 0;
bool     builtin_run[6]   = {false,};
bool     builtin_dir[6]   = {false,};
bool     builtin_break[6] = {false,};
bool     builtin_pulse_swich[6] = {false,};
uint32_t builtin_pulse[6] = {0,};
uint32_t builtin_pulse_add[6]   = {0,};
uint32_t builtin_pulse_start[6] = {0,};
uint32_t builtin_pulse_end[6]   = {0,};
uint8_t  builtin_limit[6] = {16,};
uint16_t builtin_speed[6] = {0L,};
float  builtin_speed_f[6] = {0.00f,};
float  builtin_speed_accel[6] = {0.00f,};
float  builtin_speed_decel[6] = {0.00f,};
unsigned long builtin_interval[6] = {0UL,};

void builtin_stepper(){
  bool builtin_progress = false;
  for(uint8_t index=0; index<6; index++){
    if(builtin_run[index]){
      builtin_progress = true;
      if(!builtin_break[index] && BRAKE_I[index] !=0 && BRAKE_I[index] < 8) {
        builtin_break[index] = true;
        digitalWrite(relay_pin[BRAKE_I[index]-1], true);  
        Serial.println("brake open");
      }//브레이크 풀기
    }
  }
  if(builtin_progress){
    digitalWrite(BUITIN_EN, true);
    unsigned long builtin_time = micros();
    if(shift_read++ > 160){
      pinValues  = read_shift_regs();
      shift_read = 0;
    }

    for(uint8_t index=0; index<6; index++){
      if(builtin_run[index] && (builtin_time - builtin_interval[index] > builtin_speed[index] + 200)){
        builtin_interval[index] = builtin_time;
        if(!swich_values(builtin_limit[index], pinValues, SENSOR_ON[builtin_limit[index]])){
          digitalWrite(stepMotor[index].PWM, true);
          builtin_pulse_swich[index] = true;
          #ifdef DEBUG_STEP
          Serial.print(builtin_speed_f[index]);
          Serial.print(" = ");
          Serial.println(builtin_speed[index]);
          #endif
        }else{
          if(builtin_pulse_add[index]-- > 0){ //추가 스탭 이동
            digitalWrite(stepMotor[index].PWM, true);
            builtin_pulse_swich[index] = true;
          }else{
            builtin_run[index] = false;
            if(builtin_break[index]){
              builtin_break[index] = false;
              digitalWrite(relay_pin[BRAKE_I[index]-1], false);
              Serial.println("limit detect");
            }//브레이크 잠금
            response_moter_status("motor", "run", 0, index +1, builtin[index].get_zero_set(), builtin[index].get_pos());  //종료
          }
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
          if(builtin_break[index]){
            builtin_break[index] = false;
            digitalWrite(relay_pin[BRAKE_I[index]-1], false);
            Serial.println("brake close");
          }//브레이크 잠금
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
    }else if(command.equalsIgnoreCase("set")){
      uint8_t sensor_num = json["num"];
      if(sensor_num<DATA_WIDTH){
        uint8_t sensor_on_type = json["opt"];
        EEPROM.write(EEP_SENSOR_ON[sensor_num], sensor_on_type);
        if(sensor_on_type == 0){
          SENSOR_ON[sensor_num] = false;
        }else{
          SENSOR_ON[sensor_num] = true;
        }
        set_pin_values();
      }else{
        mqtt_err_msg(control,"select wrong");
      }
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
          uint16_t temp_accel = driver[motor_number].accel_step();
          uint16_t temp_decel = driver[motor_number].decel_step();
          uint16_t temp_dla_s = driver[motor_number].delay_short();
          uint16_t temp_dla_l = driver[motor_number].delay_long();
          EEPROM.write(eepDriver[motor_number].ACCEL[0], temp_accel/256);
          EEPROM.write(eepDriver[motor_number].ACCEL[1], temp_accel%256);
          EEPROM.write(eepDriver[motor_number].DECEL[0], temp_decel/256);
          EEPROM.write(eepDriver[motor_number].DECEL[1], temp_decel%256);
          EEPROM.write(eepDriver[motor_number].DLY_S[0], temp_dla_s/256);
          EEPROM.write(eepDriver[motor_number].DLY_S[1], temp_dla_s%256);
          EEPROM.write(eepDriver[motor_number].DLY_L[0], temp_dla_l/256);
          EEPROM.write(eepDriver[motor_number].DLY_L[1], temp_dla_l%256);
          response_moter_config(control,command,drive,motor_number+1,temp_accel,temp_decel,temp_dla_s,temp_dla_l);
        }else if(command.equalsIgnoreCase("config")){
          uint8_t  temp_brk = json["brk"];
          EEPROM.write(eepDriver[motor_number].BRAKE, temp_brk);
          uint32_t temp_max = json["max"];
          HIGHT_MAX_I[motor_number] = temp_max;
          EEPROM.write(eepDriver[motor_number].MAX[3], temp_max%256);
          temp_max /= 256;
          EEPROM.write(eepDriver[motor_number].MAX[2], temp_max%256);
          temp_max /= 256;
          EEPROM.write(eepDriver[motor_number].MAX[1], temp_max%256);
          temp_max /= 256;
          EEPROM.write(eepDriver[motor_number].MAX[0], temp_max%256);
        }else if(command.equalsIgnoreCase("run")){
          bool relay_busy = false;
          for(uint8_t index = 0; index < 7; index++){
            if(relay_state[index]) relay_busy = true;
          }
          if(relay_busy){
            mqtt_err_msg(control,"relay is busy");
          }else{
            uint8_t limit_number = json["limit"];
            driver[motor_number].run_drive(stepDriver[motor_number],json["dir"],limit_number,SENSOR_ON[limit_number],json["step"],HIGHT_MAX_O[motor_number],BRAKE_O[motor_number],json["add"]);
            response_moter_status("motor", "run", drive, motor_number +1, driver[motor_number].get_zero_set(), driver[motor_number].get_pos());
          }
        }else if(command.equalsIgnoreCase("status")){
          Serial.print("driver");
          Serial.print(motor_number);
          Serial.print(":");
          driver[motor_number].status();
        }else{mqtt_err_msg(control,"command null");}
      }
    }else{ //builtin driver
      if(motor_number<1 || motor_number>6){
        mqtt_err_msg(control,"select wrong");
      }else{
        motor_number -= 1;
        if(command.equalsIgnoreCase("set")){
          builtin[motor_number].set_config(json["accel"], json["decel"], json["dla_s"], json["dla_l"]);          
          uint16_t temp_accel = builtin[motor_number].accel_step();
          uint16_t temp_decel = builtin[motor_number].decel_step();
          uint16_t temp_dla_s = builtin[motor_number].delay_short();
          uint16_t temp_dla_l = builtin[motor_number].delay_long();
          EEPROM.write(eepMotor[motor_number].ACCEL[0], temp_accel/256);
          EEPROM.write(eepMotor[motor_number].ACCEL[1], temp_accel%256);
          EEPROM.write(eepMotor[motor_number].DECEL[0], temp_decel/256);
          EEPROM.write(eepMotor[motor_number].DECEL[1], temp_decel%256);
          EEPROM.write(eepMotor[motor_number].DLY_S[0], temp_dla_s/256);
          EEPROM.write(eepMotor[motor_number].DLY_S[1], temp_dla_s%256);
          EEPROM.write(eepMotor[motor_number].DLY_L[0], temp_dla_l/256);
          EEPROM.write(eepMotor[motor_number].DLY_L[1], temp_dla_l%256);
          response_moter_config(control,command,drive,motor_number+1,temp_accel,temp_decel,temp_dla_s,temp_dla_l);
        }else if(command.equalsIgnoreCase("config")){
          uint8_t  temp_brk = json["brk"];
          EEPROM.write(eepMotor[motor_number].BRAKE, temp_brk);
          uint32_t temp_max = json["max"];
          HIGHT_MAX_I[motor_number] = temp_max;
          EEPROM.write(eepMotor[motor_number].MAX[3], temp_max%256);
          temp_max /= 256;
          EEPROM.write(eepMotor[motor_number].MAX[2], temp_max%256);
          temp_max /= 256;
          EEPROM.write(eepMotor[motor_number].MAX[1], temp_max%256);
          temp_max /= 256;
          EEPROM.write(eepMotor[motor_number].MAX[0], temp_max%256);
        }else if(command.equalsIgnoreCase("run")){
          pinValues = read_shift_regs();
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
          builtin_pulse_add[motor_number]   = json["add"];
        }else if(command.equalsIgnoreCase("status")){
          Serial.print("builtin");
          Serial.print(motor_number);
          Serial.print(":");
          driver[motor_number].status();
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
  String json = "{\"ID\":\""+ String(AIO_Subscribe) +",\"ctrl\":\"receive\",\"cmd\":\""+control+",\"opt\":\""+command+"\"}";
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
    pinValues = read_shift_regs();

    String json = "{\"id\":\"";
    json += AIO_Subscribe;
    json += "\",\"ctrl\":\"sensor\",\"cmd\":\"read\",\"sensor\":[";

    for(int index = 0; index < DATA_WIDTH; index++)
    {
      json += (pinValues >> index) & 1;
      if(index != DATA_WIDTH-1) json += ",";
    }
    json += "]}";
    
    char buffer[json.length() + 1];
    json.toCharArray(buffer, json.length() + 1);
    mqtt_response(buffer);
}

void set_pin_values(){
    pinValues = read_shift_regs();

    String json = "{\"id\":\"";
    json += AIO_Subscribe;
    json += "\",\"ctrl\":\"sensor\",\"cmd\":\"set\",\"sensor\":[";

    for(int index = 0; index < DATA_WIDTH; index++)
    {
      json += SENSOR_ON[index];
      if(index != DATA_WIDTH-1) json += ",";
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
  for (uint8_t index = 1; index < 25; index++){
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
  for (uint8_t index=0 ; index<DRIVER_O; index++) {
    driver[index].init(stepDriver[index]);
  }
  for (uint8_t index=0 ; index<DRIVER_I; index++) {
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
  SENSOR_ON[DATA_WIDTH] = true;
  for (uint8_t index = 0; index < DATA_WIDTH; index++){
    uint8_t sensor_on = EEPROM.read(EEP_SENSOR_ON[index]);
    if(sensor_on == 0){SENSOR_ON[index] = false;}
    else{SENSOR_ON[index] = true;}
    #ifdef DEBUG
      Serial.print("sensor");Serial.print(index);
      Serial.print(" on state is ");
      Serial.println(SENSOR_ON[index]);
    #endif
  }
  for (uint8_t index = 0; index < DRIVER_O; index++){
    uint16_t temp_accel = EEPROM.read(eepDriver[index].ACCEL[0])*256 + EEPROM.read(eepDriver[index].ACCEL[1]);
    uint16_t temp_decel = EEPROM.read(eepDriver[index].DECEL[0])*256 + EEPROM.read(eepDriver[index].DECEL[1]);
    uint16_t temp_dla_s = EEPROM.read(eepDriver[index].DLY_S[0])*256 + EEPROM.read(eepDriver[index].DLY_S[1]);
    uint16_t temp_dla_l = EEPROM.read(eepDriver[index].DLY_L[0])*256 + EEPROM.read(eepDriver[index].DLY_L[1]);
    BRAKE_O[index]      = EEPROM.read(eepDriver[index].BRAKE);
    uint32_t temp_max[4] = {EEPROM.read(eepDriver[index].MAX[0]),EEPROM.read(eepDriver[index].MAX[1]),EEPROM.read(eepDriver[index].MAX[2]),EEPROM.read(eepDriver[index].MAX[3])}; 
    HIGHT_MAX_O[index]  = temp_max[0]*256*256*256 + temp_max[1]*256*256 + temp_max[2]*256 + temp_max[3];
    driver[index].set_config(temp_accel, temp_decel, temp_dla_s, temp_dla_l);
    #ifdef DEBUG
      Serial.print("DRIVER "); Serial.print(index+1);
      Serial.print(", accel:");Serial.print(temp_accel);
      Serial.print(", decel:");Serial.print(temp_decel);
      Serial.print(", d_shot:");Serial.print(temp_dla_s);
      Serial.print(", d_long:");Serial.print(temp_dla_l);
      Serial.print(", max:");Serial.print(HIGHT_MAX_O[index]);
      Serial.print(", brake:");Serial.print(BRAKE_O[index]);
      Serial.println(".");
    #endif
  }
  for (uint8_t index = 0; index < DRIVER_I; index++){
    uint16_t temp_accel = EEPROM.read(eepMotor[index].ACCEL[0])*256 + EEPROM.read(eepMotor[index].ACCEL[1]);
    uint16_t temp_decel = EEPROM.read(eepMotor[index].DECEL[0])*256 + EEPROM.read(eepMotor[index].DECEL[1]);
    uint16_t temp_dla_s = EEPROM.read(eepMotor[index].DLY_S[0])*256 + EEPROM.read(eepMotor[index].DLY_S[1]);
    uint16_t temp_dla_l = EEPROM.read(eepMotor[index].DLY_L[0])*256 + EEPROM.read(eepMotor[index].DLY_L[1]);
    BRAKE_I[index]      = EEPROM.read(eepMotor[index].BRAKE);
    uint32_t temp_max[4] = {EEPROM.read(eepMotor[index].MAX[0]),EEPROM.read(eepMotor[index].MAX[1]),EEPROM.read(eepMotor[index].MAX[2]),EEPROM.read(eepMotor[index].MAX[3])}; 
    HIGHT_MAX_I[index]  = temp_max[0]*256*256*256 + temp_max[1]*256*256 + temp_max[2]*256 + temp_max[3];
    builtin[index].set_config(temp_accel, temp_decel, temp_dla_s, temp_dla_l);
    #ifdef DEBUG
      Serial.print("BUILT_IN ");Serial.print(index+1);
      Serial.print(", accel:");Serial.print(temp_accel);
      Serial.print(", decel:");Serial.print(temp_decel);
      Serial.print(", d_shot:");Serial.print(temp_dla_s);
      Serial.print(", d_long:");Serial.print(temp_dla_l);
      Serial.print(", max:");Serial.print(HIGHT_MAX_I[index]);
      Serial.print(", brake:");Serial.print(BRAKE_I[index]);
      Serial.println(".");
    #endif
  }
}//********** End Of Setup() **********//

unsigned long mqtt_req  = 0UL;
unsigned long mqtt_ping = 0UL;
//********** loop **********//
void loop() {
  if(online){
    MQTT_connect();
    unsigned long mqtt_run = millis();
    if(mqtt_run - mqtt_req > 1000){
      mqtt_req = mqtt_run;
      mqtt_requeset();
    }
    if(mqtt_run - mqtt_ping > 1000*10){
      mqtt_ping = mqtt_run;
      #ifdef DEBUG
        Serial.println("Ping");
      #endif
      if (!mqtt.ping()) mqtt.disconnect();
    }
  }
  if (Serial.available()) Serial_process();
  
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
      pinValues = read_shift_regs();
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
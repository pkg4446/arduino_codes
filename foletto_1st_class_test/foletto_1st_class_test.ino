#include "pin_setup.h"
#include "shift_regs.h"
#include "moter_control.h"

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
//#define DEBUG_SHIFT_REGS

/************************* Ethernet Client Setup *****************************/
//byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x54};

IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);
EthernetClient client;
/************************* Mqtt Server Setup *********************************/
#define AIO_SERVER      "mqtt.kro.kr"
#define AIO_SERVER_PORT  1883
#define AIO_USERNAME    "test"
#define AIO_KEY         "test"

const char* AIO_Publish   = "arduino_test_p";
const char* AIO_Subscribe = "arduino_test_s";
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVER_PORT, AIO_USERNAME, AIO_KEY);
// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }
// Setup a mqtt 
Adafruit_MQTT_Publish   response = Adafruit_MQTT_Publish(&mqtt,   AIO_Publish);
Adafruit_MQTT_Subscribe request  = Adafruit_MQTT_Subscribe(&mqtt, AIO_Subscribe);
/************************* Mqtt End *********************************/

/************************* values *********************************/
MOTOR driver[4];
MOTOR builtin[4];
unsigned long relay_start_time[7] = {0UL,};
unsigned long relay_end_time[7]   = {0UL,};
bool relay_state[7];
bool online = false;
/************************* values *********************************/


/******
// step motor ctrl request (사용 전, 후, relay status change로 반드시 브레이크 off, on)
{
    "ctrl":    "motor",
    "driver":  1,   //0내장 드라이버, 1외장드라이버(MD5-HD14)
    "type":    1,   // motor type e.g. 1번 모터 // 0 to 255
    "step":    10,  //스탭 수 (n 스탭당 1회전) unsigned int32 - 0 to 4294967295        
    "dir":     1,   //step motor direction (CW/CCW), 0:CW, 1:CCW
    "limit":   4,   //모터 진행방향 리미트센서 번호 = sensor_read 번호
    "accel":   10,  //0 to 45, 가속 구간 % 비율
    "decel":   5,   //0 to 45, 감속 구간 % 비율
    "spd_max": 10,  //unsigned int 16, 모터 스탭간격 micro sec 
    "spd_min": 1000 //unsigned int 16, 모터 스탭간격 micro sec, speed_max < speed_min    
}
*******/
/*************/
uint8_t  shift_read       = 0;
bool     builtin_run[6]   = {false,};
bool     builtin_dir[6]   = {false,};
uint32_t builtin_pulse[6] = {0,};
uint32_t builtin_pulse_start[6] = {0,};
uint32_t builtin_pulse_end[6]   = {0,};
uint8_t  builtin_limit[6] = {16,};
uint8_t  builtin_speed[6] = {0L,};
float  builtin_speed_f[6] = {0.00f,};
float  builtin_speed_accel[6] = {0.00f,};
float  builtin_speed_decel[6] = {0.00f,};
unsigned long builtin_interval[6] = {0L,};

void builtin_stepper(){
  BYTES_VAL_T pinValues;
  bool builtin_progress = false;
  for(uint8_t index=0; index<6; index++){
    if(builtin_run[index]) builtin_progress = true;
  }
  if(builtin_progress){
    digitalWrite(BUITIN_EN, true);
    unsigned long builtin_time = millis();
    if(shift_read++ > 10){
      pinValues   = read_shift_regs();
      shift_read  = 0;
    }
    for(uint8_t index=0; index<6; index++){
      if(builtin_run[index] && (builtin_time - builtin_interval[index] > builtin_speed[index])){
        if(!swich_values(builtin_limit[index], pinValues)){
          digitalWrite(stepMotor[index].PWM, true);
        }else{
          builtin_run[index] = false;
          response_moter_status("motor", 0, index +1, builtin[index].get_zero_set(), builtin[index].get_pos(), builtin[index].get_max());
          //종료
        }
      }
    }
    for(uint8_t index=0; index<6; index++){
      if(builtin_run[index]){
        digitalWrite(stepMotor[index].PWM, false);
        builtin[index].pos_update(builtin_dir[index]);
        builtin_pulse[index]--;

         if(builtin_pulse[index] <= 1){
          builtin_run[index] = false;
          response_moter_status("motor", 0, index +1, builtin[index].get_zero_set(), builtin[index].get_pos(), builtin[index].get_max());
        }
        /*
        else if(builtin_pulse[index] > builtin_pulse_start[index]){
          builtin_speed_f[index] -= builtin_speed_accel[index];
          builtin_speed[index] = builtin_speed_f[index];
        }else if(builtin_pulse[index] < builtin_pulse_end[index]){
          builtin_speed_f[index] += builtin_speed_decel[index];
          builtin_speed[index] = builtin_speed_f[index];
        }
        */
      }
    }
  }else{
    digitalWrite(BUITIN_EN, false);
  }
}
/***************/

void init_port_base(){
  for (uint8_t index=0 ; index<7; index++) {
    pinMode(relay_pin[index],OUTPUT);
    digitalWrite(relay_pin[index], false);
    relay_state[index] = false;
  }
}

char    Serial_buf[JSON_STACK+1];
uint8_t Serial_num = 0;

void Serial_process() {
  char ch;
  ch = Serial.read();
  if(ch=='}'){
    Serial_buf[Serial_num] = '}';
    Serial_buf[Serial_num+1] = 0x00;
    commend_pros(Serial_buf);
    Serial_num = 0;
  }else if(ch=='{'){
    Serial_num = 0;
    Serial_buf[Serial_num++]='{';
  }else{
    Serial_buf[ Serial_num++ ] = ch;
    Serial_num %= JSON_STACK;
  }
}

//**********MQTT**********//
void MQTT_connect() {
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  int8_t ret;
  Serial.println("Connecting to MQTT.");
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 1 seconds.");
    mqtt.disconnect();
    delay(1000);  // wait 1 seconds
  }
  Serial.println("MQTT Connected!");
}

void mqtt_requeset(){
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(1))) {
    if (subscription == &request) {
      String receive = (char *)request.lastread;
      commend_pros(receive);
    }//subscription
  }//while
}//mqtt_requeset()

void commend_pros(String receive){
  #ifdef DEBUG_MQTT
    Serial.print("MQTT: ");
    Serial.println(receive);
  #endif
  StaticJsonDocument<JSON_STACK> json;
  deserializeJson(json, receive);
  String ctrl  = json["ctrl"];
  mqtt_receive(ctrl);
  if(ctrl.equalsIgnoreCase("relay")){
    int8_t type = json["type"];
    if(type<1 || type>7){
      mqtt_err_msg("relay","select wrong");
    }else{
      type -= 1;
      int16_t duration  = json["duration"];
      relay_state[type] = true;
      digitalWrite(relay_pin[type], true);
      unsigned long sys_time = millis();
      relay_start_time[type] = sys_time;
      relay_end_time[type]   = sys_time + duration;
    }
  }else if(ctrl.equalsIgnoreCase("relay_hold")){
    int8_t type = json["type"];
    if(type<1 || type>7){
      mqtt_err_msg("relay","select wrong");
    }else{
      type -= 1;
      bool status = json["status"];
      digitalWrite(relay_pin[type], status);
      relay_status_change(type+1, status);
    }
  }else if(ctrl.equalsIgnoreCase("sensor_read")){
    read_pin_values();
  }else if(ctrl.equalsIgnoreCase("motor")){

    bool relay_busy = false;
    for(uint8_t index = 0; index < 7; index++){
      if(relay_state[index]) relay_busy = true;
    }
    
    bool drive  = json["driver"];
    
    if(drive){ //(MD5-HD14)
      uint8_t type  = json["type"];
      if(relay_busy){
        mqtt_err_msg("motor","relay is busy");
      }else if(type<1 || type>4){
        mqtt_err_msg("motor","select wrong");
      }else{
        type -= 1;
        driver[type].run_drive(json["dir"],json["limit"],json["step"],json["accel"],json["decel"],json["spd_max"],json["spd_min"]);
        response_moter_status("motor", drive, type +1, driver[type].get_zero_set(), driver[type].get_pos(), driver[type].get_max());
      }
    }else{
      uint8_t type  = json["type"];
      if(type<1 || type>6){
        mqtt_err_msg("motor","select wrong");
      }else{
        type -= 1;
        builtin_dir[type]   = json["dir"];
        digitalWrite(stepMotor[type].DIR, builtin_dir[type]);
        builtin_run[type]   = true;
        builtin_pulse[type] = json["step"];
        builtin_limit[type] = json["limit"];
        uint16_t spd_max = json["spd_max"];
        uint16_t spd_min = json["spd_min"];
        uint16_t spd_gap = spd_min - spd_max;
        builtin_speed[type] = spd_min;

        uint8_t accel = json["accel"];
        uint8_t decel = json["decel"];
        builtin_pulse_start[type] = builtin_pulse[type]*(accel/100.0);
        builtin_pulse_end[type]   = builtin_pulse[type]*(decel/100.0);

        builtin_speed_accel[type] = float(spd_gap)/float(builtin_pulse_start[type]);
        builtin_speed_decel[type] = float(spd_gap)/float(builtin_pulse_end[type]);

        builtin_pulse_start[type] = builtin_pulse[type] - builtin_pulse_start[type];
      }
    }
    
  }else if(ctrl.equalsIgnoreCase("motor_config")){
    bool drive    = json["driver"];
    bool method   = json["method"];
    uint8_t type  = json["type"];
    uint32_t step;;
    if(method){
      step = json["step"];
    }
    if(type<1 || type>4){
      mqtt_err_msg("motor","select wrong");
    }else if(drive){//MD5-HD14
      type -= 1;
      if(method)  driver[type].set_maximum(step);
      response_moter_status("motor_config", drive, type +1, driver[type].get_zero_set(), driver[type].get_pos(), driver[type].get_max());
    }else{
      if(method)  builtin[type].set_maximum(step);
      response_moter_status("motor_config", drive, type +1, builtin[type].get_zero_set(), builtin[type].get_pos(), builtin[type].get_max());
    }
  }else{
    Serial.println("nope");
    mqtt_err_msg("null","commend error");
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
  res["type"] = type;
  res["err"]  = error_msg;
  
  String json="";
  serializeJson(res, json);
  char buffer[json.length() + 1];
  json.toCharArray(buffer, json.length() + 1);
  mqtt_response(buffer);
}

void mqtt_receive(String type){
  DynamicJsonDocument res(JSON_STACK);
  res["id"]   = AIO_Subscribe;
  res["ctrl"] = "receive";
  res["type"] = type;
  
  String json="";
  serializeJson(res, json);
  char buffer[json.length() + 1];
  json.toCharArray(buffer, json.length() + 1);
  mqtt_response(buffer);
}
//**********End Of MQTT**********//
void response_moter_status(String ctrl, bool drive, uint8_t type, bool zero, uint32_t pos, uint32_t max){
  DynamicJsonDocument res(JSON_STACK);
  res["id"]     = AIO_Subscribe;
  res["ctrl"]   = ctrl;
  res["driver"] = drive;
  res["zero"]   = zero;
  res["pos"]    = pos;
  res["max"]    = max;

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
  res["id"]     = AIO_Subscribe;
  res["ctrl"]   = "relay_hold";
  res["type"]   = index;
  res["status"] = status;

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
      res["type"] = index+1;
      res["duration"] = uint16_t(sys_time - relay_start_time[index]);

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
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
    
    online = true;
    mqtt.subscribe(&request);
  }

  shift_regs_init();
  for (uint8_t index=0 ; index<4; index++) {
    driver[index].init(stepDriver[index]);
    driver[index].status();
  }
  pinMode(BUITIN_EN, OUTPUT);
  for (uint8_t index=0 ; index<6; index++) {
    builtin[index].init(stepMotor[index]);
    builtin[index].status();
  }
  init_port_base();
}//********** End Of Setup() **********//

//********** loop **********//
void loop() {
  //if need asynchronous, add something
  if(online){
    MQTT_connect();
    mqtt_requeset();
  }else if (Serial.available()) {
    Serial_process();
  }
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
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
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
/************************* Mqtt Server Setup *********************************/
#define AIO_SERVER      "mqtt.kro.kr"
#define AIO_SERVER_PORT  1883
#define AIO_USERNAME    "test"
#define AIO_KEY         "test"

const char* AIO_Publish   = "arduino_test_p";
const char* AIO_Subscribe = "arduino_test_s";
//Set up the ethernet client
EthernetClient client;
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
/************************* values *********************************/

void init_port_base(){
  for (uint8_t index=0 ; index<7; index++) {
    pinMode(relay_pin[index],OUTPUT);
    digitalWrite(relay_pin[index], false);
    relay_state[index] = false;
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
        if(relay_busy){
          mqtt_err_msg("motor","relay busy");
        }else{          
          bool drive  = json["driver"];
          if(drive){//MD5-HD14
            uint8_t type  = json["type"];
            if(type<1 || type>4){
              mqtt_err_msg("motor","select wrong");
            }else{
              type -= 1;
              driver[type].run_drive(json["dir"],json["limit"],json["step"],json["accel"],json["decel"],json["spd_max"],json["spd_min"]);
              response_moter_status("motor", drive, type +1, driver[type].get_zero_set(), driver[type].get_pos(), driver[type].get_max());
            }
          }else{
            mqtt_err_msg("motor","dosen't supported yet");
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
    }//subscription
  }//while
}//mqtt_requeset()

void mqtt_response(const char* send_data){
  if (! response.publish(send_data)) {
    Serial.println("Failed");
    mqtt_response(send_data);
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
  Ethernet.begin(mac);
  shift_regs_init();
  for (uint8_t index=0 ; index<4; index++) {
    driver[index].init(stepDriver[index]);
    driver[index].status();
  }
  for (uint8_t index=0 ; index<6; index++) {
    builtin[index].init(stepMotor[index]);
    builtin[index].status();
  }
  init_port_base();
  mqtt.subscribe(&request);
}//********** End Of Setup() **********//

//********** loop **********//
void loop() {
  //if need asynchronous, add something
  MQTT_connect();
  mqtt_requeset();
  relay_off_awiat();
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
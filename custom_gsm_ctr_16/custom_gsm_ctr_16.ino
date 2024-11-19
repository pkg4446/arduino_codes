//ze03센서 추가
#include <EEPROM.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient
#include <DS3231.h>
#include <max6675.h>
#include <Adafruit_SHT31.h>
#include <SoftwareSerial.h>//https://github.com/plerup/espsoftwareserial/
#include "PCA9555.h"
#include "uart_print.h"

SoftwareSerial ZE03serial;
#define ZE03_RX 25
#define ZE03_TX 26

#define TOTAL_RELAY 16
#define EEPROM_SIZE_CONFIG  24
#define EEPROM_SIZE_VALUE   3
#define EEPROM_SIZE_CTR     16
#define COMMAND_LENGTH      32
#define UPDATE_INTERVAL     1000L
#define ZE03_CMD_LENGTH     9

PCA9555 ioport(0x20);

const uint8_t thermoDO[4] = {32,33,26,27};
const uint8_t thermoCS    = 15;
const uint8_t thermoCLK   = 14;
const uint8_t pin_led     = 25;

MAX6675 thermocouple1(thermoCLK, thermoCS, thermoDO[0]);
MAX6675 thermocouple2(thermoCLK, thermoCS, thermoDO[1]);
MAX6675 thermocouple3(thermoCLK, thermoCS, thermoDO[2]);
MAX6675 thermocouple4(thermoCLK, thermoCS, thermoDO[3]);
Adafruit_SHT31 sht31 = Adafruit_SHT31();

int16_t temp_water    = 0;
int16_t temp_liquid   = 0;
int16_t temp_air      = 0;
int16_t temp_out      = 0;

DS3231 RTC_DS3231;
HardwareSerial nxSerial(2);
bool    nextion_shift = false;
uint8_t nextion_page  = 0;
bool    serial_uart   = false;
bool    wing_state    = false;
enum RelayFunc {
    Lamp_1 = 0,
    Lamp_2,
    Lamp_3,
    Lamp_4,
    Line_1,
    Line_2,
    Liquid_1,
    Liquid_2,
    Heater_1,
    Heater_2,
    Cooler,
    Heater,
    Fan_A,
    Fan_B,
    Wing_A,
    Wing_B
};
const String server = "http://gsm.routes.kro.kr/";
/***************EEPROM*********************/
const uint8_t eep_ssid[EEPROM_SIZE_CONFIG] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
const uint8_t eep_pass[EEPROM_SIZE_CONFIG] = {24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47};
const uint8_t eep_var[EEPROM_SIZE_VALUE*EEPROM_SIZE_CTR] = {48,49,50, 51,52,53, 54,55,56,
                                                            57,58,59, 60,61,62, 63,64,65,
                                                            66,67,68, 69,70,71, 72,73,74,
                                                            75,76,77, 78,79,80, 81,82,83,
                                                            84,85,86, 87,88,89, 90,91,92,
                                                            93,94,95};
/***************EEPROM*********************/
/***************MQTT_CONFIG****************/
const char*     mqttServer    = "gsm.routes.kro.kr";
const uint16_t  mqttPort      = 1883;
const char*     mqttUser      = "gsmkr";
const char*     mqttPassword  = "gsmkrmqtt";
const char*     topic_pub     = "SHS";

char            sendID[21]    = "ID=";
char            deviceID[18];

WiFiClient    mqtt_client;
PubSubClient  mqttClient(mqtt_client);
uint8_t       mqtt_count      = 0;
/***************MQTT_CONFIG****************/
/***************Interval_timer*************/
unsigned long prevUpdateTime = 0L;
unsigned long prevPageChange = 0L;
unsigned long prePpdatePost  = 0L;

uint8_t       update_order   = 0;
/***************Interval_timer*************/
uint8_t lquid_gap = 3;
/***************Variable*******************/
char    ssid[EEPROM_SIZE_CONFIG];
char    password[EEPROM_SIZE_CONFIG];
/***************Variable*******************/
typedef struct ctr_var{
    bool    enable;
    bool    state;
    uint8_t run;
    uint8_t stop;
}ctr_var;
/***************Variable*******************/
bool      wifi_able;

bool      relay_state[TOTAL_RELAY] = {false,};
ctr_var   iot_ctr[EEPROM_SIZE_CTR];
uint16_t  water_ctr_time[2]   = {0,};
uint16_t  fan_ctr_time[2]   = {0,};
/*
ctr_var water[2];   // run:동작_초, stop:정지_분
ctr_var lamp[3];    // run:시작시간,stop:정지시간
ctr_var temp_ctr;   // run:목표값,  stop:허용치
ctr_var circulate;  // run:동작_분, stop:정지_분
//0:wing    state
//1:heat    state
//2:fan in  state
//3:fan out state
*/
//ZE03 측정
uint8_t  ZE03[ZE03_CMD_LENGTH] = {0,};
uint8_t  ZE03Index = 0;
uint16_t ze03_value = 0;
/***************Variable*******************/
char    command_buf[COMMAND_LENGTH];
int8_t  command_num;
/***************Functions******************/
void relay_ctr(uint8_t num_relay, bool status_relay){
  ioport.digitalWrite(num_relay, status_relay);
  relay_state[num_relay] = status_relay;
  if(nextion_page == 2) nextion_display("page_manu.bt"+String(num_relay),status_relay,&nxSerial);
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  for (int index = 0; index < length; index++) {
    command_buf[index] = payload[index];
  }
  command_num = 0;
  Serial.println("MQTT");
  command_service();
}
/******************************************/
void WIFI_scan(bool wifi_state){
  wifi_able = wifi_state;
  WiFi.disconnect(true);
  nextion_print(&nxSerial,"page 1");//nextion page 이동
  nextion_page == 1;
  nextion_print(&nxSerial,"page_wifi.t_wifi.txt=\"WIFI scanning...\"");
  if(serial_uart) Serial.println("WIFI Scanning…");
  uint8_t networks = WiFi.scanNetworks();
  if (networks == 0) {
    nextion_print(&nxSerial,"page_wifi.t_wifi.txt=\"WIFI not found!\"");
    if(serial_uart) Serial.println("WIFI not found!");
  }else {
    nextion_print(&nxSerial,"page_wifi.t_wifi.txt=\"<= WIFI list are here!\\rscroll & select\"");
    if(serial_uart){
      Serial.print(networks);
      Serial.println(" networks found");
      Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    }
    String wifi_list ="";
    for (int index = 0; index < networks; ++index) {
      // Print SSID and RSSI for each network found
      if(serial_uart){
        Serial.printf("%2d",index + 1);
        Serial.print(" | ");
        Serial.printf("%-32.32s", WiFi.SSID(index).c_str());
        Serial.print(" | ");
        Serial.printf("%4d", WiFi.RSSI(index));
        Serial.print(" | ");
        Serial.printf("%2d", WiFi.channel(index));
        Serial.print(" | ");
        byte wifi_type = WiFi.encryptionType(index);
        String wifi_encryptionType;
        if(wifi_type == WIFI_AUTH_OPEN){wifi_encryptionType = "open";}
        else if(wifi_type == WIFI_AUTH_WEP){wifi_encryptionType = "WEP";}
        else if(wifi_type == WIFI_AUTH_WPA_PSK){wifi_encryptionType = "WPA";}
        else if(wifi_type == WIFI_AUTH_WPA2_PSK){wifi_encryptionType = "WPA2";}
        else if(wifi_type == WIFI_AUTH_WPA_WPA2_PSK){wifi_encryptionType = "WPA2";}
        else if(wifi_type == WIFI_AUTH_WPA2_ENTERPRISE){wifi_encryptionType = "WPA2-EAP";}
        else if(wifi_type == WIFI_AUTH_WPA3_PSK){wifi_encryptionType = "WPA3";}
        else if(wifi_type == WIFI_AUTH_WPA2_WPA3_PSK){wifi_encryptionType = "WPA2+WPA3";}
        else if(wifi_type == WIFI_AUTH_WAPI_PSK){wifi_encryptionType = "WAPI";}
        else{wifi_encryptionType = "unknown";}
        if(serial_uart) Serial.println(wifi_encryptionType);
      }
      if(nextion_page == 1){
        wifi_list += WiFi.SSID(index);
        if(index<networks-1)wifi_list += "\\r";
        if(index<10) nextion_print(&nxSerial,"page_wifi.wifi"+String(index)+".txt=\""+WiFi.SSID(index)+"\"");
      }
    }
    if(serial_uart) Serial.println("");
    if(nextion_page == 1) nextion_print(&nxSerial,"page_wifi.list.path=\""+wifi_list+"\"");
  }

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();
  if(wifi_able){
    wifi_connect();
  }
}
/******************************************/
/******************************************/
void command_service(){
  String cmd_text     = "";
  String temp_text    = "";
  bool   eep_change   = false;
  uint8_t check_index = 0;
  
  for(uint8_t index_check=0; index_check<COMMAND_LENGTH; index_check++){
    if(command_buf[index_check] == 0x20 || command_buf[index_check] == 0x00){
      check_index = index_check+1;
      break;
    }
    cmd_text += command_buf[index_check];
  }
  for(uint8_t index_check=check_index; index_check<COMMAND_LENGTH; index_check++){
    if(command_buf[index_check] == 0x20 || command_buf[index_check] == 0x00){
      check_index = index_check+1;
      break;
    }
    temp_text += command_buf[index_check];
  }
  /**********/
  Serial.print("cmd: ");
  Serial.println(command_buf);

  if(cmd_text=="time"){
    time_show();
  }else if(cmd_text=="timeset"){
    if(wifi_able) time_set();
    else Serial.println("wifi not connected");
  }else if(cmd_text=="manual"){
    uint8_t relay_num = temp_text.toInt();
    if(relay_num < TOTAL_RELAY){
      String cmd_select = "";
      for(uint8_t index_check=check_index; index_check<COMMAND_LENGTH; index_check++){
        if(command_buf[index_check] == 0x20 || command_buf[index_check] == 0x00){
          check_index = index_check+1;
          break;
        }
        cmd_select += command_buf[index_check];
      }
      if(cmd_select == "on")  relay_ctr(relay_num, true);
      else  relay_ctr(relay_num, false);
    }
  }else if(cmd_text=="reboot"){
    ESP.restart();
  }else if(cmd_text=="page"){
    nextion_shift = true;
    nextion_page  = temp_text.toInt();
  }else if(cmd_text=="send"){
    nextion_print(&nxSerial,temp_text);
  }else if(cmd_text=="wing"){
    if(temp_text == "on") wing_state = true;
    else                  wing_state = false;
    nextion_print(&nxSerial,"page 0");
    relay_ctr(Wing_A, wing_state);
    relay_ctr(Wing_B, wing_state);
    nextion_display("page_main.bt_wing",wing_state,&nxSerial);
  }else if(cmd_text=="set"){
    uint8_t iot_ctr_type = 255;
    if(temp_text=="line_a"){iot_ctr_type=Line_1;}
    else if(temp_text=="line_b"){iot_ctr_type=Line_2;}
    else if(temp_text=="water"){iot_ctr_type=Liquid_1;}
    else if(temp_text=="liquid"){iot_ctr_type=Liquid_2;}
    else if(temp_text=="wat_h"){iot_ctr_type=Heater_1;}
    else if(temp_text=="liq_h"){iot_ctr_type=Heater_2;}
    else if(temp_text=="lamp_a"){iot_ctr_type=Lamp_1;}
    else if(temp_text=="lamp_b"){iot_ctr_type=Lamp_2;}
    else if(temp_text=="lamp_c"){iot_ctr_type=Lamp_3;}
    else if(temp_text=="lamp_d"){iot_ctr_type=Lamp_4;}
    else if(temp_text=="temp"){iot_ctr_type=Cooler;}
    else if(temp_text=="circul_i"){iot_ctr_type=Fan_A;}
    else if(temp_text=="circul_o"){iot_ctr_type=Fan_B;}
    if(iot_ctr_type != 255){
      eep_change = true;
      String cmd_select = "";
      for(uint8_t index_check=check_index; index_check<COMMAND_LENGTH; index_check++){
        if(command_buf[index_check] == 0x20 || command_buf[index_check] == 0x00){
          check_index = index_check+1;
          break;
        }
        cmd_select += command_buf[index_check];
      }
      String cmd_value = "";
      for(uint8_t index_check=check_index; index_check<COMMAND_LENGTH; index_check++){
        if(command_buf[index_check] == 0x20 || command_buf[index_check] == 0x00){
          check_index = index_check+1;
          break;
        }
        cmd_value += command_buf[index_check];
      }
      
      if(cmd_select=="ena"){
        iot_ctr[iot_ctr_type].enable = cmd_value.toInt();
        EEPROM.write(eep_var[iot_ctr_type*3], iot_ctr[iot_ctr_type].enable);
      }else if(cmd_select=="run"){
        iot_ctr[iot_ctr_type].run    = cmd_value.toInt();
        EEPROM.write(eep_var[iot_ctr_type*3+1], iot_ctr[iot_ctr_type].run);
      }else if(cmd_select=="stp"){
        iot_ctr[iot_ctr_type].stop   = cmd_value.toInt();
        EEPROM.write(eep_var[iot_ctr_type*3+2], iot_ctr[iot_ctr_type].stop);
      }
      if(serial_uart){
        Serial.print("enable: ");Serial.print(iot_ctr[iot_ctr_type].enable);
        Serial.print(", run :");Serial.print(iot_ctr[iot_ctr_type].run);
        Serial.print(", stop :");Serial.println(iot_ctr[iot_ctr_type].stop);
      }
      String config_post = "{\"DEVID\":\""+String(deviceID)+"\",";
      config_post += "\"cmd\":\""+String(temp_text)+"\",";
      config_post += "\"type\":\""+String(cmd_select)+"\",";
      config_post += "\"val\":"+String(cmd_value)+"}";
      String response = httpPOSTRequest(server+"device/config",config_post);
      if(iot_ctr_type<=Lamp_4)        nextion_print(&nxSerial,"page 6"); //LED 페이지
      else if(iot_ctr_type<=Heater_2) nextion_print(&nxSerial,"page 4"); //양액 페이지
      else if(iot_ctr_type<=Heater)   nextion_print(&nxSerial,"page 5");
      else if(iot_ctr_type<=Fan_B)    nextion_print(&nxSerial,"page 7");
    }
  }else if(cmd_text=="debug"){
    if(temp_text == "off"){
      serial_uart = false;
    }else{
      serial_uart = true;
    }
  }else if(cmd_text=="config"){
    uint8_t iot_ctr_type = 255;
    if(temp_text=="line_a"){iot_ctr_type=Line_1;}
    else if(temp_text=="line_b"){iot_ctr_type=Line_2;}
    else if(temp_text=="water"){iot_ctr_type=Liquid_1;}
    else if(temp_text=="liquid"){iot_ctr_type=Liquid_2;}
    else if(temp_text=="lamp_a"){iot_ctr_type=Lamp_1;}
    else if(temp_text=="lamp_b"){iot_ctr_type=Lamp_2;}
    else if(temp_text=="lamp_c"){iot_ctr_type=Lamp_3;}
    else if(temp_text=="lamp_d"){iot_ctr_type=Lamp_4;}
    else if(temp_text=="heat_a"){iot_ctr_type=Heater_1;}
    else if(temp_text=="heat_b"){iot_ctr_type=Heater_2; }
    else if(temp_text=="temp"){iot_ctr_type=Cooler;}
    else if(temp_text=="circul"){iot_ctr_type=Fan_A;}
    if(iot_ctr_type != 255){
      Serial.print("enable: ");Serial.print(iot_ctr[iot_ctr_type].enable);
      Serial.print(", run :");Serial.print(iot_ctr[iot_ctr_type].run);
      Serial.print(", stop :");Serial.println(iot_ctr[iot_ctr_type].stop);
    }else{
      Serial.print("temp_water:\t");Serial.println(temp_water);
      Serial.print("temp_liquid:\t");Serial.println(temp_liquid);
      Serial.print("temp_air:\t");Serial.println(temp_air);
      Serial.print("temp_out:\t");Serial.println(temp_out);
      for (int index = 0; index < EEPROM_SIZE_CTR; index++) {
        Serial.print("config no.");Serial.print(index);
        Serial.print(", enable: ");Serial.print(iot_ctr[index].enable);
        Serial.print(", run :");Serial.print(iot_ctr[index].run);
        Serial.print(", stop :");Serial.println(iot_ctr[index].stop);
      }
    }
  }else if(cmd_text=="ssid"){
    wifi_able = false;
    WiFi.disconnect(true);
    if(serial_uart) Serial.print("ssid: ");
    if(temp_text.length() > 0){
      for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
        if(index < temp_text.length()){
          if(serial_uart) Serial.print(temp_text[index]);
          ssid[index] = temp_text[index];
          EEPROM.write(eep_ssid[index], byte(temp_text[index]));
        }else{
          ssid[index] = 0x00;
          EEPROM.write(eep_ssid[index], byte(0x00));
        }
      }
      eep_change = true;
    }
    if(serial_uart) Serial.println("");
  }else if(cmd_text=="pass"){
    wifi_able = false;
    WiFi.disconnect(true);
    if(serial_uart) Serial.print("pass: ");
    if(temp_text.length() > 0){
      for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
        if(index < temp_text.length()){
          if(serial_uart) Serial.print(temp_text[index]);
          password[index] = temp_text[index];
          EEPROM.write(eep_pass[index], byte(temp_text[index]));
        }else{
          password[index] = 0x00;
          EEPROM.write(eep_pass[index], byte(0x00));
        }
      }
      eep_change = true;
    }
    if(serial_uart) Serial.println("");
  }else if(cmd_text=="wifi"){
    if(temp_text=="stop"){
      wifi_able = false;
      if(serial_uart) Serial.print("WIFI disconnect");
      WiFi.disconnect(true);
    }else if(temp_text=="scan"){
      WIFI_scan(WiFi.status() == WL_CONNECTED);
    }else{
      wifi_connect();
    }
  }else if(cmd_text=="temp"){
    Serial.print("CPU: ");Serial.print(RTC_DS3231.getTemperature(), 2);Serial.println("°C");
    Serial.print("PT100 1: ");Serial.print(thermocouple1.readCelsius());Serial.println("°C");
    Serial.print("PT100 2: ");Serial.print(thermocouple2.readCelsius());Serial.println("°C");
    Serial.print("PT100 3: ");Serial.print(thermocouple3.readCelsius());Serial.println("°C");
    Serial.print("PT100 4: ");Serial.print(thermocouple4.readCelsius());Serial.println("°C");
  }else if(cmd_text=="help"){
    serial_command_help(&Serial);
  }else{ serial_err_msg(&Serial, command_buf); }
  if(eep_change){ EEPROM.commit(); }
}
uint8_t error_index = 0;

void command_process(char ch) {
  if(ch=='\n'){
    command_buf[command_num] = 0x00;
    command_num = 0;
    command_service();
    memset(command_buf, 0x00, COMMAND_LENGTH);
  }else if(ch==0x1A){
    error_index = 0;
  }else if(ch==0xFF){
    if(++error_index>=3){
      Serial.println("nx_err");
    }
  }else if(ch!='\r'){
    command_buf[command_num++] = ch;
    command_num %= COMMAND_LENGTH;
  }
}
/******************************************/
void mqtt_connect() {
  mqttClient.disconnect();
  if(wifi_able){
    mqttClient.setServer(mqttServer, mqttPort);
    mqttClient.setCallback(mqtt_callback);

    char* topic_sub = deviceID;
    char* sub_ID    = sendID;

    unsigned long WIFI_wait  = millis();
    bool mqtt_connected = true;
    // while (!mqttClient.connected()) {
    //   if (millis() > WIFI_wait + 1000) {
    //     WIFI_wait = millis();
    //     if (WiFi.status() != WL_CONNECTED){
    //       Serial.println("WIFI was not connected");
    //       mqttClient.disconnect();
    //       return;
    //     }else if(mqttClient.connect(deviceID, mqttUser, mqttPassword )) {
    //       Serial.println("connected");
    //     } else {
    //       Serial.print("failed with state ");
    //       Serial.println(mqttClient.state());
    //       mqtt_connected = false;
    //       break;
    //     }
    //   }
    // }
    if(!mqttClient.connected()){
      if (WiFi.status() != WL_CONNECTED){
        Serial.println("WIFI X");
        return;
      }else if(mqttClient.connect(deviceID, mqttUser, mqttPassword )) {
      } else {
        mqtt_connected = false;
      }
    }
    Serial.print("MQTT ");
    if(mqtt_connected){
      mqttClient.subscribe(topic_sub);
      mqttClient.publish(topic_pub, sub_ID);
      Serial.println(sub_ID);
    }else{      
      Serial.println(mqttClient.state());
    }
  }
}

void wifi_connect() {
  wifi_able = true;
  serial_wifi_config(&Serial,ssid,password);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  if(nextion_page==1) nextion_print(&nxSerial,"page_wifi.t_wifi.txt=\"Try to WIFI connect...\"");
  WiFi.begin(ssid, password);
  unsigned long wifi_config_update  = millis();
  while (WiFi.status() != WL_CONNECTED) {
    unsigned long update_time = millis();
    if(update_time - wifi_config_update > 5000){
      wifi_able = false;
      Serial.println("WIFI fail");
      break;
    }
  }
  if(nextion_page==0)  nextion_display("page_main.wifi",wifi_able,&nxSerial);
  else if(nextion_page==1){
    if(wifi_able) nextion_print(&nxSerial,"page_wifi.t_wifi.txt=\"WIFI connected!\"");
    else          nextion_print(&nxSerial,"page_wifi.t_wifi.txt=\"password wrong!\\rpw: "+String(password)+"\"");
  }
}

void time_show(){
  bool century = false;
  bool h12Flag;
  bool pmFlag;
  uint8_t clock_year  = RTC_DS3231.getYear();
  uint8_t clock_month = RTC_DS3231.getMonth(century);
  uint8_t clock_day   = RTC_DS3231.getDate();
  uint8_t clock_dow   = RTC_DS3231.getDoW();
  uint8_t clock_hour  = RTC_DS3231.getHour(h12Flag, pmFlag);
  uint8_t clock_min   = RTC_DS3231.getMinute();
  uint8_t clock_sec   = RTC_DS3231.getSecond();

  if(serial_uart){
    Serial.print('2');
    if(century)Serial.print('1');
    else Serial.print('0');
    if(clock_year<10) Serial.print('0');
    Serial.print(clock_year);Serial.print('/');Serial.print(clock_month);Serial.print('/');Serial.print(clock_day);Serial.print(',');
    Serial.print(clock_hour);Serial.print(':');Serial.print(clock_min);Serial.print(':');Serial.println(clock_sec);
  }
  nextion_print(&nxSerial,"rtc0=20"+String(clock_year));
  nextion_print(&nxSerial,"rtc1="+String(clock_month));
  nextion_print(&nxSerial,"rtc2="+String(clock_day));
  nextion_print(&nxSerial,"rtc3="+String(clock_hour));
  nextion_print(&nxSerial,"rtc4="+String(clock_min));
  nextion_print(&nxSerial,"rtc5="+String(clock_sec));
  nextion_print(&nxSerial,"rtc6="+String(clock_dow));
}

void time_set(){
  HTTPClient http;
  http.begin("http://www.google.com/");
  const char *headerKeys[] = {"Date"};
  const size_t headerKeysCount = sizeof(headerKeys) / sizeof(headerKeys[0]);
  http.collectHeaders(headerKeys, headerKeysCount);
  http.addHeader("Content-Type", "application/json");
  uint16_t httpResponseCode = http.GET();
  if(httpResponseCode==200){
    String server_time = http.header("Date");
    Serial.println(server_time);
    //Mon, 03 Jun 2024 01:19:55 GMT
    String  time_data[5];
    uint8_t time_index = 0;
    String  time_text = "";
    for(uint8_t index=0; index<server_time.length(); index++){
      if(server_time[index] == 0x20){
        time_data[time_index++] = time_text;
        time_text = "";
        index+=1; //passing to 0x20
        if(time_index > 4){
          time_index  = 0;
          break;
        }
      };
      time_text += server_time[index];
    }
    uint8_t time_dow,time_day,time_month,time_year,time_time[3];
    if(time_data[0]=="Mon,")      time_dow = 1;
    else if(time_data[0]=="Tue,") time_dow = 2;
    else if(time_data[0]=="Wed,") time_dow = 3;
    else if(time_data[0]=="Thu,") time_dow = 4;
    else if(time_data[0]=="Fri,") time_dow = 5;
    else if(time_data[0]=="Sat,") time_dow = 6;
    else time_dow = 7;
    time_day = (time_data[1].toInt())%100;
    if(time_data[2]=="Jan")      time_month = 1;
    else if(time_data[2]=="Feb") time_month = 2;
    else if(time_data[2]=="Mar") time_month = 3;
    else if(time_data[2]=="Apr") time_month = 4;
    else if(time_data[2]=="May") time_month = 5;
    else if(time_data[2]=="Jun") time_month = 6;
    else if(time_data[2]=="Jul") time_month = 7;
    else if(time_data[2]=="Aug") time_month = 8;
    else if(time_data[2]=="Sep") time_month = 9;
    else if(time_data[2]=="Oct") time_month = 10;
    else if(time_data[2]=="Nov") time_month = 11;
    else time_month = 12;
    time_year = (time_data[3].toInt())%100;
    for(uint8_t index=0; index<time_data[4].length(); index++){
      if(time_data[4][index] == 0x3A){
        time_time[time_index++] = (time_text.toInt())%100;
        time_text = "";
        index+=1; //passing to 0x20
      };
      time_text += time_data[4][index];
    }
    time_time[time_index] = (time_text.toInt())%100;
    time_time[0] += 9;
    if(time_time[0] >= 23){
      time_time[0] = 0;
      time_day    += 1;
      if(++time_dow > 7) time_dow = 1;
    }
    RTC_DS3231.setClockMode(false);
    RTC_DS3231.setYear(time_year);
    RTC_DS3231.setMonth(time_month);
    RTC_DS3231.setDate(time_day);
    RTC_DS3231.setDoW(time_dow);
    RTC_DS3231.setHour(time_time[0]);
    RTC_DS3231.setMinute(time_time[1]);
    RTC_DS3231.setSecond(time_time[2]);
  }else{
    Serial.println("   err");
  }
  http.end();           // Free resources
}
/******************************************/
/***************Functions******************/

void setup() {
  Serial.begin(115200);
  nxSerial.begin(115200, SERIAL_8N1, 16, 17);
  ZE03serial.begin(9600, SWSERIAL_8N1, ZE03_RX, ZE03_TX);

  ioport.begin();
  ioport.setClock(400000);
  pinMode(pin_led, OUTPUT);
  digitalWrite(pin_led, true);
  for (uint8_t index = 0; index < TOTAL_RELAY; index++)
  {
    ioport.pinMode(index, OUTPUT);
    relay_ctr(index, false);
  }

  if (!EEPROM.begin((EEPROM_SIZE_CONFIG*2) + (EEPROM_SIZE_VALUE*EEPROM_SIZE_CTR))){
    Serial.println("Failed to initialise eeprom");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
  for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
    ssid[index]     = EEPROM.read(eep_ssid[index]);
    password[index] = EEPROM.read(eep_pass[index]);
  }

  for (int index = 0; index < 17; index++) {
    if(WiFi.macAddress()[index]==':'){
      sendID[index + 3] = '_';
    }else{
      sendID[index + 3] = WiFi.macAddress()[index];
    }
    deviceID[index]   = sendID[index + 3];
  }

  for (int index = 0; index < EEPROM_SIZE_CTR; index++) {
    iot_ctr[index].state  = true;
    iot_ctr[index].enable = EEPROM.read(eep_var[3*index]);
    iot_ctr[index].run    = EEPROM.read(eep_var[3*index+1]);
    iot_ctr[index].stop   = EEPROM.read(eep_var[3*index+2]);
  }
  sht31.begin(0x44);
  wifi_connect();
  serial_command_help(&Serial);
  Serial.println("System online");
  nextion_print(&nxSerial,"page 0");
}

// the loop function runs over and over again forever
void loop() {
  if(wifi_able){
    if (mqttClient.connected()){mqttClient.loop();}
    else{mqtt_connect();}
  }
  if (Serial.available()) command_process(Serial.read());
  if (nxSerial.available()) command_process(nxSerial.read());
  const unsigned long millisec = millis();
  system_ctr(millisec);
  sensor_upload(millisec);
  page_change();
}

void page_change(){
  if(nextion_shift){
    nextion_shift = false;
    Serial.print("page "); Serial.println(nextion_page);
    if(nextion_page == 0){
      time_show();
      nextion_display("page_main.wifi",wifi_able,&nxSerial);
      for (uint8_t index = 0; index < 4; index++){
        nextion_display("page_main.sw_l"+String(index+1),iot_ctr[Lamp_1+index].enable,&nxSerial);
        nextion_display("page_main.led"+String(index+1)+"o",iot_ctr[Lamp_1+index].run,&nxSerial);
        nextion_display("page_main.led"+String(index+1)+"f",iot_ctr[Lamp_1+index].stop,&nxSerial);
      }
      for (uint8_t index = 0; index < 2; index++){
        nextion_display("page_main.sw_fw"+String(index+1),iot_ctr[Liquid_1+index].enable,&nxSerial);
        nextion_display("page_main.flow"+String(index+1)+"o",iot_ctr[Liquid_1+index].run,&nxSerial);
        nextion_display("page_main.flow"+String(index+1)+"f",iot_ctr[Liquid_1+index].stop,&nxSerial);
      }
      nextion_display("sw_t",iot_ctr[Cooler].enable,&nxSerial);
      nextion_display("tempt",iot_ctr[Cooler].run,&nxSerial);
      nextion_display("tempg",iot_ctr[Cooler].stop,&nxSerial);
      nextion_display("sw_f1",iot_ctr[Fan_A].enable,&nxSerial);
      nextion_display("sw_f2",iot_ctr[Fan_B].enable,&nxSerial);
      nextion_display("fano1",iot_ctr[Fan_A].run,&nxSerial);
      nextion_display("fanf1",iot_ctr[Fan_A].stop,&nxSerial);
      nextion_display("fano2",iot_ctr[Fan_B].run,&nxSerial);
      nextion_display("fanf2",iot_ctr[Fan_B].stop,&nxSerial);
    }else if(nextion_page == 4){
      for (uint8_t index = 0; index < 2; index++){
        nextion_display("page_liq.sw_liq"+String(index+1),iot_ctr[Liquid_1+index].enable,&nxSerial);
        nextion_display("page_liq.run"+String(index+1),iot_ctr[Liquid_1+index].run,&nxSerial);
        nextion_display("page_liq.stp"+String(index+1),iot_ctr[Liquid_1+index].stop,&nxSerial);
        nextion_display("page_liq.sw_line"+String(index+1),iot_ctr[Line_1+index].enable,&nxSerial);
      }
      nextion_display("page_liq.sw_liq_h",iot_ctr[Heater_1].enable,&nxSerial);
      nextion_display("page_liq.run_h",iot_ctr[Heater_1].run,&nxSerial);
    }else if(nextion_page == 5){
        nextion_display("sw_temp",iot_ctr[Cooler].enable,&nxSerial);
        nextion_display("run_t",iot_ctr[Cooler].run,&nxSerial);
        nextion_display("stp_t",iot_ctr[Cooler].stop,&nxSerial);
    }else if(nextion_page == 6){
      for (uint8_t index = 0; index < 4; index++){
        nextion_display("page_led.sw_led"+String(index+1),iot_ctr[Lamp_1+index].enable,&nxSerial);
        nextion_display("page_led.run"+String(index+1),iot_ctr[Lamp_1+index].run,&nxSerial);
        nextion_display("page_led.stp"+String(index+1),iot_ctr[Lamp_1+index].stop,&nxSerial);
      }
    }else if(nextion_page == 7){
        /*
        for (uint8_t index = 0; index < 2; index++){
          nextion_display("sw_fan"+String(index+1),iot_ctr[Fan_A+index].enable,&nxSerial);
          nextion_display("run_f"+String(index+1),iot_ctr[Fan_A+index].run,&nxSerial);
          nextion_display("stp_f"+String(index+1),iot_ctr[Fan_A+index].stop,&nxSerial);
        }
        */
        nextion_display("sw_fan1",iot_ctr[Fan_A].enable,&nxSerial);
        nextion_display("run_f1",iot_ctr[Fan_A].run,&nxSerial);
        nextion_display("stp_f1",iot_ctr[Fan_A].stop,&nxSerial);
        nextion_display("sw_fan2",iot_ctr[Fan_B].enable,&nxSerial);
        nextion_display("run_f2",iot_ctr[Fan_B].run,&nxSerial);
        nextion_display("stp_f2",iot_ctr[Fan_B].stop,&nxSerial);
    }else if(nextion_page == 2){
      nextion_print(&nxSerial,"page_manu.dvid.txt=\""+String(deviceID)+"\"");
      for (uint8_t index = 0; index < TOTAL_RELAY; index++){
        nextion_display("page_manu.bt"+String(index),relay_state[index],&nxSerial);
      }
    }
  }
}
void system_ctr(unsigned long millisec){
  if(millisec > prevUpdateTime + 200){
    prevUpdateTime = millisec;
    update_order += 1;
    if(update_order == 1){
      temp_water  = thermocouple1.readCelsius()*10;
      temp_liquid = thermocouple2.readCelsius()*10;
      int16_t temp_rtc = RTC_DS3231.getTemperature()*10;
      if(temp_water == -1)  temp_water=999;
      if(temp_liquid == -1) temp_liquid=999;
      if(nextion_page == 0){
        nextion_display("page_main.temp_wat",temp_water,&nxSerial);
        nextion_display("page_main.temp_liq",temp_liquid,&nxSerial);
        //nextion_display("page_main.temp_cpu1",temp_rtc,&nxSerial);
      }
      if(iot_ctr[Heater_1].enable){
        if(iot_ctr[Heater_1].run>40) iot_ctr[Heater_1].run=40; //물 온도 상한.
        uint8_t liq_temp = (temp_water/10);
        if(liq_temp > 99){
          //온도센서 고장
          iot_ctr[Heater_1].state = false;
          relay_ctr(Heater_1, false);
        }else if(liq_temp > iot_ctr[Heater_1].run){
          relay_ctr(Heater_1, false);
          iot_ctr[Heater_1].state = false;
        }else if(liq_temp < iot_ctr[Heater_1].run - lquid_gap){ //물 온도 갭 //iot_ctr[Heater_1].stop){
          relay_ctr(Heater_1, true);
          iot_ctr[Heater_1].state = true;
        }else if((iot_ctr[Heater_1].state && liq_temp > iot_ctr[Heater_1].run)||(!iot_ctr[Heater_1].state && liq_temp < iot_ctr[Heater_1].run)){
          relay_ctr(Heater_1, false);
        }
      }else if(nextion_page!=2){
        iot_ctr[Heater_1].state = true;
        relay_ctr(Heater_1, false);
      }

      if(iot_ctr[Heater_2].enable){
        if(iot_ctr[Heater_1].run>40) iot_ctr[Heater_1].run=40; //양액 온도 상한.
        uint8_t liq_temp = (temp_liquid/10);
        if(liq_temp > 99){
          //온도센서 고장
          iot_ctr[Heater_2].state = false;
          relay_ctr(Heater_2, false);
        }else if(liq_temp > iot_ctr[Heater_1].run){
          relay_ctr(Heater_2, false);
          iot_ctr[Heater_2].state = false;
        }else if(liq_temp < iot_ctr[Heater_1].run - lquid_gap){ //양액 온도 갭 //iot_ctr[Heater_2].stop){
          relay_ctr(Heater_2, true);
          iot_ctr[Heater_2].state = true;
        }else if((iot_ctr[Heater_2].state && liq_temp > iot_ctr[Heater_1].run)||(!iot_ctr[Heater_2].state && liq_temp < iot_ctr[Heater_1].run)){
          relay_ctr(Heater_2, false);
        }
      }else if(nextion_page!=2){
        iot_ctr[Heater_2].state = true;
        relay_ctr(Heater_2, false);
      }
    }else if(update_order == 2){
      temp_air    = thermocouple3.readCelsius()*10;
      temp_out    = thermocouple4.readCelsius()*10;
      int16_t temp_rtc = RTC_DS3231.getTemperature()*10;
      if(temp_air == -1)    temp_air=999;
      if(temp_out == -1)    temp_out=999;
      if(nextion_page == 0){
        nextion_display("page_main.temp_air",temp_air,&nxSerial);
        nextion_display("page_main.temp_out",temp_out,&nxSerial);
        nextion_display("page_main.bt_wing",wing_state,&nxSerial);
        //nextion_display("page_main.temp_cpu1",temp_rtc,&nxSerial);
      }
      if(iot_ctr[Cooler].enable){
        if(temp_air < temp_rtc-100 || temp_air > temp_rtc+100){
          //온도센서 고장
          relay_ctr(Cooler, false);
          relay_ctr(Heater, false);
        }else if(temp_air > iot_ctr[Cooler].run + iot_ctr[Cooler].stop){
          relay_ctr(Cooler, true);
          relay_ctr(Heater, false);
          iot_ctr[Cooler].state = true;
        }else if(temp_air < iot_ctr[Cooler].run - iot_ctr[Cooler].stop){
          relay_ctr(Cooler, false);
          relay_ctr(Heater, true);
          iot_ctr[Cooler].state = false;
        }else if((iot_ctr[Cooler].state && temp_air > iot_ctr[Cooler].run)||(!iot_ctr[Cooler].state && temp_air < iot_ctr[Cooler].run)){
          relay_ctr(Cooler, false);
          relay_ctr(Heater, false);
          iot_ctr[Cooler].state = false;
        }
      }else if(nextion_page!=2){
        relay_ctr(Cooler, false);
        relay_ctr(Heater, false);
        iot_ctr[Cooler].state = false;
      }
    }else if(update_order == 3){
      for (uint8_t index = 0; index < 2; index++){
        if(iot_ctr[Liquid_1+index].enable && (iot_ctr[Line_1].enable || iot_ctr[Line_2].enable)){
          if(iot_ctr[Liquid_1+index].state){
            if(nextion_page == 0) nextion_display("page_main.flow"+String(index+1)+"m",water_ctr_time[index]-1,&nxSerial);
            if(--water_ctr_time[index] < 1){
              iot_ctr[Liquid_1+index].state = false;
              water_ctr_time[index] = iot_ctr[Liquid_1+index].stop*60;
            }else if(water_ctr_time[index] > iot_ctr[Liquid_1+index].run) water_ctr_time[index] = iot_ctr[Liquid_1+index].run;
          }else{
            if(nextion_page == 0) nextion_display("page_main.flow"+String(index+1)+"m",(water_ctr_time[index]/60),&nxSerial);
            if(--water_ctr_time[index] < 1){
              iot_ctr[Liquid_1+index].state = true;
              water_ctr_time[index] = iot_ctr[Liquid_1+index].run;
            }else if(water_ctr_time[index] > iot_ctr[Liquid_1+index].stop*60) water_ctr_time[index] = iot_ctr[Liquid_1+index].stop*60;
          }
          relay_ctr(Line_1+index, iot_ctr[Line_1+index].enable);
          relay_ctr(Liquid_1+index, iot_ctr[Liquid_1+index].state);
        }else if(nextion_page!=2){
          iot_ctr[Liquid_1+index].state = false;
          relay_ctr(Line_1+index, false);
          relay_ctr(Liquid_1+index, false);
        }
      }
    }else if(update_order == 4){
      for(uint8_t index=0; index<2; index++){
        if(iot_ctr[Fan_A+index].enable){
          if(iot_ctr[Fan_A+index].state){
            //if(nextion_page == 0) nextion_display("fanom",(fan_ctr_time[index]/60),&nxSerial);
            if(--fan_ctr_time[index] < 1){
              iot_ctr[Fan_A+index].state = false;
              fan_ctr_time[index] = iot_ctr[Fan_A+index].stop*60;
            }else if(fan_ctr_time[index] > iot_ctr[Fan_A+index].run) fan_ctr_time[index] = iot_ctr[Fan_A+index].run;
          }else{
            //if(nextion_page == 0) nextion_display("fanfm",(fan_ctr_time[index]/60),&nxSerial);
            if(--fan_ctr_time[index] < 1){
              iot_ctr[Fan_A+index].state = true;
              fan_ctr_time[index] = iot_ctr[Fan_A+index].run;
            }else if(fan_ctr_time[index] > iot_ctr[Fan_A+index].stop*60) fan_ctr_time[index] = iot_ctr[Fan_A+index].stop*60;
          }
          relay_ctr(Fan_A+index, iot_ctr[Fan_A+index].state);
        }else if(nextion_page!=2){
          iot_ctr[Fan_A+index].state = false;
          relay_ctr(Fan_A+index, iot_ctr[Fan_A+index].state);
        }
      }
    }else {
      update_order = 0;
      bool h12Flag;
      bool pmFlag;
      uint8_t hour_now = RTC_DS3231.getHour(h12Flag, pmFlag);
      for (uint8_t index = 0; index < 4; index++){
        if(iot_ctr[Lamp_1+index].enable){
          if(iot_ctr[Lamp_1+index].run < iot_ctr[Lamp_1+index].stop){
            if(hour_now >= iot_ctr[Lamp_1+index].run && hour_now<iot_ctr[Lamp_1+index].stop){
              iot_ctr[Lamp_1+index].state = true;
            }else{
              iot_ctr[Lamp_1+index].state = false;
            }
          }else if(iot_ctr[Lamp_1+index].run==iot_ctr[Lamp_1+index].stop){
            if(hour_now == iot_ctr[Lamp_1+index].run){
              iot_ctr[Lamp_1+index].state = true;
            }else{
              iot_ctr[Lamp_1+index].state = false;
            }
          }else{
            if(hour_now >= iot_ctr[Lamp_1+index].stop && hour_now<iot_ctr[Lamp_1+index].run){
              iot_ctr[Lamp_1+index].state = false;
            }else{
              iot_ctr[Lamp_1+index].state = true;
            }
          }
          relay_ctr(Lamp_1+index, iot_ctr[Lamp_1+index].state);
          if(iot_ctr[Lamp_1+index].state){

          }else{

          }//nextion 표기
        }else if(nextion_page!=2){
          iot_ctr[Lamp_1+index].state = false;
          relay_ctr(Lamp_1+index, iot_ctr[Lamp_1+index].state);
        }
      }

    }
  }
}

String sensor_json(){
  String response = "{\"DEVID\":\""+String(deviceID)+"\",";
  response += "\"t_w\":\""+String(temp_water)+"\",";
  response += "\"t_l\":\""+String(temp_liquid)+"\",";
  response += "\"t_a\":\""+String(temp_air)+"\",";
  response += "\"t_o\":\""+String(temp_out)+"\",";  
  response += "\"i_t\":\""+String(sht31.readTemperature(),2)+"\",";  
  response += "\"i_h\":\""+String(sht31.readHumidity(),2)+"\",";
  response += "\"ze3\":\""+String(ze03_value)+"\"}";
  return response;
}

void sensor_upload(unsigned long millisec){
  if(wifi_able && (millisec > prePpdatePost + 1000*60*1)){
    prePpdatePost = millisec;
    String response = httpPOSTRequest(server+"device/log",sensor_json());
    Serial.println(response);
  }
}

String httpPOSTRequest(String server_url, String send_data) {
  String response = "";
  if(wifi_able){
    WiFiClient client;
    HTTPClient http;
    http.begin(client, server_url);
    http.addHeader("Content-Type", "application/json");
    int response_code = http.POST(send_data);
    response          = http.getString();
    http.end();
  }
  return response;
}////httpPOSTRequest_End
////--------------------- ZE03 read ------------////
void ZE03_sensor_read(){
  if (ZE03serial.available()){
    uint8_t reciveSensor = ZE03serial.read();
    if (reciveSensor == 0xFF) {
      ZE03Index = 0;
      ZE03[ZE03Index++] = reciveSensor;
    } else if (ZE03Index >= ZE03_CMD_LENGTH) {
      ZE03Index = 0;
    } else {
      ZE03[ZE03Index++] = reciveSensor;
      if (ZE03Index == ZE03_CMD_LENGTH-1) {
        uint8_t checksum = 0;
        for(uint8_t index = 1; index < ZE03_CMD_LENGTH - 1; index++) {
            checksum += ZE03[index];
        }
        checksum = ~checksum + 1;
        if(ZE03[ZE03_CMD_LENGTH-1] != checksum){
          ze03_value = ZE03[2]*256 + ZE03[3];
        }else{
          ze03_value = 404;
        }
      }
    }
  }
}
////--------------------- ZE03 read ------------////
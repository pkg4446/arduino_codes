#include <EEPROM.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient
#include <DS3231.h>
#include <max6675.h>
#include <Adafruit_SHT31.h>
#include "uart_print.h"

#define TOTAL_RELAY 10
#define EEPROM_SIZE_CONFIG  16
#define EEPROM_SIZE_VALUE   3
#define EEPROM_SIZE_CTR     8
#define COMMAND_LENGTH  32
#define UPDATE_INTERVAL 1000L

const uint8_t thermoDO[2] = {32,34};
const uint8_t thermoCS    = 15;
const uint8_t thermoCLK   = 14;

MAX6675 thermocouple1(thermoCLK, thermoCS, thermoDO[0]);
MAX6675 thermocouple2(thermoCLK, thermoCS, thermoDO[1]);
Adafruit_SHT31 sht31 = Adafruit_SHT31();

DS3231 RTC_DS3231;
HardwareSerial nxSerial(2);
uint8_t nextion_page = 0;
enum RelayFunc {
    Water_A = 0,
    Water_B,
    Water_H,
    Lamp_A,
    Lamp_B,
    Lamp_C,
    Circulater,
    Cooler,
    Heater,
    Spare_A
};
/***************EEPROM*********************/
const uint8_t eep_ssid[EEPROM_SIZE_CONFIG] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
const uint8_t eep_pass[EEPROM_SIZE_CONFIG] = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
const uint8_t eep_var[EEPROM_SIZE_VALUE*EEPROM_SIZE_CTR] = {32,33,34, 35,36,37, 38,39,40,
                                                            41,42,43, 44,45,46, 47,48,49,
                                                            50,51,52, 53,54,55};
/***************EEPROM*********************/
/***************MQTT_CONFIG****************/
const char*     mqttServer    = "smarthive.kr";
const uint16_t  mqttPort      = 1883;
const char*     mqttUser      = "hive";
const char*     mqttPassword  = "hive";
const char*     topic_pub     = "SHS";

WiFiClient    mqtt_client;
PubSubClient  mqttClient(mqtt_client);
uint8_t       mqtt_count      = 0;
/***************MQTT_CONFIG****************/
/***************PIN_CONFIG*****************/
const int8_t Relay[TOTAL_RELAY] = {2,4,5,12,13,23,27,26,25,33};
/***************PIN_CONFIG*****************/
/***************Interval_timer*************/
unsigned long prevUpdateTime = 0L;
uint8_t       update_order   = 0;
/***************Interval_timer*************/
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
ctr_var   iot_ctr[EEPROM_SIZE_CTR];
uint16_t  water_ctr_time[2]   = {0,};
uint16_t  Circulater_ctr_time = 0;
/*
ctr_var water[2];   // run:동작_초, stop:정지_분
ctr_var lamp[3];    // run:시작시간,stop:정지시간
ctr_var temp_ctr;   // run:목표값,  stop:허용치
ctr_var circulate;  // run:동작_분, stop:정지_분
*/
//이산화탄소 측정 추가
bool    wifi_able;
bool    uart_type = true;
/***************Variable*******************/
char    command_buf[COMMAND_LENGTH];
int8_t  command_num;
/***************Functions******************/
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  for (int index = 0; index < length; index++) {
    command_buf[index] = payload[index];
  }
  command_num = 0;
  Serial.println("MQTT");
  command_service(false);
}
/******************************************/
void wifi_config() {
  if(uart_type){
    serial_wifi_config(&Serial,ssid,password);
  }else{

  }
}
/******************************************/
void WIFI_scan(bool wifi_state){
  wifi_able = wifi_state;
  WiFi.disconnect(true);
  nextion_print(&nxSerial,"page 1");//nextion page 이동
  if(uart_type) Serial.println("WIFI Scanning…");
  else{}
  uint8_t networks = WiFi.scanNetworks();
  if (networks == 0) {
      if(uart_type) Serial.println("no networks found");
  }else {
    if(uart_type){
      Serial.print(networks);
      Serial.println(" networks found");
      Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    }else{}
    for (int index = 0; index < networks; ++index) {
      // Print SSID and RSSI for each network found
      if(uart_type){
        Serial.printf("%2d",index + 1);
        Serial.print(" | ");
        Serial.printf("%-32.32s", WiFi.SSID(index).c_str());
        Serial.print(" | ");
        Serial.printf("%4d", WiFi.RSSI(index));
        Serial.print(" | ");
        Serial.printf("%2d", WiFi.channel(index));
        Serial.print(" | ");
      }else{}
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
      if(uart_type) Serial.println(wifi_encryptionType);
    }
  }
  if(uart_type) Serial.println("");
  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();
  if(wifi_able){
    wifi_connect();
  }
}
/******************************************/
void command_service(bool command_type){
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
  if(!command_type){
    Serial.print("nextion: ");
    Serial.print(command_buf);
  }
  if(cmd_text=="time"){
    time_show();
  }else if(cmd_text=="timeset"){
    time_set();
  }else if(cmd_text=="reboot"){
    ESP.restart();
  }else if(cmd_text=="page"){
    nextion_page = temp_text.toInt();
  }else if(cmd_text=="send"){
    nextion_print(&nxSerial,temp_text);
  }else if(cmd_text=="temp"){
    //temp
    if(uart_type){
      Serial.print("CPU: ");Serial.print(RTC_DS3231.getTemperature(), 2);Serial.println("°C");
      Serial.print("PT100 1: ");Serial.print(thermocouple1.readCelsius());Serial.println("°C");
      Serial.print("PT100 2: ");Serial.print(thermocouple2.readCelsius());Serial.println("°C");
      Serial.print("SHT30: ");
      Wire.beginTransmission(0x44);
      if(!Wire.endTransmission()){
        Serial.print(sht31.readTemperature());
        Serial.println("°C, ");
        Serial.print(sht31.readHumidity());
        Serial.println("%");
      }else{
        Serial.println("No device");
      }
    }
    nextion_display("temp1",thermocouple1.readCelsius()*10,&nxSerial);
    nextion_display("temp2",thermocouple2.readCelsius()*10,&nxSerial);
    nextion_display("temp3",RTC_DS3231.getTemperature()*10,&nxSerial);
    Wire.beginTransmission(0x44);
    if(!Wire.endTransmission()){
      nextion_display("temp4",sht31.readTemperature()*10,&nxSerial);
      nextion_display("humi",sht31.readHumidity()*10,&nxSerial);
    }else{
      nextion_display("temp4",999,&nxSerial);
    }
  }else if(cmd_text=="set"){
    uint8_t iot_ctr_type = 255;
    if(temp_text=="water_a"){iot_ctr_type=Water_A;}
    else if(temp_text=="water_b"){iot_ctr_type=Water_B;}
    else if(temp_text=="water_h"){iot_ctr_type=Water_H;}
    else if(temp_text=="lamp_a"){iot_ctr_type=Lamp_A;}
    else if(temp_text=="lamp_b"){iot_ctr_type=Lamp_B;}
    else if(temp_text=="lamp_c"){iot_ctr_type=Lamp_C;}
    else if(temp_text=="circul"){iot_ctr_type=Circulater;}
    else if(temp_text=="temp"){ iot_ctr_type=Cooler; }
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
      if(uart_type){
        Serial.print("enable: ");Serial.print(iot_ctr[iot_ctr_type].enable);
        Serial.print(", run :");Serial.print(iot_ctr[iot_ctr_type].run);
        Serial.print(", stop :");Serial.println(iot_ctr[iot_ctr_type].stop);
      }
    }
  }else if(cmd_text=="config"){
    uint8_t iot_ctr_type = 255;
    if(temp_text=="water_a"){iot_ctr_type=Water_A;}
    else if(temp_text=="water_b"){iot_ctr_type=Water_B;}
    else if(temp_text=="water_h"){iot_ctr_type=Water_H;}
    else if(temp_text=="lamp_a"){iot_ctr_type=Lamp_A;}
    else if(temp_text=="lamp_b"){iot_ctr_type=Lamp_B;}
    else if(temp_text=="lamp_c"){iot_ctr_type=Lamp_C;}
    else if(temp_text=="circul"){iot_ctr_type=Circulater;}
    else if(temp_text=="temp"){iot_ctr_type=Cooler;}
    if(iot_ctr_type != 255){
      if(uart_type){
        Serial.print("enable: ");Serial.print(iot_ctr[iot_ctr_type].enable);
        Serial.print(", run :");Serial.print(iot_ctr[iot_ctr_type].run);
        Serial.print(", stop :");Serial.println(iot_ctr[iot_ctr_type].stop);
      }else{

      }
    }else if(uart_type){
      for (int index = 0; index < EEPROM_SIZE_CTR; index++) {
        Serial.print("config no.");Serial.print(index);
        Serial.print(", enable: ");Serial.print(iot_ctr[index].enable);
        Serial.print(", run :");Serial.print(iot_ctr[index].run);
        Serial.print(", stop :");Serial.println(iot_ctr[index].stop);
      }
    }
  }
  /*****OFF_LINE_CMD*****/
  else if(command_type){
    if(cmd_text=="ssid"){
      wifi_able = false;
      WiFi.disconnect(true);
      if(uart_type) Serial.print("ssid: ");
      if(temp_text.length() > 0){
        for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
          if(index < temp_text.length()){
            if(uart_type) Serial.print(temp_text[index]);
            ssid[index] = temp_text[index];
            EEPROM.write(eep_ssid[index], byte(temp_text[index]));
          }else{
            EEPROM.write(eep_ssid[index], byte(0x00));
          }
        }
        eep_change = true;
      }
      if(uart_type) Serial.println("");
    }else if(cmd_text=="pass"){
      wifi_able = false;
      WiFi.disconnect(true);
      if(uart_type) Serial.print("pass: ");
      if(temp_text.length() > 0){
        for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
          if(index < temp_text.length()){
            if(uart_type) Serial.print(temp_text[index]);
            password[index] = temp_text[index];
            EEPROM.write(eep_pass[index], byte(temp_text[index]));
          }else{
            EEPROM.write(eep_pass[index], byte(0x00));
          }
        }
        eep_change = true;
      }
      if(uart_type) Serial.println("");
    }else if(cmd_text=="wifi"){
      if(temp_text=="stop"){
        wifi_able = false;
        if(uart_type) Serial.print("WIFI disconnect");
        WiFi.disconnect(true);
      }else if(temp_text=="scan"){
        WIFI_scan(WiFi.status() == WL_CONNECTED);
      }else{
        wifi_connect();
      }
    }else if(uart_type && cmd_text=="help"){
      serial_command_help(&Serial);
    }else{ serial_err_msg(&Serial, command_buf); }
  }
  /*****OFF_LINE_CMD*****/
  else{ serial_err_msg(&Serial, command_buf); }
  if(eep_change){
    EEPROM.commit();
  }
}
void command_process(char ch, bool type_uart) {
  uart_type = type_uart;
  if(ch=='\n'){
    command_buf[command_num] = 0x00;
    command_num = 0;
    command_service(true);
    memset(command_buf, 0x00, COMMAND_LENGTH);
  }else if(ch!='\r'){
    command_buf[command_num++] = ch;
    command_num %= COMMAND_LENGTH;
  }
}
/******************************************/
void mqtt_connect() {
  mqttClient.disconnect();
  if(wifi_able){
    //nextion_print(&nxSerial,"MAC.txt=\"Try to connect WIFI\"");
    //nextion_print(&nxSerial,"btWIFI.val=0");
    mqttClient.setServer(mqttServer, mqttPort);
    mqttClient.setCallback(mqtt_callback);

    char  deviceID[18];
    char  sendID[21]  = "ID=";

    for (int i = 0; i < 17; i++) {
      sendID[i + 3] = WiFi.macAddress()[i];
      deviceID[i]   = sendID[i + 3];
    }

    char* topic_sub = deviceID;
    char* sub_ID    = sendID;

    unsigned long WIFI_wait  = millis();
    bool mqtt_connected = true;
    while (!mqttClient.connected()) {
      if (millis() > WIFI_wait + 1000) {
        WIFI_wait = millis();
        if (!wifi_able){
          if(uart_type) Serial.println("WIFI was not connected");
          mqttClient.disconnect();
          return;
        }else if(mqttClient.connect(deviceID, mqttUser, mqttPassword )) {
          if(uart_type) Serial.println("connected");
        } else {
          if(uart_type){
            Serial.print("failed with state ");
            Serial.print(mqttClient.state());
          }
          mqtt_connected = false;
          break;
        }
      }
    }
    if(uart_type) Serial.print("MQTT Connected ");
    if(mqtt_connected){
      mqttClient.subscribe(topic_sub);
      mqttClient.publish(topic_pub, sub_ID);
      if(uart_type) Serial.println(sub_ID);
    }else{
      if(uart_type) Serial.println("fail");
    }
  }
}

void wifi_connect() {
  wifi_able = true;
  wifi_config();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long wifi_config_update  = millis();
  while (WiFi.status() != WL_CONNECTED) {
    unsigned long update_time = millis();
    if(update_time - wifi_config_update > 5000){
      wifi_able = false;
      if(uart_type) Serial.println("WIFI fail");
      break;
    }
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

  if(uart_type){
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
  nextion_print(&nxSerial,"rtc3="+String(clock_dow));
  nextion_print(&nxSerial,"rtc4="+String(clock_hour));
  nextion_print(&nxSerial,"rtc5="+String(clock_min));
  nextion_print(&nxSerial,"rtc6="+String(clock_sec));
}

void time_set(){
  /*
  JS CODE
  {
    let now  = new Date();
    let time = `time ${now.getFullYear()} ${now.getMonth()+1} ${now.getDate()} ${now.getUTCDay()} ${now.getHours()} ${now.getMinutes()} ${now.getSeconds()}`;
  }
  DATA
    timeset 2024 5 22 3 15 48 39
  */
  uint8_t time_data[6];
  uint8_t time_index = 0;
  String time_text = "";
  for(uint8_t index=8; index<COMMAND_LENGTH; index++){
    if(command_buf[index] == 0x20 || command_buf[index] == 0x00){
      time_data[time_index++] = (time_text.toInt())%100;
      time_text = "";
      index+=1; //passing to 0x20
      if(time_index > 5) break;
    };
    time_text += command_buf[index];
  }
  if(time_data[3]==0) time_data[3] = 7;
  RTC_DS3231.setYear(time_data[0]);
  RTC_DS3231.setMonth(time_data[1]);
  RTC_DS3231.setDate(time_data[2]);
  RTC_DS3231.setDoW(time_data[3]);
  RTC_DS3231.setHour(time_data[4]);
  RTC_DS3231.setMinute(time_data[5]);
  RTC_DS3231.setSecond(time_data[6]);
}
/******************************************/
/***************Functions******************/

void setup() {
  Serial.begin(115200);
  nxSerial.begin(115200);
  Wire.begin();

  for (uint8_t index = 0; index < TOTAL_RELAY; index++)
  {
    pinMode(Relay[index], OUTPUT);
    digitalWrite(Relay[index], false);
  }

  if (!EEPROM.begin((EEPROM_SIZE_CONFIG*2) + (EEPROM_SIZE_VALUE*EEPROM_SIZE_CTR))){
    if(uart_type){
      Serial.println("Failed to initialise eeprom");
      Serial.println("Restarting...");
    }
    delay(1000);
    ESP.restart();
  }
  for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
    ssid[index]     = EEPROM.read(eep_ssid[index]);
    password[index] = EEPROM.read(eep_pass[index]);
  }

  for (int index = 0; index < EEPROM_SIZE_CTR; index++) {
    iot_ctr[index].state  = true;
    iot_ctr[index].enable = EEPROM.read(eep_var[3*index]);
    iot_ctr[index].run    = EEPROM.read(eep_var[3*index+1]);
    iot_ctr[index].stop   = EEPROM.read(eep_var[3*index+2]);
  }

  wifi_connect();
  serial_command_help(&Serial);
  if(uart_type) Serial.println("System online");
  nextion_print(&nxSerial,"page 0");
  nextion_display("wifi",wifi_able,&nxSerial);
}

// the loop function runs over and over again forever
void loop() {
  if(wifi_able){
    if (mqttClient.connected()){mqttClient.loop();}
    else{mqtt_connect();}
  }
  if (Serial.available()) command_process(Serial.read(),true);
  if (nxSerial.available()) command_process(nxSerial.read(),false);
  system_ctr(millis());
}

void system_ctr(unsigned long millisec){
  if(millisec > prevUpdateTime + 250){
    prevUpdateTime = millisec;
    update_order += 1;
    if(update_order == 1){
      if(iot_ctr[Cooler].enable){
        int8_t temp_now = 25;//온도
        int8_t temp_rtc = int8_t(RTC_DS3231.getTemperature());
        if(temp_now < temp_rtc-10 || temp_now > temp_rtc+10){
          //온도센서 고장
          digitalWrite(Relay[Cooler], false);
          digitalWrite(Relay[Heater], false);
        }else if(temp_now > iot_ctr[Cooler].run + iot_ctr[Cooler].stop){
          digitalWrite(Relay[Cooler], true);
          digitalWrite(Relay[Heater], false);
          iot_ctr[Cooler].state = true;
        }else if(temp_now < iot_ctr[Cooler].run - iot_ctr[Cooler].stop){
          digitalWrite(Relay[Cooler], false);
          digitalWrite(Relay[Heater], true);
          iot_ctr[Cooler].state = false;
        }else if((iot_ctr[Cooler].state && temp_now > iot_ctr[Cooler].run)||(!iot_ctr[Cooler].state && temp_now < iot_ctr[Cooler].run)){
          digitalWrite(Relay[Cooler], false);
          digitalWrite(Relay[Heater], false);
        }
      }
      if(iot_ctr[Water_H].enable){
        int8_t temp_now = 25;//온도
        int8_t temp_rtc = int8_t(RTC_DS3231.getTemperature());
        if(temp_now < temp_rtc-20 || temp_now > temp_rtc+20){
          //온도센서 고장
          digitalWrite(Relay[Water_H], false);
        }else if(temp_now > iot_ctr[Water_H].run + iot_ctr[Water_H].stop){
          digitalWrite(Relay[Water_H], false);
          iot_ctr[Water_H].state = false;
        }else if(temp_now < iot_ctr[Water_H].run - iot_ctr[Water_H].stop){
          digitalWrite(Relay[Water_H], true);
          iot_ctr[Water_H].state = true;
        }else if((iot_ctr[Water_H].state && temp_now > iot_ctr[Water_H].run)||(!iot_ctr[Water_H].state && temp_now < iot_ctr[Water_H].run)){
          digitalWrite(Relay[Water_H], false);
        }
      }
    }else if(update_order == 2){
      for (uint8_t index = 0; index < 2; index++){
        if(iot_ctr[Water_A+index].enable){
          if(iot_ctr[Water_A+index].state){
            if(--water_ctr_time[index] < 1){
              iot_ctr[Water_A+index].state = false;
              water_ctr_time[index] = iot_ctr[Water_A+index].stop*60;
            }else if(water_ctr_time[index] > iot_ctr[Water_A+index].run) water_ctr_time[index] = iot_ctr[Water_A+index].run;
          }else{
            if(--water_ctr_time[index] < 1){
              iot_ctr[Water_A+index].state = true;
              water_ctr_time[index] = iot_ctr[Water_A+index].run;
            }else if(water_ctr_time[index] > iot_ctr[Water_A+index].stop) water_ctr_time[index] = iot_ctr[Water_A+index].stop;
          }
        }
      }
    }else if(update_order == 3){
      if(iot_ctr[Circulater].enable){
        if(iot_ctr[Circulater].state){
          if(--Circulater_ctr_time < 1){
            iot_ctr[Circulater].state = false;
            Circulater_ctr_time = iot_ctr[Circulater].stop*60;
          }else if(Circulater_ctr_time > iot_ctr[Circulater].run) Circulater_ctr_time = iot_ctr[Circulater].run;
        }else{
          if(--Circulater_ctr_time < 1){
            iot_ctr[Circulater].state = true;
            Circulater_ctr_time = iot_ctr[Circulater].run*60;
          }else if(Circulater_ctr_time > iot_ctr[Circulater].stop) Circulater_ctr_time = iot_ctr[Circulater].stop;
        }
      }
    }else {
      update_order = 0;
      bool h12Flag;
      bool pmFlag;
      uint8_t hour_now = RTC_DS3231.getHour(h12Flag, pmFlag);
      for (uint8_t index = 0; index < 3; index++){
        if(iot_ctr[Lamp_A+index].enable){
          if(iot_ctr[Lamp_A+index].run < iot_ctr[Lamp_A+index].stop){
            if(hour_now >= iot_ctr[Lamp_A+index].run && hour_now<iot_ctr[Lamp_A+index].stop){
              iot_ctr[Lamp_A+index].state = true;
            }else{
              iot_ctr[Lamp_A+index].state = false;
            }
          }else if(iot_ctr[Lamp_A+index].run==iot_ctr[Lamp_A+index].stop){
            if(hour_now == iot_ctr[Lamp_A+index].run){
              iot_ctr[Lamp_A+index].state = true;
            }else{
              iot_ctr[Lamp_A+index].state = false;
            }
          }else{
            if(hour_now >= iot_ctr[Lamp_A+index].stop && hour_now<iot_ctr[Lamp_A+index].run){
              iot_ctr[Lamp_A+index].state = false;
            }else{
              iot_ctr[Lamp_A+index].state = true;
            }
          }
          digitalWrite(Relay[index], iot_ctr[Lamp_A+index].state);
          if(iot_ctr[Lamp_A+index].state){

          }else{

          }//nextion 표기
        }
      }


    }
  }
}
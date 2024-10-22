//// 2024.10.10
//// http post 로 인터넷 설정 받아오는부분 추가할것.
#include <EEPROM.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "uart_print.h"

#define EEPROM_SIZE_WIFI  24
#define EEPROM_SIZE_CODE  12
#define EEPROM_INTERVAL   1
#define EEPROM_EXTRA      1
#define COMMAND_LENGTH    32
#define ZE03_CMD_LENGTH   9
#define SECONDE           1000L

// #include <SoftwareSerial.h>//https://github.com/plerup/espsoftwareserial/
// SoftwareSerial ZE03serial;
// #define ZE03_RX 25
// #define ZE03_TX 26

HardwareSerial ZE03serial(2);
#define ZE03_RX 18
#define ZE03_TX 19

const char* server_main = "http://server.kr/";
const char* server_lemi = "https://www.lemi.or.kr/oms/odorAnls/insertSensorOu.do";
char    deviceID[18];
char    command_buf[COMMAND_LENGTH];
int8_t  command_num;
////--------------------- EEPROM ----------------------////
const uint8_t eep_interval = 0;
const uint8_t eep_ssid[EEPROM_SIZE_WIFI] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
const uint8_t eep_pass[EEPROM_SIZE_WIFI] = {25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48};
const uint8_t eep_farm[EEPROM_SIZE_CODE] = {49,50,51,52,53,54,55,56,57,58,59,60};
const uint8_t eep_sens[EEPROM_SIZE_CODE] = {61,62,63,64,65,66,67,68,69,70,71,72};
const uint8_t eep_arlam   = 73;

char  ssid[EEPROM_SIZE_WIFI];
char  password[EEPROM_SIZE_WIFI];
char  farm_code[EEPROM_SIZE_CODE];
char  sens_code[EEPROM_SIZE_CODE];
////--------------------- EEPROM ----------------------////
////--------------------- Pin out ---------------------////
////--------------------- Pin out ---------------------////
////--------------------- Flage -----------------------////
bool  wifi_able     = false;
////--------------------- Flage -----------------------////
////--------------------- Interval timer  -------------////
unsigned long prev_update     = 0L;
unsigned long prev_data_post  = 0L;
uint8_t  data_post_interval   = 1;
////--------------------- Interval timer  -------------////
////--------------------- Value -----------------------////
uint8_t  ZE03[ZE03_CMD_LENGTH] = {0,};
uint8_t  ZE03Index = 0;
uint16_t ze03_value = 0;
bool     ze03_alarm = false;
////--------------------- Value -----------------------////
////--------------------- setup() ---------------------////
void setup()
{
  Serial.begin(115200);
  // ZE03serial.begin(9600, SWSERIAL_8N1, ZE03_RX, ZE03_TX);
  ZE03serial.begin(9600, SERIAL_8N1, ZE03_RX, ZE03_TX);
  if (!EEPROM.begin((EEPROM_SIZE_WIFI+EEPROM_SIZE_CODE)*2 + EEPROM_INTERVAL + EEPROM_EXTRA)){
    Serial.println("Failed to initialise eeprom");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  };
  data_post_interval = EEPROM.read(eep_interval);
  for (int index = 0; index < EEPROM_SIZE_WIFI; index++) {
    ssid[index]     = EEPROM.read(eep_ssid[index]);
    password[index] = EEPROM.read(eep_pass[index]);
  }
  for (int index = 0; index < EEPROM_SIZE_CODE; index++) {
    farm_code[index] = EEPROM.read(eep_farm[index]);
    sens_code[index] = EEPROM.read(eep_sens[index]);
  }
  data_post_interval = EEPROM.read(eep_interval);
  wifi_connect();
  for (uint8_t index = 0; index < 17; index++) {
    if(WiFi.macAddress()[index]==':'){
      deviceID[index] = '_';
    }else{
      deviceID[index] = WiFi.macAddress()[index];
    }
  }
  
  Serial.println("System online");
}
////--------------------- setup() ---------------------////
////--------------------- loop() ----------------------////
void loop()
{
  const unsigned long millisec = millis();
  if (Serial.available()) command_process(Serial.read());
  sensor_upload(millisec);
  ZE03_sensor_read();
}
////--------------------- loop() ----------------------////
////--------------------- command ---------------------////
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
  ////cmd start
  if(cmd_text=="reboot"){
    ESP.restart();
  }else if(cmd_text=="show"){
    Serial.print("ZE03 data = ");
    Serial.println(ze03_value);
  }else if(cmd_text=="post"){
    // httpPOST_JSON(server_main,sensor_json());
    String response = httpPOST_QUERY(server_lemi,sensor_query());
    Serial.println(response);
    // data post
  }else if(cmd_text=="set"){
    String cmd_value = "";
    for(uint8_t index_check=check_index; index_check<COMMAND_LENGTH; index_check++){
      if(command_buf[index_check] == 0x20 || command_buf[index_check] == 0x00){
        check_index = index_check+1;
        break;
      }
      cmd_value += command_buf[index_check];
    }

    if(temp_text=="interval"){
      uint8_t set_value = cmd_value.toInt();
      EEPROM.write(eep_interval, set_value);
      data_post_interval = set_value;
      Serial.print("Log interval = ");
      Serial.print(data_post_interval);
      Serial.println("Minute");
      EEPROM.commit();
    }else if(temp_text=="farm"){
      if(cmd_value.length() > 0){
        for (int index = 0; index < EEPROM_SIZE_CODE; index++) {
          if(index < cmd_value.length()){
            Serial.print(cmd_value[index]);
            farm_code[index] = cmd_value[index];
            EEPROM.write(eep_farm[index], byte(cmd_value[index]));
          }else{
            farm_code[index] = 0x00;
            EEPROM.write(eep_farm[index], byte(0x00));
          }
        }
      }
      EEPROM.commit();
    }else if(temp_text=="sens"){
      if(cmd_value.length() > 0){
        for (int index = 0; index < EEPROM_SIZE_CODE; index++) {
          if(index < cmd_value.length()){
            Serial.print(cmd_value[index]);
            sens_code[index] = cmd_value[index];
            EEPROM.write(eep_sens[index], byte(cmd_value[index]));
          }else{
            sens_code[index] = 0x00;
            EEPROM.write(eep_sens[index], byte(0x00));
          }
        }
      }
      EEPROM.commit();
    }    
  }else if(cmd_text=="config"){
    Serial.print("Log interval = ");
    Serial.print(data_post_interval);
    Serial.println("Minute");
    Serial.print("Farm code = ");
    Serial.println(farm_code);
    Serial.print("Sensor code = ");
    Serial.println(sens_code);
    EEPROM.commit();
  }else if(cmd_text=="ssid"){
    wifi_able = false;
    WiFi.disconnect(true);
    Serial.print("ssid: ");
    if(temp_text.length() > 0){
      for (int index = 0; index < EEPROM_SIZE_WIFI; index++) {
        if(index < temp_text.length()){
          Serial.print(temp_text[index]);
          ssid[index] = temp_text[index];
          EEPROM.write(eep_ssid[index], byte(temp_text[index]));
        }else{
          ssid[index] = 0x00;
          EEPROM.write(eep_ssid[index], byte(0x00));
        }
      }
    }
    Serial.println("");
    EEPROM.commit();
  }else if(cmd_text=="pass"){
    wifi_able = false;
    WiFi.disconnect(true);
    Serial.print("pass: ");
    if(temp_text.length() > 0){
      for (int index = 0; index < EEPROM_SIZE_WIFI; index++) {
        if(index < temp_text.length()){
          Serial.print(temp_text[index]);
          password[index] = temp_text[index];
          EEPROM.write(eep_pass[index], byte(temp_text[index]));
        }else{
          password[index] = 0x00;
          EEPROM.write(eep_pass[index], byte(0x00));
        }
      }
    }
    Serial.println("");
    EEPROM.commit();
  }else if(cmd_text=="wifi"){
    if(temp_text=="stop"){
      wifi_able = false;
      WiFi.disconnect(true);
      Serial.print("WIFI disconnect");
    }else if(temp_text=="scan"){
      WIFI_scan(WiFi.status() == WL_CONNECTED);
    }else{
      wifi_connect();
    }
  }else{
    serial_command_help(&Serial);
  }
}
////---------------------------------------------------////
void command_process(char ch) {
  if(ch=='\n'){
    command_buf[command_num] = 0x00;
    command_num = 0;
    command_service();
    memset(command_buf, 0x00, COMMAND_LENGTH);
  }else if(ch!='\r'){
    command_buf[command_num++] = ch;
    command_num %= COMMAND_LENGTH;
  }
}
////--------------------- command ---------------------////
////--------------------- wifi ------------------------////
void WIFI_scan(bool wifi_state){
  wifi_able = wifi_state;
  WiFi.disconnect(true);
  Serial.println("WIFI Scanning…");
  uint8_t networks = WiFi.scanNetworks();
  if (networks == 0) {
    Serial.println("WIFI not found!");
  }else {
    Serial.print(networks);
    Serial.println(" networks found");
    Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    String wifi_list ="";
    for (int index = 0; index < networks; ++index) {
      // Print SSID and RSSI for each network found
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
      Serial.println(wifi_encryptionType);
    }
    Serial.println("");
  }
  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();
  if(wifi_able){
    wifi_connect();
  }
}
////---------------------------------------------------////
void wifi_connect() {
  wifi_able = true;
  serial_wifi_config(&Serial,ssid,password);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
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
  if(wifi_able) Serial.println("WIFI connected");
}
////--------------------- wifi ------------------------////
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
        ze03_value = ZE03[2]*256 + ZE03[3];
        uint8_t checksum = 0;
        for(uint8_t index = 1; index < ZE03_CMD_LENGTH - 1; index++) {
            checksum += ZE03[index];
        }
        checksum = ~checksum + 1;
        if(ZE03[ZE03_CMD_LENGTH-1] != checksum){
          Serial.print("checksum err: ");
          Serial.println(ze03_value);
        }
      }
    }
  }
}
////--------------------- ZE03 read ------------////
////--------------------- sensor data upload ----------////
String sensor_json(){
  return "{\"DVC\":\""+String(deviceID)+"\",\"ZE03\":"+ze03_value+"}";
}
String httpPOST_JSON(String server_url, String send_data) {
  String response = "";
  if(wifi_able){
    WiFiClient client;
    HTTPClient http;
    http.begin(client, server_url);
    http.addHeader("Content-Type", "application/json");
    int response_code = http.POST(send_data);
    Serial.print("response_code: ");
    Serial.println(response_code);
    response          = http.getString();
    http.end();
  }
  return response;
}////httpPOSTRequest_End
String sensor_query(){
  String response = "0," + String(ze03_value) + ",0,0,0,";
  if(ze03_alarm) response += "1,";
  else           response += "0,";
  response += "0,0";
  return response;
}
String httpPOST_QUERY(String server_url, String send_data) {
  String response = "";
  if(wifi_able){
    WiFiClient client;
    HTTPClient http;
    String postUrl = server_url + "?FARM=" + String(farm_code) + 
                    "&SENSOR=" + String(sens_code) + "@" + String(sens_code) + 
                    "&VALUE=" + send_data;
    http.begin(client, postUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("cache-control", "no-cache");
    int response_code = http.POST("");
    Serial.print("response_code: ");
    Serial.println(response_code);
    response          = http.getString();
    http.end();
  }
  return response;
}////httpPOSTRequest_End
void sensor_upload(unsigned long millisec){
  if(wifi_able && (millisec > prev_data_post + SECONDE*60*data_post_interval)){
    prev_data_post = millisec;
    String response = "";
    // response = "1:"+httpPOST_JSON(server_main,sensor_json());
    // Serial.println(response);
    response = "2:"+httpPOST_QUERY(server_lemi,sensor_query());
    Serial.println(response);
  }
}
////--------------------- sensor data upload ----------////
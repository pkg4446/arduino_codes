#include <Update.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include "uart_print.h"

String firmwareVersion = "0.0.1";

#define TCAADDR                   0x70
#define TOTAL_TEMPERATURE_SENSOR  4
#define EEPROM_SIZE_CONFIG        24
#define COMMAND_LENGTH            32
#define SECONDE                   1000L

#define SENSOR_HEAT               0
#define SENSOR_AIR                1


const   String server = "http://yc.beetopia.kro.kr/";
char    deviceID[18];
char    command_buf[COMMAND_LENGTH];
int8_t  command_num;
////--------------------- EEPROM ----------------------////
const uint8_t eep_ssid = 0;
const uint8_t eep_pass = 24;
const uint8_t eep_use_heat    = 48;
const uint8_t upload_interval = 49;
const uint8_t eep_temp = 50;

char  ssid[EEPROM_SIZE_CONFIG];
char  password[EEPROM_SIZE_CONFIG];
////--------------------- EEPROM ----------------------////
////--------------------- Pin out ---------------------////
const uint8_t MOSFET = 4;
////--------------------- Pin out ---------------------////
////--------------------- Flage -----------------------////
bool    wifi_able     = false;
bool    manual_mode   = false;
bool    heat_use      = false;
bool    heater_state  = false;
uint8_t upload_period = 5;
////--------------------- Flage -----------------------////
////--------------------- temperature sensor ----------////
Adafruit_SHT31 sht31 = Adafruit_SHT31();
float sensor_temperature[TOTAL_TEMPERATURE_SENSOR] = {0.00f,};
float sensor_humidity[TOTAL_TEMPERATURE_SENSOR]    = {0.00f,};
////--------------------- temperature sensor ----------////
////--------------------- temperature control ---------////
const uint8_t temperature_gap = 0;
uint8_t temperature_goal      = 0;
////--------------------- temperature control ---------////
////--------------------- Interval timer --------------////
unsigned long prev_update     = 0L;
unsigned long prev_data_post  = 0L;
unsigned long prev_led_heater = 0L;
unsigned long prev_led_toggle = 0L;
unsigned long prev_reconnect  = 0L;

uint16_t working_total  = 0;
uint16_t heater_working = 0;
////--------------------- Interval timer --------------////
////--------------------- Serial command --------------////
void Serial_command(){ if(Serial.available()) command_process(Serial.read()); }
////--------------------- Serial command --------------////
////--------------------- setup() ---------------------////
void setup()
{
  Serial.begin(115200);
  Wire.begin();
  Serial.print("ver:");
  Serial.println(firmwareVersion);
  pinMode(MOSFET, OUTPUT);
  digitalWrite(MOSFET, false);

  if (!EEPROM.begin((EEPROM_SIZE_CONFIG*2) + 3)){
    Serial.println("Failed to initialise eeprom");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
  bool check_init = false;
  for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
    ssid[index]     = EEPROM.read(eep_ssid+index);
    password[index] = EEPROM.read(eep_pass+index);
  }

  heat_use          = EEPROM.read(eep_use_heat);
  upload_period     = EEPROM.read(upload_interval);  
  temperature_goal  = EEPROM.read(eep_temp);

  if(upload_period > 60){
    check_init = true;
    uint init_interval = 5;
    EEPROM.write(upload_interval, init_interval);
    upload_period = init_interval;
  }else if(upload_period == 0){
    check_init = true;
    uint init_interval = 1;
    EEPROM.write(upload_interval, init_interval);
    upload_period = init_interval;
  }

  if(temperature_goal > 50){
    check_init = true;
    uint init_goal = 3;
    EEPROM.write(eep_temp, init_goal);
    temperature_goal = init_goal;
  }
  
  if(check_init)EEPROM.commit();

  wifi_connect();
  for (uint8_t index = 0; index < 17; index++) {
    if(WiFi.macAddress()[index]==':'){
      deviceID[index] = '_';
    }else{
      deviceID[index] = WiFi.macAddress()[index];
    }
  }
  Serial.print("System online, mac:");
  Serial.println(deviceID);
}
////--------------------- setup() ---------------------////
////--------------------- loop() ----------------------////
void loop()
{
  const unsigned long millisec = millis();
  Serial_command();
  system_control(millisec);
  upload_loop(millisec);
  wifi_reconnect(millisec);
}
////--------------------- loop() ----------------------////`
////--------------------- system control --------------////
void system_control(unsigned long millisec){
  if(millisec - prev_update > SECONDE){
    prev_update   = millisec;
    working_total += 1;
    temperature_sensor_read();
    if(!manual_mode){
      if(heat_use &&
        (isnan(sensor_temperature[SENSOR_AIR]) || sensor_temperature[SENSOR_AIR] < 33.00f) &&
        (temperature_sensor_tm[SENSOR_HEAT] < temperature_goal-temperature_gap))
      {
        heater_state = true;
        heater_working += 1;
      }else heater_state = false;
      digitalWrite(MOSFET, heater_state);
    }//if not manual mode
  }
}
////--------------------- system control --------------////
////--------------------- command ---------------------////
void command_service(){
  bool    eep_change  = false;
  uint8_t check_index = 0;
  String cmd_text  = String_slice(&check_index, command_buf, 0x20);
  String temp_text = String_slice(&check_index, command_buf, 0x20);
  ////cmd start
  if(cmd_text=="reboot"){
    ESP.restart();

  }else if(cmd_text=="temp"){
    temperature_sensor_read();
    for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++){
      Serial.print(index);
      Serial.print(" IC:");
      Serial.print(sensor_temperature[index]);
      Serial.print("℃,");
      Serial.print(sensor_humidity[index]);
      Serial.println("%");
    }

  }else if(cmd_text=="run"){
    bool change_flage = false;
    if(temp_text=="on" && !heat_use){
      EEPROM.write(eep_use_heat, true);
      heat_use = true;
      change_flage = true;
    }else if(heat_use){
      EEPROM.write(eep_use_heat, false);
      heat_use = false;
      change_flage = true;
    }
    if(change_flage){
      EEPROM.commit();
      config_upload();
    }
    Serial.println(command_buf);

  }else if(cmd_text=="set"){
    uint8_t set_value = temp_text.toInt();
    bool change_flage = false;
    if(50 > set_value && set_value > 0){
      EEPROM.write(eep_temp, set_value);
      temperature_goal = set_value;
      Serial.println(command_buf);
      change_flage = true;
    }else{
      Serial.println("set value error");
    }
    if(change_flage){
      EEPROM.commit();
      config_upload();
    }

  }else if(cmd_text=="config"){
    Serial.print("upload interval : ");
    Serial.print(upload_period);
    Serial.println(" Min");
    Serial.print("homeothermy mode : ");
    if(heat_use) Serial.println("ON");
    else Serial.println("OFF");
    Serial.print("temperature setup ");
    Serial.print(temperature_goal);
    Serial.println(" ℃");

  }else if(cmd_text=="gap"){
    uint8_t set_value = temp_text.toInt();
    EEPROM.write(upload_interval, set_value);
    upload_period = set_value;
    EEPROM.commit();
    Serial.print("upload interval ");
    Serial.println(upload_period);

  }else if(cmd_text=="test"){
    uint8_t set_value = String_slice(&check_index, command_buf, 0x20).toInt();
    if(temp_text=="mode"){
      manual_mode = true;
      Serial.println("Manual mode ON");
    }else if(temp_text=="on"){
      digitalWrite(MOSFET, true);
    }else if(temp_text=="off"){
      digitalWrite(MOSFET, false);
    }else if(temp_text=="data"){
      sensor_upload();
    }else{
      manual_mode = false;
      Serial.println("Manual mode OFF");
    }

  }else if(cmd_text=="ssid"){
    wifi_able = false;
    WiFi.disconnect(true);
    Serial.print("ssid: ");
    if(temp_text.length() > 0){
      for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
        if(index < temp_text.length()){
          Serial.print(temp_text[index]);
          ssid[index] = temp_text[index];
          EEPROM.write(eep_ssid+index, byte(temp_text[index]));
        }else{
          ssid[index] = 0x00;
          EEPROM.write(eep_ssid+index, byte(0x00));
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
      for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
        if(index < temp_text.length()){
          Serial.print(temp_text[index]);
          password[index] = temp_text[index];
          EEPROM.write(eep_pass+index, byte(temp_text[index]));
        }else{
          password[index] = 0x00;
          EEPROM.write(eep_pass+index, byte(0x00));
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

  }else if(cmd_text=="firm"){
    firmware_upadte();
    
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
    Serial_command();
    if(update_time - wifi_config_update > SECONDE){
      wifi_able = false;
      Serial.println("WIFI fail");
      break;
    }
  }
  if(wifi_able) Serial.println("WIFI connected");
}
////---------------------------------------------------////
void wifi_reconnect(unsigned long millisec) {
  if(!wifi_able && millisec - prev_reconnect > SECONDE*60){
    prev_reconnect = millisec;
    wifi_connect();
  }
}
////--------------------- wifi ------------------------////

////--------------------- temperature read ------------////
void tcaselect(uint8_t index) {
  if (index > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << index);
  Wire.endTransmission();
}
void temperature_sensor_read(){
  for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++){
    tcaselect(index);
    Wire.beginTransmission(68);
    if (!Wire.endTransmission() && sht31.begin(0x44)) {
      sensor_temperature[index] = sht31.readTemperature();
      sensor_humidity[index]    = sht31.readHumidity();
    }else{
      sensor_temperature[index] = NAN;
      sensor_humidity[index]    = NAN;
    }
  }
}// heater recovery
////--------------------- temperature read ------------////
////--------------------- sensor data upload ----------////
String sensor_json(){
  const uint8_t post_menu = 2;
  String res_array[post_menu] = {"\"HM\":[","\"TP\":[",};

  for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++){
    res_array[0] += "\""+String(sensor_humidity[index])+"\"";
    res_array[1] += "\""+String(sensor_temperature[index])+"\"";
    if(index < TOTAL_TEMPERATURE_SENSOR-1){
      for (uint8_t menu_index = 0; menu_index < post_menu; menu_index++){
        res_array[menu_index] += ",";
      }
    }
  }
  for (uint8_t menu_index = 0; menu_index < post_menu; menu_index++){
    res_array[menu_index] += "]";
  }
  
  String response = "{\"DVC\":\""+String(deviceID)+"\","+
    res_array[0] + "," + res_array[1] + ",\"WK\":"+String(heater_working) + ",\"GAP"+"\":"+String(working_total)+"}";
  heater_working = 0;
  working_total = 0;
  return response;
}
void upload_loop(unsigned long millisec){
  if(wifi_able && (millisec - prev_data_post > SECONDE*60*upload_period)){
    prev_data_post  = millisec;
    sensor_upload();
  }
}
void sensor_upload(){
  String response = httpPOSTRequest(server+"device/log",sensor_json());
  //여기서 설정 변경
  uint8_t check_index = 0;
  String cmd_text = String_slice(&check_index,response, 0x2C);
  if (cmd_text == "set"){
    uint8_t set_value = 0;
    bool change_flage = false;
    
    set_value = String_slice(&check_index,response, 0x2C).toInt();
    if(temperature_goal != set_value){
      Serial.println(" O");
      EEPROM.write(eep_temp, set_value);
      temperature_goal = set_value;
      change_flage = true;
    }

    set_value = String_slice(&check_index,response, 0x2C).toInt();
    if((set_value == 1 && !heat_use)||(set_value == 0 && heat_use)){
      EEPROM.write(eep_use_heat, set_value);
      heat_use = set_value;
      change_flage = true;
    }
    if(change_flage){
      EEPROM.commit();
      config_upload();
    }
  }else if(cmd_text == "updt"){
    firmware_upadte();
  }
}
void config_upload(){
  String set_data = "{\"DVC\":\"" + String(deviceID) + "\",\"TMP\":" + String(temperature_goal[index]) + ",\"RUN\":" + String(heat_use) + "}";
  String response = httpPOSTRequest(server+"device/hive_set",set_data);
  Serial.println("http:");
  Serial.println(response);
}
//여기까지 작업 25.11.04
////--------------------- DS3231 ----------------------////
void time_set(){
  String server_time;
  HTTPClient http;
  http.begin("http://www.google.com/");
  const char *headerKeys[] = {"Date"};
  const size_t headerKeysCount = sizeof(headerKeys) / sizeof(headerKeys[0]);
  http.collectHeaders(headerKeys, headerKeysCount);
  http.addHeader("Content-Type", "application/json");
  uint16_t httpResponseCode = http.GET();
  if(httpResponseCode==200||httpResponseCode==201) server_time = http.header("Date");
  http.end(); // Free resources
}
////--------------------- DS3231 ----------------------////
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
////--------------------- sensor data upload ----------////
////--------------------- String_slice ----------------////
String String_slice(uint8_t *check_index, String text, char check_char){
  String response = "";
  for(uint8_t index_check=*check_index; index_check<text.length(); index_check++){
    if(text[index_check] == check_char || text[index_check] == 0x00){
      *check_index = index_check+1;
      break;
    }
    response += text[index_check];
  }
  return response;
}
////--------------------- String_slice ----------------////
////--------------------- firmware_update -------------////
void firmware_upadte() {
  if(wifi_able){
    String serverUrl = server + "firmware/device";   //API adress
    WiFiClient client;
    HTTPClient http;
    http.begin(client, serverUrl);
    // 타임아웃 설정 증가
    http.setTimeout(30000);  // 30초
    
    http.addHeader("Content-Type", "application/json");
    String httpRequestData = (String)"{\"DVC\":\""+String(deviceID)+"\",\"ver\":\"" + firmwareVersion + "\"}";
    int httpResponseCode = http.POST(httpRequestData);
    if (httpResponseCode == 200) {
      int contentLength = http.getSize();
      Serial.printf("Update size: %d\n", contentLength);
      if (contentLength <= 0) {
          Serial.println("Invalid content length");
          http.end();
          return;
      }
      if (!Update.begin(contentLength)) {
          Serial.printf("Not enough space for update. Required: %d\n", contentLength);
          http.end();
          return;
      }
      WiFiClient * stream = http.getStreamPtr();
      // 버퍼 크기 증가 및 타임아웃 처리 추가
      size_t written = 0;
      uint8_t buff[2048] = { 0 };
      int timeout = 0;
      while (written < contentLength) {
          delay(1);  // WiFi 스택에 시간 양보
          size_t available = stream->available();
          if (available) {
              size_t toRead = min(available, sizeof(buff));
              size_t bytesRead = stream->readBytes(buff, toRead);
              if (bytesRead > 0) {
                  size_t bytesWritten = Update.write(buff, bytesRead);
                  if (bytesWritten > 0) {
                      written += bytesWritten;
                      timeout = 0;  // 타임아웃 리셋
                      // 진행률 표시
                      float progress = (float)written / contentLength * 100;
                      Serial.printf("Progress: %.1f%%\n", progress);
                  }
              }
          } else {
              timeout++;
              if (timeout > 100) {  // 약 10초 타임아웃
                  Serial.println("Download timeout");
                  Update.abort();
                  break;
              }
              delay(100);
          }
      }
      if (written == contentLength) {
          if (Update.end(true)) {
              Serial.println("Update Success!");
              ESP.restart();
          } else {
              Serial.printf("Update failed with error: %d\n", Update.getError());
          }
      } else {
          Update.abort();
          Serial.println("Update failed: incomplete download");
      }
  } 
  else if (httpResponseCode == 204) {
      Serial.println("No update available");
  }
  else {
      Serial.printf("HTTP error: %d\n", httpResponseCode);
  }
  http.end();           // Free resources
  }
}////httpPOSTRequest_End

// unsigned long restart_timer = 0;
// uint8_t restart_count       = 0;
// void esp_restart(unsigned long millisec){
//   if(millisec - restart_timer > 1000*60){
//     restart_timer = millisec;
//     if(restart_count++ > 240) ESP.restart();
//   }
// }
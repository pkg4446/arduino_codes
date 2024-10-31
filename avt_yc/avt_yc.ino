//// 2024.10.10
//// http post 로 인터넷 설정 받아오는부분 추가할것.
#include <EEPROM.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <DS3231.h>
#include <max6675.h>
#include <Adafruit_SHT31.h>
#include "uart_print.h"

#define TCAADDR                   0x70
#define TOTAL_TEMPERATURE_SENSOR  5
#define EEPROM_SIZE_CONFIG        24
#define TOTAL_LED                 4
#define COMMAND_LENGTH            32
#define SECONDE                   1000L

#define LED_INTERNET 0
#define LED_MANUAL   1
#define LED_HEATUSE  2
#define LED_HEATER   3

const   String server = "http://yc.beetopia.kro.kr/";
char    deviceID[18];
char    command_buf[COMMAND_LENGTH];
int8_t  command_num;
////--------------------- EEPROM ----------------------////
const uint8_t eep_use_heat    = 0;
const uint8_t upload_interval = 1;
const uint8_t eep_ssid[EEPROM_SIZE_CONFIG] = {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25};
const uint8_t eep_pass[EEPROM_SIZE_CONFIG] = {26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49};
const uint8_t eep_temp[TOTAL_TEMPERATURE_SENSOR] = {50,51,52,53,54};

char  ssid[EEPROM_SIZE_CONFIG];
char  password[EEPROM_SIZE_CONFIG];
////--------------------- EEPROM ----------------------////
////--------------------- Pin out ---------------------////
const uint8_t LED[TOTAL_LED] = {13,14,16,17};
const uint8_t MOSFET[TOTAL_TEMPERATURE_SENSOR] = {4,25,26,27,32};
////--------------------- Pin out ---------------------////
////--------------------- Flage -----------------------////
bool    wifi_able     = false;
bool    manual_mode   = false;
bool    heat_use      = false;
uint8_t upload_period = 5;
uint8_t heater_work   = 0;
////--------------------- Flage -----------------------////
DS3231 RTC_DS3231;
////--------------------- temperature sensor ----------////
Adafruit_SHT31 sht31 = Adafruit_SHT31();
const uint8_t thermoCS    = 5;
const uint8_t thermoCLK   = 18;
const uint8_t thermoDO[TOTAL_TEMPERATURE_SENSOR] = {33,35,34,39,36};
float temperature_sensor_tm[TOTAL_TEMPERATURE_SENSOR] = {0.00f,};
float temperature_sensor_ic[TOTAL_TEMPERATURE_SENSOR] = {0.00f,};
float humidity_sensor_ic[TOTAL_TEMPERATURE_SENSOR]    = {0.00f,};
// MAX6675 객체 배열 선언
MAX6675 thermocouple[TOTAL_TEMPERATURE_SENSOR] = {
  MAX6675(thermoCLK, thermoCS, thermoDO[0]),
  MAX6675(thermoCLK, thermoCS, thermoDO[1]),
  MAX6675(thermoCLK, thermoCS, thermoDO[2]),
  MAX6675(thermoCLK, thermoCS, thermoDO[3]),
  MAX6675(thermoCLK, thermoCS, thermoDO[4])
};
////--------------------- temperature sensor ----------////
////--------------------- temperature control ---------////
const uint8_t temperature_gap = 1;
uint8_t temperature_goal[TOTAL_TEMPERATURE_SENSOR] = {0,};
////--------------------- temperature control ---------////
////--------------------- Interval timer  -------------////
unsigned long prev_update     = 0L;
unsigned long prev_data_post  = 0L;
unsigned long prev_led_heater = 0L;
unsigned long prev_led_toggle = 0L;
bool  flage_led_heater = false;
bool  flage_led_toggle = false;

uint8_t heater_working[TOTAL_TEMPERATURE_SENSOR] = {0,};
////--------------------- Interval timer  -------------////
////--------------------- setup() ---------------------////
void setup()
{
  Serial.begin(115200);
  Wire.begin();
  for (uint8_t index = 0; index < TOTAL_LED; index++){
    pinMode(LED[index], OUTPUT);
    digitalWrite(LED[index], false);
  }
  for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++){
    pinMode(MOSFET[index], OUTPUT);
    digitalWrite(MOSFET[index], false);
  }
  if (!EEPROM.begin((EEPROM_SIZE_CONFIG*2) + TOTAL_TEMPERATURE_SENSOR + 2)){
    Serial.println("Failed to initialise eeprom");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
  bool check_init = false;
  heat_use      = EEPROM.read(eep_use_heat);
  upload_period = EEPROM.read(upload_interval);
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

  for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
    ssid[index]     = EEPROM.read(eep_ssid[index]);
    password[index] = EEPROM.read(eep_pass[index]);
  }

  for (int index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++) {
    temperature_goal[index] = EEPROM.read(eep_temp[index]);
    if(temperature_goal[index] > 50){
      check_init = true;
      uint init_goal = 3;
      EEPROM.write(eep_temp[index], init_goal);
      temperature_goal[index] = init_goal;
    }
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
  Serial.println("System online");
}
////--------------------- setup() ---------------------////
////--------------------- loop() ----------------------////
void loop()
{
  const unsigned long millisec = millis();
  if (Serial.available()) command_process(Serial.read());
  system_control(millisec);
  upload_loop(millisec);
  led_toggle(millisec);
  led_heater(millisec);
}
////--------------------- loop() ----------------------////
////--------------------- system control --------------////
void system_control(unsigned long millisec){
  if(millisec > prev_update + SECONDE){
    prev_update = millisec;
    heater_work = 0;
    temperature_sensor_read();
    if(!manual_mode){
      if(heat_use){
        for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++){
          if(temperature_sensor_tm[index]<temperature_goal[index]-temperature_gap){
            digitalWrite(MOSFET[index], true);
            heater_work += 1;
            heater_working[index] += 1;
          }else{
            digitalWrite(MOSFET[index], false);
          }
        }
      }else{
        for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++){
          digitalWrite(MOSFET[index], false);
        }
      }
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
      Serial.print(temperature_sensor_ic[index]);
      Serial.print("℃,");
      Serial.print(humidity_sensor_ic[index]);
      Serial.println("%");
      Serial.print(index);
      Serial.print(" TM:");
      Serial.print(temperature_sensor_tm[index]);
      Serial.println("℃");
    }
  }else if(cmd_text=="run"){
    bool change_flage = false;
    if(temp_text=="on" && heat_use){
      EEPROM.write(eep_use_heat, true);
      heat_use = true;
      change_flage = true;
    }else if(!heat_use){
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
    uint8_t set_index = temp_text.toInt();
    uint8_t set_value = String_slice(&check_index, command_buf, 0x20).toInt();
    bool change_flage = false;
    if(set_index < TOTAL_TEMPERATURE_SENSOR){
      if(50 > set_value && set_value > 0){
        EEPROM.write(eep_temp[set_index], set_value);
        temperature_goal[set_index] = set_value;
        Serial.println(command_buf);
        change_flage = true;
      }else{
        Serial.println("set value error");
      }
    }else if(set_index == TOTAL_TEMPERATURE_SENSOR){
      if(50 > set_value && set_value > 0){
        for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++){
          EEPROM.write(eep_temp[index], set_value);
          temperature_goal[index] = set_value;
        }
        Serial.println(command_buf);
        change_flage = true;
      }else{
        Serial.println("set value error");
      }
    }else{
      Serial.println("set index error");
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
    for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++)
    {
      Serial.print("temperature setup ");
      Serial.print(index);
      Serial.print(" : ");
      Serial.print(temperature_goal[index]);
      Serial.println(" ℃");
    }
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
      if(set_value < TOTAL_TEMPERATURE_SENSOR){
        digitalWrite(MOSFET[set_value], true);
      }else if(set_value == TOTAL_TEMPERATURE_SENSOR){
        for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++){
          digitalWrite(MOSFET[index], true);
        }
      }
    }else if(temp_text=="off"){
      if(set_value < TOTAL_TEMPERATURE_SENSOR){
        digitalWrite(MOSFET[set_value], false);
      }else if(set_value == TOTAL_TEMPERATURE_SENSOR){
        for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++){
          digitalWrite(MOSFET[index], false);
        }
      }
    }else if(temp_text=="data"){
      sensor_upload();
    }else{
      manual_mode = false;
      Serial.println("Manual mode OFF");
    }
  }else if(cmd_text=="time"){
    time_show();
  }else if(cmd_text=="timeset"){
    if(wifi_able) time_set();
    else Serial.println("wifi not connected");
  }else if(cmd_text=="ssid"){
    wifi_able = false;
    WiFi.disconnect(true);
    Serial.print("ssid: ");
    if(temp_text.length() > 0){
      for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
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
      for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
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
////--------------------- DS3231 ----------------------////
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
  Serial.print('2');
  if(century)Serial.print('1');
  else Serial.print('0');
  if(clock_year<10) Serial.print('0');
  Serial.print(clock_year);Serial.print('/');Serial.print(clock_month);Serial.print('/');Serial.print(clock_day);Serial.print(',');
  Serial.print(clock_hour);Serial.print(':');Serial.print(clock_min);Serial.print(':');Serial.println(clock_sec);
}
////---------------------------------------------------////
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
    Serial.println("err");
  }
  http.end(); // Free resources
}
////--------------------- DS3231 ----------------------////
////--------------------- temperature read ------------////
void tcaselect(uint8_t index) {
  if (index > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << index);
  Wire.endTransmission();
}
void temperature_sensor_read(){
  for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++){
    temperature_sensor_tm[index] = thermocouple[index].readCelsius();
    tcaselect(index);
    Wire.beginTransmission(68);
    if (!Wire.endTransmission() && sht31.begin(0x44)) {
      temperature_sensor_ic[index] = sht31.readTemperature();
      humidity_sensor_ic[index]    = sht31.readHumidity();
    }else{
      temperature_sensor_ic[index] = NAN;
      humidity_sensor_ic[index]    = NAN;
    }
  }
}
////--------------------- temperature read ------------////
////--------------------- sensor data upload ----------////
String sensor_json(){
  temperature_sensor_read();

  const uint8_t post_menu = 4;
  String res_array[post_menu] = {"\"HM\":[","\"IC\":[","\"TM\":[","\"WK\":["};

  for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++){
    res_array[0] += "\""+String(humidity_sensor_ic[index])+"\"";
    res_array[1] += "\""+String(temperature_sensor_ic[index])+"\"";
    res_array[2] += "\""+String(temperature_sensor_tm[index])+"\"";
    res_array[3] += String(heater_working[index]);
    if(index < TOTAL_TEMPERATURE_SENSOR-1){
      for (uint8_t menu_index = 0; menu_index < post_menu; menu_index++){
        res_array[menu_index] += ",";
      }
    }
    heater_working[index] = 0;
  }
  for (uint8_t menu_index = 0; menu_index < post_menu; menu_index++){
    res_array[menu_index] += "]";
  }
  
  String response = "{\"DVC\":\""+String(deviceID)+"\","+
    res_array[0] + "," + res_array[1] + "," + res_array[2] + "," + res_array[3] + "," +
    "\"GAP"+"\":"+String(upload_period)+"}";
  return response;
}
void upload_loop(unsigned long millisec){
  if(wifi_able && (millisec > prev_data_post + SECONDE*60*upload_period)){
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
    for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++){
      set_value = String_slice(&check_index,response, 0x2C).toInt();
      if(temperature_goal[index] != set_value){
        Serial.println(" O");
        EEPROM.write(eep_temp[index], set_value);
        temperature_goal[index] = set_value;
        change_flage = true;
      }
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
  }else{}
}
void config_upload(){
  String set_data = "{\"DVC\":\""+String(deviceID)+"\",\"TMP\":[";
  for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++){
    set_data += String(temperature_goal[index]);
    if(index < TOTAL_TEMPERATURE_SENSOR-1) set_data += ",";
  }
  set_data += "],\"RUN\":" + String(heat_use) + "}";
  String response = httpPOSTRequest(server+"device/hive_set",set_data);
  Serial.println("http:");
  Serial.println(response);
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
////--------------------- sensor data upload ----------////
////--------------------- LED_toggle ------------------////
void led_toggle(unsigned long millisec){
  if(millisec > prev_led_toggle + SECONDE){
    prev_led_toggle  = millisec;
    flage_led_toggle = true;
    if(wifi_able)   digitalWrite(LED[LED_INTERNET], true);
    if(manual_mode) digitalWrite(LED[LED_MANUAL], true);
    if(heat_use)    digitalWrite(LED[LED_HEATUSE], true);
  }else if(flage_led_toggle && millisec > prev_led_toggle + SECONDE - 700){
    flage_led_toggle = false;
    for (uint8_t index = 0; index < TOTAL_LED; index++){
      digitalWrite(LED[index], false);
    }
  }
}
void led_heater(unsigned long millisec){
  if(heater_work>0 && millisec > prev_led_heater + SECONDE){
    prev_led_heater  = millisec;
    flage_led_heater = true;
    digitalWrite(LED[LED_HEATER], true);
  }else if(flage_led_heater && millisec > prev_led_heater + heater_work*200){
    flage_led_heater = false;
    digitalWrite(LED[LED_HEATER], false);
  }
}
////--------------------- LED_toggle ------------------////
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
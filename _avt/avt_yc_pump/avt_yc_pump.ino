#include <Update.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <DS3231.h>
#include <Adafruit_SHT31.h>
#include <HardwareSerial.h>
#include "uart_print.h"

String firmwareVersion = "0.0.1";

#define TOTAL_LED       3
#define TOTAL_CONTROL   3
#define OUTPUT          5
#define EEPROM_SIZE     24
#define COMMAND_LENGTH  32
#define SECONDE         1000L

#define LED_INTERNET  0
#define HEATER        0
#define VALVE_A       1
#define VALVE_B       2

#define SONA      2
#define SONA_DATA 4

// UART 핀 설정
#define SENSOR1_RX 13
#define SENSOR1_TX 14
#define SENSOR2_RX 16
#define SENSOR2_TX 17
// 하드웨어 UART 인스턴스 생성
HardwareSerial ultrasonic1(2); // UART1 (GPIO13, GPIO14)
HardwareSerial ultrasonic2(1); // UART2 (GPIO16, GPIO17)

const   String server = "http://192.168.1.36:3002/";
// const   String server = "http://yc.beetopia.kro.kr/";
char    deviceID[18];
char    command_buf[COMMAND_LENGTH];
int8_t  command_num;
////--------------------- EEPROM ----------------------////
const uint8_t eep_ssid = 0;
const uint8_t eep_pass = 24;
const uint8_t upload_interval = 48;
const uint8_t eep_time_on   = 49;
const uint8_t eep_time_off  = 52;

char  ssid[EEPROM_SIZE];
char  password[EEPROM_SIZE];
////--------------------- EEPROM ----------------------////
////--------------------- Pin out ---------------------////
const uint8_t LED[TOTAL_LED] = {4,5,33};
const uint8_t MOSFET[OUTPUT] = {23,25,26,27,32};
////--------------------- Pin out ---------------------////
////--------------------- Flage -----------------------////
bool  wifi_able       = false;
bool  manual_mode     = false;
bool  ctr_state[TOTAL_CONTROL]  = {false,};
uint8_t time_on[TOTAL_CONTROL]  = {0,};
uint8_t time_off[TOTAL_CONTROL] = {0,};
uint8_t upload_period   = 5;
////--------------------- Flage -----------------------////
DS3231 RTC_DS3231;
////--------------------- sensor temperature ----------////
Adafruit_SHT31 sht31 = Adafruit_SHT31();
float sensor_temperature    = 0.00f;
float sensor_humidity       = 0.00f;
////--------------------- sensor temperature ----------////
////--------------------- sensor sona -----------------////
float   sensor_ultrasonic[SONA]      = {NAN,};
uint8_t sona_buffer[SONA][SONA_DATA] = {{0,},};
bool    sona_flage[SONA]    = {false,};
uint8_t sona_index[SONA]    = {0,};
////--------------------- sensor sona -----------------////
////--------------------- Interval timer --------------////
unsigned long prev_update       = 0L;
unsigned long prev_led_toggle   = 0L;
unsigned long prev_sensor_read  = 0L;
unsigned long prev_data_post    = 0L;
unsigned long prev_reconnect    = 0L;
bool  flage_led_toggle = false;
////--------------------- Interval timer --------------////
////--------------------- Serial command --------------////
void Serial_command(){if(Serial.available()) command_process(Serial.read());}
////--------------------- Serial command --------------////
////--------------------- setup() ---------------------////
void setup()
{
  Serial.begin(115200);
  ultrasonic1.begin(9600, SERIAL_8N1, SENSOR1_RX, SENSOR1_TX);
  ultrasonic2.begin(9600, SERIAL_8N1, SENSOR2_RX, SENSOR2_TX);
  Wire.begin();
  Serial.print("ver:");
  Serial.println(firmwareVersion);
  for (uint8_t index = 0; index < TOTAL_LED; index++){
    pinMode(LED[index], OUTPUT);
    digitalWrite(LED[index], false);
  }
  for (uint8_t index = 0; index < OUTPUT; index++){
    pinMode(MOSFET[index], OUTPUT);
    digitalWrite(MOSFET[index], false);
  }
  if (!EEPROM.begin((EEPROM_SIZE*2) + TOTAL_CONTROL*2 + 1)){
    Serial.println("Failed to initialise eeprom");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }

  for (int index = 0; index < EEPROM_SIZE; index++) {
    ssid[index]     = EEPROM.read(eep_ssid+index);
    password[index] = EEPROM.read(eep_pass+index);
  }

  bool check_init = false;
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

  for (uint8_t index = 0; index < OUTPUT; index++){
    pinMode(MOSFET[index], OUTPUT);
    digitalWrite(MOSFET[index], false);
  }

  for (int index = 0; index < TOTAL_CONTROL; index++) {
    time_on[index]  = EEPROM.read(eep_time_on+ index);
    time_off[index] = EEPROM.read(eep_time_off+index);
    if(time_on[index]> 24) time_on[index]  = 0;
    if(time_off[index]>24) time_off[index] = 0;
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
  sensor_sona();
  sensor_sht(millisec);
  wifi_reconnect(millisec);
  // loop_upload(millisec);
  system_control(millisec);
  led_toggle(millisec);
}
////--------------------- loop() ----------------------////`
////--------------------- system control --------------////
void system_control(unsigned long millisec){
  if(millisec - prev_update > SECONDE){
    prev_update = millisec;
  }
}
////--------------------- system control --------------////
////--------------------- sensor reads ----------------////
void sensor_sht(unsigned long millisec){
  if(millisec - prev_sensor_read > SECONDE){
    prev_sensor_read = millisec;
    sensor_temperature_read();
  }
}
void sensor_sona(){
  sensor_ultrasonic_read(ultrasonic1,0,SENSOR1_RX, SENSOR1_TX);
  sensor_ultrasonic_read(ultrasonic2,1,SENSOR2_RX, SENSOR2_TX);
}
////--------------------- sensor reads ----------------////
////--------------------- command ---------------------////
void command_service(){
  bool    eep_change  = false;
  uint8_t check_index = 0;
  String cmd_text  = String_slice(&check_index, command_buf, 0x20);
  String temp_text = String_slice(&check_index, command_buf, 0x20);
  ////cmd start
  if(cmd_text=="reboot"){
    ESP.restart();
  }else if(cmd_text=="gap"){
    uint8_t set_value = temp_text.toInt();
    EEPROM.write(upload_interval, set_value);
    upload_period = set_value;
    EEPROM.commit();
    Serial.print("upload interval ");
    Serial.println(upload_period);
  }else if(cmd_text=="time"){
    time_show();
  }else if(cmd_text=="sensor"){
    for(uint8_t index=0; index<2; index++){
      Serial.print("ultrasonic Sensor ");
      Serial.print(index);
      Serial.print(" Distance: ");
      Serial.print(sensor_ultrasonic[index]);
      Serial.println(" cm");
    }
    Serial.print("sht: ");
    Serial.print(sensor_temperature);
    Serial.print("℃,");
    Serial.print(sensor_humidity);
    Serial.println("%");
  }else if(cmd_text=="valve"){
    uint8_t valve = HEATER;
    if(temp_text == "a") valve = VALVE_A;
    if(temp_text == "b") valve = VALVE_B;
    if(valve != HEATER){
      uint8_t set_on  = String_slice(&check_index, command_buf, 0x20).toInt();
      uint8_t set_off = String_slice(&check_index, command_buf, 0x20).toInt();
      if(set_on>23)  set_on  = 0;
      if(set_off>23) set_off = 0;
      EEPROM.write(eep_time_on+ valve, set_on);
      EEPROM.write(eep_time_off+valve, set_on);
      time_on[valve]  = set_on;
      time_off[valve] = set_off;
      Serial.println(command_buf);
      EEPROM.commit();
      config_upload(cmd_text,temp_text,valve);
    }else{
      Serial.println("valve error");
    }
  }else if(cmd_text=="heat"){
    uint8_t set_on  = temp_text.toInt();
    uint8_t set_off = String_slice(&check_index, command_buf, 0x20).toInt();
    if(set_on>23)  set_on  = 0;
    if(set_off>23) set_off = 0;
    EEPROM.write(eep_time_on,  set_on);
    EEPROM.write(eep_time_off, set_on);
    time_on[HEATER]  = set_on;
    time_off[HEATER] = set_off;
    Serial.println(command_buf);
    EEPROM.commit();
    config_upload(cmd_text,"",HEATER);
  }else if(cmd_text=="config"){
    if(wifi_able) Serial.println("WIFI on");
    else Serial.println("WIFI off");
    Serial.print("upload interval : ");
    Serial.print(upload_period);
    Serial.println(" Min");
    for (uint8_t index = 0; index < OUTPUT; index++)
    {
      if(index == HEATER){
        Serial.print("Heater: ");
      }else{
        Serial.print("VALVE ");
        if(index == VALVE_A) Serial.print("A: ");
        else Serial.print("B: ");
      }
      Serial.print("on:");
      Serial.print(time_on[index]);
      Serial.print(", off:");
      Serial.println(time_off[index]);
    }
  }else if(cmd_text=="test"){
    uint8_t set_value = String_slice(&check_index, command_buf, 0x20).toInt();
    if(temp_text=="mode"){
      manual_mode = true;
      Serial.println("Manual mode ON");
    }else if(temp_text=="on"){
      if(set_value < OUTPUT){
        uint8_t valve_delay = 50;
        if(set_value == OUTPUT-1){
          valve_delay = 0;
        }else if(set_value%2 == 1){
          digitalWrite(MOSFET[set_value-1], false);
        }else{
          digitalWrite(MOSFET[set_value+1], false);
        }
        delay(valve_delay);
        digitalWrite(MOSFET[set_value], true);
      }
    }else if(temp_text=="off"){
      if(set_value < OUTPUT){
        digitalWrite(MOSFET[set_value], false);
      }else if(set_value == OUTPUT){
        for (uint8_t index = 0; index < OUTPUT; index++){
          digitalWrite(MOSFET[index], false);
        }
      }
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
      for (int index = 0; index < EEPROM_SIZE; index++) {
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
      for (int index = 0; index < EEPROM_SIZE; index++) {
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
    if(update_time - wifi_config_update > SECONDE*10){
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
    if(WiFi.status() != WL_CONNECTED) wifi_connect();
  }
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
void sensor_temperature_read(){
  Wire.beginTransmission(68);
  if (!Wire.endTransmission() && sht31.begin(0x44)) {
    sensor_temperature = sht31.readTemperature();
    sensor_humidity    = sht31.readHumidity();
  }else{
    sensor_temperature = NAN;
    sensor_humidity    = NAN;
  }
}
void sensor_ultrasonic_read(HardwareSerial &sensorSerial, uint8_t sensor_index, uint8_t rxPin, uint8_t txPin) {
  if(sensorSerial.available()){
    uint8_t get_serial = sensorSerial.read();
    if(sona_index[sensor_index] >= SONA_DATA) sona_index[sensor_index] = 0;

    if (get_serial == 255){
      sona_flage[sensor_index] = false;
      sona_index[sensor_index] = 0;
    }else if(sona_index[sensor_index] == SONA_DATA-1){
      sona_flage[sensor_index] = true;
    }
    sona_buffer[sensor_index][sona_index[sensor_index]++] = get_serial;
  }

  if (sona_flage[sensor_index]) {
    uint16_t  distance = 0;
    distance = (sona_buffer[sensor_index][1] << 8) | sona_buffer[sensor_index][2];
    sensor_ultrasonic[sensor_index] = float(distance)/10.0;
  }
}
////--------------------- sensor data upload ----------////
String sensor_json(){  
  String response = "{\"DVC\":\""+String(deviceID);
  response += "\",\"DATA\":{\"temp\":\""+String(sensor_temperature);
  response += "\",\"humi\":\"" + String(sensor_humidity);
  response += "\",\"sona1\":\"" + String(sensor_ultrasonic[0]);
  response += "\",\"sona2\":\"" + String(sensor_ultrasonic[1])+"\"}}";
  return response;
}
void loop_upload(unsigned long millisec){
  if(wifi_able && (millisec - prev_data_post > SECONDE*60*upload_period)){
    prev_data_post  = millisec;
    sensor_upload();
    //업로드 마다 시계 확인하고 업데이트 하기
  }
}
void sensor_upload(){
  String response = httpPOSTRequest(server+"device/pump",sensor_json());
  //여기서 설정 변경
  uint8_t check_index = 0;
  String cmd_text = String_slice(&check_index,response, 0x2C);
  if (cmd_text == "set"){
    bool change_flage = false;
    uint8_t set_value = String_slice(&check_index,response, 0x2C).toInt();;
    // if(change_flage){
    //   EEPROM.commit();
    //   config_upload();
    // }
  }else if(cmd_text == "updt"){
    firmware_upadte();
  }
}
void config_upload(String ctr,String type,uint8_t index){
  String set_data = "{\"DVC\":\""+String(deviceID)+"\",\"CTR\":\""+ctr+"\",\"TYPE\":\""+type;
        set_data += "\",\"TIME\":[" + String(time_on[index]) + ","+String(time_off[index]) + "]}";
  String response = httpPOSTRequest(server+"device/hive_set",set_data);
  Serial.print("http:");
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
  if(millisec - prev_led_toggle > SECONDE){
    prev_led_toggle  = millisec;
    flage_led_toggle = true;
    if(wifi_able)   digitalWrite(LED[LED_INTERNET], true);
  }else if(flage_led_toggle && millisec - prev_led_toggle > SECONDE - 700){
    flage_led_toggle = false;
    for (uint8_t index = 0; index < TOTAL_LED; index++){
      digitalWrite(LED[index], false);
    }
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
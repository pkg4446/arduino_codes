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
#define RLY_OUTPUT      5
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

// const   String server = "http://192.168.1.36:3002/";
const   String server = "http://yc.beetopia.kro.kr/";
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
const uint8_t RELAY[RLY_OUTPUT]  = {23,25,26,27,32};
////--------------------- Pin out ---------------------////
////--------------------- Flage -----------------------////
bool  wifi_able       = false;
bool  manual_mode     = false;
bool  ctr_state[TOTAL_CONTROL]  = {false,};
bool  Hbridge[RLY_OUTPUT-1]     = {false,};
uint8_t valve_count[VALVE_B]    = {0,};

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
unsigned long prev_vlave_relay  = 0L;
unsigned long prev_led_toggle   = 0L;
unsigned long prev_sensor_read  = 0L;
unsigned long prev_data_post    = 0L;
unsigned long prev_reconnect    = 0L;
bool  flage_led_toggle = false;
////--------------------- Interval timer --------------////
////--------------------- Serial command --------------////
void Serial_command(){if(Serial.available()) command_process(Serial.read());}
////--------------------- Serial command --------------////
////--------------------- clock_array -----------------////
String str_dow[7]   = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
String str_month[12]= {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
////--------------------- clock_array -----------------////
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
  for (uint8_t index = 0; index < RLY_OUTPUT; index++){
    pinMode(RELAY[index], OUTPUT);
    digitalWrite(RELAY[index], false);
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
  for (int index = 0; index < TOTAL_CONTROL; index++) {
    time_on[index]  = EEPROM.read(eep_time_on +index);
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
  loop_upload(millisec);
  if(!manual_mode){
    system_control(millisec);
    //릴레이가 충분한 시간을 두고 스위칭 할 수 있도록 valve_relay_off 가 system_control 밑에 있어야 함.
    valve_relay_off(millisec);
  }
  led_toggle(millisec);
}
////--------------------- loop() ----------------------////`
////--------------------- system control --------------////
void valve_relay_off(unsigned long millisec){
  if(millisec - prev_vlave_relay > SECONDE){
    prev_vlave_relay = millisec;
    for(uint8_t index=0; index<VALVE_B; index++){
      if(Hbridge[index*2]||Hbridge[index*2+1]){
        if(valve_count[index]<10){
          valve_count[index] += 1;
        }else {
          Hbridge[index*2]   = false;
          Hbridge[index*2+1] = false;
          digitalWrite(RELAY[index*2], false);
          digitalWrite(RELAY[index*2+1], false);
        }
      }
    }
  }
}
////--------------------- valve check -----------------////
void valve_3way(uint8_t valve,bool valve_state){
  if(ctr_state[valve] != valve_state){
    ctr_state[valve] = valve_state;
    valve -= 1;
    valve_count[valve] = 0;
    if(valve_state){
      //on
      if(Hbridge[valve*2+1]){
        digitalWrite(RELAY[valve*2+1], false);
        Hbridge[valve*2+1] = false;
        delay(500);
      }
      digitalWrite(RELAY[valve*2], true);
      Hbridge[valve*2] = true;
    }else{
      //off
      if(Hbridge[valve*2]){
        digitalWrite(RELAY[valve*2], false);
        delay(500);
      }
      digitalWrite(RELAY[valve*2+1], true);
      Hbridge[valve*2+1] = true;
    }
  }
}
////--------------------- valve onoff -----------------////
void system_control(unsigned long millisec){
  if(millisec - prev_update > SECONDE){
    prev_update = millisec;
    const uint8_t time_hour = korean_time();
    bool output_state[TOTAL_CONTROL] = {false,};
    for (uint8_t index = 0; index < TOTAL_CONTROL; index++){
      if(time_on[index] != time_off[index]){
        if(time_on[index] < time_off[index]){
          if(time_on[index]<=time_hour && time_hour<time_off[index]) output_state[index] = true;
          else output_state[index] = false;
        }else{
          if(time_off[index]<=time_hour && time_hour<time_on[index]) output_state[index] = false;
          else output_state[index] = true;
        }
      }else{
        output_state[index] =  false;
      }
    }

    if(ctr_state[HEATER] != output_state[HEATER]){
      ctr_state[HEATER] = output_state[HEATER];
      digitalWrite(RELAY[RLY_OUTPUT-1], ctr_state[HEATER]);
    }
    valve_3way(VALVE_A,output_state[VALVE_A]);
    valve_3way(VALVE_B,output_state[VALVE_B]);
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
    if(set_value==0) set_value = 1;
    EEPROM.write(upload_interval, set_value);
    upload_period = set_value;
    EEPROM.commit();
    Serial.print("upload interval ");
    Serial.println(upload_period);
  }else if(cmd_text=="time"){
    Serial.println(time_show(true));
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
      EEPROM.write(eep_time_on +valve, set_on);
      EEPROM.write(eep_time_off+valve, set_off);
      time_on[valve]  = set_on;
      time_off[valve] = set_off;
      Serial.println(command_buf);
      EEPROM.commit();
      config_upload();
    }else{
      Serial.println("valve error");
    }
  }else if(cmd_text=="heat"){
    uint8_t set_on  = temp_text.toInt();
    uint8_t set_off = String_slice(&check_index, command_buf, 0x20).toInt();
    if(set_on>23)  set_on  = 0;
    if(set_off>23) set_off = 0;
    EEPROM.write(eep_time_on,  set_on);
    EEPROM.write(eep_time_off, set_off);
    time_on[HEATER]  = set_on;
    time_off[HEATER] = set_off;
    Serial.println(command_buf);
    EEPROM.commit();
    config_upload();
  }else if(cmd_text=="config"){
    if(wifi_able) Serial.println("WIFI on");
    else Serial.println("WIFI off");
    Serial.print("upload interval : ");
    Serial.print(upload_period);
    Serial.println(" Min");
    for (uint8_t index = 0; index < TOTAL_CONTROL; index++)
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
    }else if(temp_text=="on" && manual_mode){
      if(set_value < RLY_OUTPUT){
        uint8_t valve_delay = 0;
        Serial.print("relay ");
        if(set_value != RLY_OUTPUT-1){
          bool    str_print = false;
          uint8_t relay_num = 0;
          if(set_value%2 == 1){
            if(Hbridge[set_value-1]){
              digitalWrite(RELAY[set_value-1], false);
              Hbridge[set_value-1] = false;
              relay_num = set_value-1;
              str_print = true;
            }
          }else{
            if(Hbridge[set_value+1]){
              digitalWrite(RELAY[set_value+1], false);
              Hbridge[set_value+1] = false;
              relay_num = set_value+1;
              str_print = true;
            }
          }
          if(str_print){
            valve_delay = 500;
            Serial.print("OFF: ");
            Serial.print(relay_num);
            Serial.print(", ");
          }
        }else{
          ctr_state[RLY_OUTPUT-1] = true;
        }
        delay(valve_delay);
        digitalWrite(RELAY[set_value], true);
        Hbridge[set_value] = true;
        Serial.print("ON: ");
        Serial.println(set_value);
      }else{
        Serial.println("relay 0~4");
      }
    }else if(temp_text=="off" && manual_mode){
      Serial.print("relay OFF: ");
      if(set_value < RLY_OUTPUT){
        if(set_value < RLY_OUTPUT-1) Hbridge[set_value] = false;
        else ctr_state[RLY_OUTPUT-1] = false;
        digitalWrite(RELAY[set_value], false);
        Serial.println(set_value);
      }else if(set_value == RLY_OUTPUT){
        ctr_state[RLY_OUTPUT-1] = false;
        for (uint8_t index = 0; index < RLY_OUTPUT; index++){
          digitalWrite(RELAY[index], false);
        }
        Serial.println("ALL");
      }else{
        Serial.println("relay 0~4");
      }
    }else if(temp_text=="data"){
      sensor_upload();
    }else if(!manual_mode){
      Serial.println("Manual mode OFF");
    }else{
      manual_mode = false;
      ctr_state[RLY_OUTPUT-1] = false;
      for (uint8_t index = 0; index < RLY_OUTPUT; index++){
        digitalWrite(RELAY[index], false);
      }
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
uint8_t korean_time(){
  bool h12Flag;
  bool pmFlag;
  uint8_t clock_hour = RTC_DS3231.getHour(h12Flag, pmFlag)+9;
  if(clock_hour>23) clock_hour = 0;
  return clock_hour;
}
////--------------------- DS3231 ----------------------////
String time_show(bool type){
  String response = "";

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

  if(type){
    //한국 기준 시간으로 변환
    clock_hour += 9;
    if(clock_hour > 23){
      clock_hour = 0;
      clock_day += 1;
      clock_dow += 1;
      if(clock_dow > 7) clock_dow = 1;
    }
  }
  
  response += str_dow[clock_dow-1]+", "+String(clock_day)+" "+str_month[clock_month-1]+" 2";
  if(century) response += "1";
  else response += "0";
  if(clock_year<10) response += '0';
  response += String(clock_year)+" ";
  if(clock_hour<10) response += '0';
  response += String(clock_hour)+":";
  if(clock_min<10) response += '0';
  response += String(clock_min)+":";
  if(clock_sec<10) response += '0';
  response += String(clock_sec)+" GMT";

  return response;
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
  }
}
void sensor_upload(){
  String response = httpPOSTRequest(server+"device/pump",sensor_json());
  Serial.print("res:");
  Serial.println(response);
  //여기서 설정 변경
  uint8_t check_index = 0;
  String cmd_text = String_slice(&check_index,response, 0x2C);
  if (cmd_text == "set"){
    bool commint_flage                = false;
    for (uint8_t index = 0; index < TOTAL_CONTROL; index++){
      uint8_t set_time_on   = String_slice(&check_index,response, 0x2C).toInt();
      uint8_t set_time__off = String_slice(&check_index,response, 0x2C).toInt();
      if(time_on[index] != set_time_on){
        commint_flage = true;
        time_on[index] = set_time_on;
        EEPROM.write(eep_time_on +index, set_time_on);
      }
      if(time_off[index] != set_time__off){
        commint_flage = true;
        time_off[index] = set_time__off;
        EEPROM.write(eep_time_off+index, set_time__off);
      }
    }
    if(commint_flage){
      EEPROM.commit();
      config_upload();
    }
  }else if(cmd_text == "updt"){
    firmware_upadte();
  }
}
void config_upload(){
  String set_data = "{\"DVC\":\""+String(deviceID)+"\",\"SET\":\"";
    for (uint8_t index=0; index < TOTAL_CONTROL; index++){
      if(index != 0) set_data += ",";
      set_data += String(time_on[index]) + ","+String(time_off[index]);
    }
    set_data += "\"}";
  String response = httpPOSTRequest(server+"device/pump_set",set_data);
  Serial.print("code:");
  Serial.println(response);
}
void server_time_parser(String server_time){
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
  for(uint8_t index=0; index<7; index++){
    if(str_dow[index]+"," == time_data[0]) time_dow = index+1;
  }
  time_day = (time_data[1].toInt())%100;
  for(uint8_t index=0; index<12; index++){
    if(str_month[index] == time_data[2]) time_month = index+1;
  }
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
  //시간설정
  RTC_DS3231.setClockMode(false);
  RTC_DS3231.setYear(time_year);
  RTC_DS3231.setMonth(time_month);
  RTC_DS3231.setDate(time_day);
  RTC_DS3231.setDoW(time_dow);
  RTC_DS3231.setHour(time_time[0]);
  RTC_DS3231.setMinute(time_time[1]);
  RTC_DS3231.setSecond(time_time[2]);
}
String httpPOSTRequest(String server_url, String send_data) {
  String response = "";
  if(wifi_able){
    String server_time = "";
    WiFiClient client;
    HTTPClient http;
    http.begin(client, server_url);
    const char *headerKeys[] = {"Date"};
    const size_t headerKeysCount = sizeof(headerKeys) / sizeof(headerKeys[0]);
    http.collectHeaders(headerKeys, headerKeysCount);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000);
    int response_code = http.POST(send_data);
    if(response_code==200||response_code==201){
      response = http.getString();
      server_time = http.header("Date");
    }
    http.end();
    Serial.print("http:");
    Serial.println(response_code);

    if(response_code==200||response_code==201){
      if(server_time != time_show(false)) server_time_parser(server_time);
    }
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
    if(ctr_state[VALVE_A]) digitalWrite(LED[VALVE_A], true);
    if(ctr_state[VALVE_B]) digitalWrite(LED[VALVE_B], true);
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
    String serverUrl = server + "firmware/pump";   //API adress
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
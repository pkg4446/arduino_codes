#include <Wire.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "filesys_esp.h"
#include "uart_print.h"

#define UPDATE_INTERVAL_SENSOR  300L
#define UPDATE_INTERVAL_HTTP    60000L
#define UPDATE_INTERVAL_CSV     60000L

#define TCA9548A_COUNT  7
#define SENSOR_COUNT    50
#define MOVING_AVERAGE  10

#define COMMAND_LENGTH  32
#define WIFI_WAIT       3000

#define EEPROM_SIZE_CONFIG  24

#define SDA_PIN 21
#define SCL_PIN 22

/*********************************************************/
const uint8_t TMP112_ADDRESS = 0x48; // TMP112 온도 센서 주소
byte tcaAddresses[TCA9548A_COUNT] = {0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76}; // 멀티플렉서 주소
const uint8_t eep_ssid[EEPROM_SIZE_CONFIG] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
const uint8_t eep_pass[EEPROM_SIZE_CONFIG] = {24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47};
String http_server_addr = "http://50.beetopia.kro.kr/device/log";
String path_savedata    = "/data";
char  deviceID[18];
/*********************************************************/
bool able_sdcard = false;
bool able_wifi   = false;
/*********************************************************/
int16_t temperatures[SENSOR_COUNT][MOVING_AVERAGE];
char    ssid[EEPROM_SIZE_CONFIG];
char    password[EEPROM_SIZE_CONFIG];
char    command_buf[COMMAND_LENGTH];
int8_t  command_num = 0;
/*********************************************************/
unsigned long pre_sensor_read = 0UL;
unsigned long pre_update_post = 0UL;
unsigned long pre_save_csv    = 0UL;
uint8_t  index_sensor   = 0;
uint8_t  index_average  = 0;
/*********************************************************/
void tcaSelect(uint8_t tcaAddress, uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(tcaAddress);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void tcaDisable(uint8_t tcaAddress) {
  Wire.beginTransmission(tcaAddress);
  Wire.write(0);
  Wire.endTransmission();
}

int16_t readTemp() {
  Wire.requestFrom(TMP112_ADDRESS, 2);
  if (Wire.available() == 2) {
    int16_t raw = (Wire.read() << 8) | Wire.read();
    raw >>= 4; // TMP112는 12비트 해상도, 따라서 4비트 쉬프트
    return int16_t(raw * 6.25); // TMP112 온도 변환
  } else {
    return 9999; // 읽기 실패
  }
}

void sensor_mapping(unsigned long millisec){
  if(millisec > pre_sensor_read + UPDATE_INTERVAL_SENSOR){//runtime : 32 millisec
    uint8_t  index_sensor = 0;
    // 모든 센서 값 읽기
    for (uint8_t index = 0; index < TCA9548A_COUNT; index++) {
      uint8_t channelLimit = (index == TCA9548A_COUNT - 1) ? 2 : 8; // 마지막 멀티플렉서는 2개의 채널만 사용
      for (uint8_t ch = 0; ch < channelLimit; ch++) {
        tcaSelect(tcaAddresses[index], ch);
        temperatures[index_sensor++][index_average] = readTemp();
      }
      tcaDisable(tcaAddresses[index]); // 각 멀티플렉서 비활성화
    }
    if(++index_average>=MOVING_AVERAGE) index_average = 0;
  }
}

void sensor_value_init(){
  for (uint8_t index = 0; index < SENSOR_COUNT; index++) {
    for (uint8_t average = 1; average < MOVING_AVERAGE; average++) {
      temperatures[index][average] = temperatures[index][0];
    }
  }
}

String sensor_json(){
  String response = "{\"dvid\":\""+String(deviceID)+"\"";
  for (uint8_t row = 0; row < 5; row++){
    response += ",\"row"+String(row)+"\":[";
    for (uint8_t col = 0; col < 10; col++){
      uint8_t index = col*5 + row;
      int32_t sensor_temperature = 0;
      for (uint8_t average = 0; average < MOVING_AVERAGE; average++) {
        sensor_temperature += temperatures[index][average];
      }
      sensor_temperature /= MOVING_AVERAGE;
      if (sensor_temperature == 9999) response += "-404";
      else response += String(sensor_temperature);
      if(col<9) response += ",";
    }
    response += "]";
  }
  response += "}";
  return response;
}

String sensor_csv(bool type_save){
  String response = "";
  for (uint8_t row = 0; row < 5; row++){
    for (uint8_t col = 0; col < 10; col++){
      uint8_t index = col*5 + row;
      int32_t sensor_temperature = 0;
      for (uint8_t average = 0; average < MOVING_AVERAGE; average++) {
        sensor_temperature += temperatures[index][average];
      }
      sensor_temperature /= MOVING_AVERAGE;
      if (sensor_temperature == 9999) {
        response += "NULL";
      } else {
        response += String(sensor_temperature);
      }
      if(col<9){
        response += ",";
      }else{
        if(type_save)   response += "\n";
        else if(row<4)  response += ",";
      }
    }
  }
  response += "\n";
  return response;
}
/*********************************************************/
void WIFI_scan(bool wifi_state){
  able_wifi = wifi_state;
  WiFi.disconnect(true);
  Serial.println("WIFI Scanning…");
  uint8_t networks = WiFi.scanNetworks();
  if (networks == 0) {
    Serial.println("WIFI not found!");
  }else {
    Serial.print(networks);
    Serial.println(" networks found");
    Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
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
  }
  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();
  if(able_wifi){
    wifi_connect();
  }else{
    WiFi.disconnect(true);
  }
}
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
  Serial.print(cmd_text);
  Serial.print(", ");
  Serial.println(temp_text);

  if(cmd_text=="reboot"){
    ESP.restart();
  }else if(cmd_text=="sensor"){
    Serial.println(sensor_csv(true));
  }else if(cmd_text=="ssid"){
    able_wifi = false;
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
      eep_change = true;
    }
    Serial.println("");
  }else if(cmd_text=="pass"){
    able_wifi = false;
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
      eep_change = true;
    }
    Serial.println("");
  }else if(cmd_text=="wifi"){
    if(temp_text=="stop"){
      able_wifi = false;
      Serial.print("WIFI disconnect");
      WiFi.disconnect(true);
    }else if(temp_text=="scan"){
      WIFI_scan(WiFi.status() == WL_CONNECTED);
    }else{
      wifi_connect();
    }
  }else if(cmd_text=="help"){
    serial_command_help(&Serial);
  }else{ serial_err_msg(&Serial, command_buf); }

  if(eep_change){
    EEPROM.commit();
  }
}
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
/******************************************/
void wifi_connect() {
  able_wifi = true;
  serial_wifi_config(&Serial,ssid,password);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long wifi_config_update  = millis();
  while (WiFi.status() != WL_CONNECTED) {
    unsigned long update_time = millis();
    if(update_time - wifi_config_update > WIFI_WAIT){
      able_wifi = false;
      Serial.println("WIFI fail");
      WiFi.disconnect(true);
      break;
    }
  }
  for (int index = 0; index < 17; index++) {
    if(WiFi.macAddress()[index]==':'){
      deviceID[index] = '_';
    }else{
      deviceID[index] = WiFi.macAddress()[index];
    }
  }
  Serial.println("WIFI connected");
  Serial.print("deviceID:");
  Serial.println(deviceID);
}
/*********************************************************/
void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000); // I2C 속도를 100kHz로 설정
  sd_init(5, &able_sdcard);

  if (!EEPROM.begin(EEPROM_SIZE_CONFIG*2)){
    Serial.println("Failed to initialise eeprom");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
  for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
    ssid[index]     = EEPROM.read(eep_ssid[index]);
    password[index] = EEPROM.read(eep_pass[index]);
  }

  if(able_sdcard) dir_make(path_savedata);
  

  sensor_mapping(UPDATE_INTERVAL_SENSOR+1);
  sensor_value_init();
  wifi_connect();
  serial_command_help(&Serial);
}
/*********************************************************/
void loop() {
  unsigned long millisec = millis();
  if (Serial.available()) command_process(Serial.read());
  sensor_mapping(millisec);
  sensor_upload(millisec);
  csv_data_save(millisec);
}
/*********************************************************/
void sensor_upload(unsigned long millisec){
  if(millisec > pre_update_post + UPDATE_INTERVAL_HTTP){
    pre_update_post = millisec;
    String response = httpPOSTRequest(http_server_addr,sensor_json());
    //Serial.println(response);
  }
}

String httpPOSTRequest(String server_url, String send_data) {
  String response = "";
  if(able_wifi){
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
/*********************************************************/
void csv_data_save(unsigned long millisec){
  if(millisec > pre_save_csv + UPDATE_INTERVAL_CSV){
    pre_save_csv = millisec;
    csv_save(sensor_csv(false));
  }
}

String csv_file_name(uint16_t file_number){
  String response     = path_savedata + "/data";
  uint16_t num_digit  = file_number;
  for(uint8_t index=0; index<4; index++){
    num_digit /= 10;
    if(num_digit==0) response += "0";
  }
  response += String(file_number) + ".csv";
  return response;
}

void csv_save(String save_data){
  if(able_sdcard){
    uint16_t file_number = dir_list(path_savedata,false,false);
    String file_name = "";
    Serial.println(file_number);
    if(file_number == 0){
      file_name = csv_file_name(1);
      file_write(file_name,"");
    }else{
      file_name = csv_file_name(file_number);
      if(!exisits_check(file_name)) file_write(file_name,"");
    }
    if(file_size(file_name)>250*5){
      file_name = csv_file_name(file_number+1);
      file_write(file_name,"");
    }
    if(dir_list(path_savedata,false,false)==0){
      able_sdcard = false;
    }else{
      file_append(file_name,save_data);
      Serial.println(file_size(file_name));
    }
  }
}
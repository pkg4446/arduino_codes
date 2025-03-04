#include <Wire.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <HTTPClient.h>

String firmwareVersion = "0.0.1";
#define uS_TO_S_FACTOR      1000000  //Conversion factor for micro seconds to seconds
#define SECONDE             1000L
RTC_DATA_ATTR uint32_t bootCount = 0;

#define UPDATE_INTERVAL_SENSOR  300L
#define UPDATE_INTERVAL_HTTP    60000L
#define UPDATE_INTERVAL_CSV     60000L

#define TCA9548A_COUNT  8
#define SENSOR_COUNT    64
#define MOVING_AVERAGE  10

#define COMMAND_LENGTH  32
#define WIFI_WAIT       10

#define EEPROM_SIZE_CONFIG  24

#define SDA_PIN 21
#define SCL_PIN 22
////--------------------- Pin out ---------------------////
const uint8_t pin_config = 36;
////--------------------- Pin out ---------------------////

/*********************************************************/
const uint8_t TMP112_ADDRESS = 0x48; // TMP112 온도 센서 주소
byte tcaAddresses[TCA9548A_COUNT] = {0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77}; // 멀티플렉서 주소
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

void setTMP112Mode(bool wake) {
  Wire.beginTransmission(TMP112_ADDRESS);
  Wire.write(0x01);
  if (wake) {
    Wire.write(0x00);  // 깨우기 - 연속 변환 모드
  } else {
    Wire.write(0x01);  // 셧다운 모드
  }
  Wire.write(0x00);
  Wire.endTransmission();
  
  if (wake) {
    delay(30);  // 첫 변환을 위한 대기 시간
  }
}

int16_t readTemp() {
  // 온도 읽기
  Wire.beginTransmission(TMP112_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(TMP112_ADDRESS, 2);
  if (Wire.available() == 2) {
    int16_t raw = (Wire.read() << 8) | Wire.read();
    raw >>= 4; // TMP112는 12비트 해상도, 따라서 4비트 쉬프트
    return int16_t(raw * 6.25); // TMP112 온도 변환
  } else {
    return 9999; // 읽기 실패
  }
}

void sensor_mode(bool wake){
  for (uint8_t index = 0; index < TCA9548A_COUNT; index++) {
    for (uint8_t ch = 0; ch < 8; ch++) {
      tcaSelect(tcaAddresses[index], ch);
      setTMP112Mode(wake);
    }
    tcaDisable(tcaAddresses[index]);
  }
}

void sensor_mapping(){
  uint8_t  index_sensor = 0;
  // 모든 센서 값 읽기
  for (uint8_t index = 0; index < TCA9548A_COUNT; index++) {
    for (uint8_t ch = 0; ch < 8; ch++) {
      tcaSelect(tcaAddresses[index], ch);
      temperatures[index_sensor++][index_average] = readTemp();
    }
    tcaDisable(tcaAddresses[index]); // 각 멀티플렉서 비활성화
  }
  if(++index_average>=MOVING_AVERAGE) index_average = 0;
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

  const uint8_t col_row = 8;
  const uint8_t comma = col_row-1;
  for (uint8_t row = 0; row < col_row; row++){
    response += ",\"row"+String(row)+"\":[";
    for (uint8_t col = 0; col < col_row; col++){
      uint8_t index = col*col_row + row;
      int32_t sensor_temperature = 0;
      for (uint8_t average = 0; average < MOVING_AVERAGE; average++) {
        sensor_temperature += temperatures[index][average];
      }
      sensor_temperature /= MOVING_AVERAGE;
      if (sensor_temperature == 9999) response += "-404";
      else response += String(sensor_temperature);
      if(col<comma) response += ",";
    }
    response += "]";
  }
  response += "}";
  return response;
}

String sensor_csv(bool type_save){
  String response = "";
  const uint8_t col_row = 8;
  const uint8_t comma = col_row-1;
  for (uint8_t row = 0; row < col_row; row++){
    for (uint8_t col = 0; col < col_row; col++){
      uint8_t index = col*col_row + row;
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
        else if(row<comma)  response += ",";
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
    serial_command_help();
  }else{ serial_err_msg(command_buf); }

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
bool wifi_connect() {
  able_wifi = true;
  serial_wifi_config(ssid,password);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long wifi_config_update  = millis();
  while (WiFi.status() != WL_CONNECTED) {
    unsigned long update_time = millis();
    config_update_check();
    if(update_time - wifi_config_update > WIFI_WAIT*SECONDE){
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
////---------------------------------------------------////
void config_update_check(){
  if(digitalRead(pin_config)){
    serial_command_help();
    while (digitalRead(pin_config)){
      if(Serial.available()) command_process(Serial.read());
    }
  }
}
/*********************************************************/
void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000); // I2C 속도를 100kHz로 설정
  pinMode(pin_config, INPUT);
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
  wifi_connect();
  if(able_wifi){
    sensor_mode(true);
    for (uint8_t index = 0; index < TCA9548A_COUNT; index++){sensor_mapping();}
    if(able_wifi) sensor_upload();
  }
  sensor_mode(false);

  Serial.print("ver:");
  Serial.println(firmwareVersion);

  // 타이머 wake-up 설정
  esp_sleep_enable_timer_wakeup(60 * uS_TO_S_FACTOR);
  // 딥슬립 시작
  esp_deep_sleep_start();


}
/*********************************************************/
void loop() {}
/*********************************************************/
void sensor_upload(){
  String response = httpPOSTRequest(http_server_addr,sensor_json());
  Serial.println(response);
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
void serial_err_msg(char *msg){
  Serial.print("wrong cmd: ");
  Serial.println(msg);
}
void serial_command_help() {
  Serial.println("************* help *************");
  Serial.println("reboot   * system reboot");
  Serial.println("sensor   * sensor read");
  Serial.println("ssid     * ex)ssid your ssid");
  Serial.println("pass     * ex)pass your password");
  Serial.println("wifi     * WIFI connet");
  Serial.println("   scan  * WIFI scan");
  Serial.println("   stop  * WIFI disconnet");
  Serial.println("help     * this text");
  Serial.println("************* help *************");
}
void serial_wifi_config(char *ssid, char *pass){
  Serial.println("********* wifi config *********");
  Serial.print("your ssid: "); Serial.println(ssid);
  Serial.print("your pass: "); Serial.println(pass);
  Serial.println("********* wifi config *********");
}
#include <Update.h>
#include <EEPROM.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_MAX1704X.h>
#include <OneWireNg_CurrentPlatform.h>

String firmwareVersion = "0.0.1"; // 버전만 수정됨 표시

#define uS_TO_S_FACTOR      1000000  //Conversion factor for micro seconds to seconds
#define SECONDE             1000L
#define COMMAND_LENGTH  32
#define WIFI_WAIT       10

////--------------------- Pin out ---------------------////
#define PIN_SDA         6   
#define PIN_SCL         7   
#define PIN_AC_DETECT   3   
#define PIN_LED_STATUS  22  
#define PIN_SSR_HEATER  23  
#define PIN_DS_EXT      4   
#define PIN_DS_SPACE    5   
#define PIN_CONFIG      2 
////--------------------- Pin out ---------------------////
/*
#define SDA_PIN 21
#define SCL_PIN 22
*/
////--------------------- EEPROM ---------------------////
#define EEPROM_SIZE_CONFIG  24
////--------------------- EEPROM ---------------------////

////--------------------- Object ---------------------////
RTC_DATA_ATTR uint32_t bootCount = 0;
Adafruit_SHT31 sht31 = Adafruit_SHT31(); 
Adafruit_MAX17048 maxlipo;
OneWireNg *owExt = NULL;
OneWireNg *owSpace = NULL;
bool able_maxlipo = false;
////--------------------- Object ---------------------////

/*********************************************************/
const uint8_t ADDR_SSID = 0;   
const uint8_t ADDR_PASS = 24;  
const uint8_t ADDR_HEAT = 48; 
const uint8_t ADDR_GAP  = 49; 
const uint8_t ADDR_GOAL = 50; 

// String http_server_addr = "http://array.beetopia.kro.kr/device/log";
String http_server_addr = "http://test.beetopia.kro.kr/device/log";
char  deviceID[18] = {0};  // 초기화 추가
/*********************************************************/
bool able_sdcard = false;
bool able_wifi   = false;
/*********************************************************/
float   temp_heat   = 0.00f;
float   temp_air    = 0.00f;
float   temperature = 0.00f;
float   humidity    = 0.00f;
char    ssid[EEPROM_SIZE_CONFIG];
char    password[EEPROM_SIZE_CONFIG];
char    command_buf[COMMAND_LENGTH];
int8_t  command_num  = 0;
/*********************************************************/
unsigned long pre_sensor_read = 0UL;
unsigned long pre_update_post = 0UL;
unsigned long pre_save_csv    = 0UL;
uint8_t  index_sensor   = 0;
uint8_t  index_average  = 0;
/*********************************************************/

// 함수 프로토타입 선언 추가
void sensor_upload();
bool wifi_connect();
void WIFI_scan(bool wifi_state);
void serial_wifi_config(char *ssid, char *pass);
void config_update_check();
void serial_command_help();
void serial_err_msg(char *msg);

String sensor_json(){
  String response = "{\"dvid\":\""+String(deviceID)+"\"";
  if(able_maxlipo) response += ",\"lipo\":["+String(maxlipo.cellPercent())+","+String(maxlipo.cellVoltage());
  response += "],\"temp:"+isnan(temperature) ? String(temperature):"\"NAN\"";
  response += ",\"air\":"+isnan(temperature) ?"\"NAN\"": String(temp_air);
  response += ",\"heat\":"+isnan(temperature) ? "\"NAN\"":String(temp_heat);
  response += "}";
  return response;
}

float readDS18B20(OneWireNg *ow) {
    if (!ow) return NAN; 

    // 1. 온도 변환 시작
    if (ow->reset() != OneWireNg::EC_SUCCESS) return NAN; 
    ow->writeByte(0xCC); 
    ow->writeByte(0x44);

    // 2. 비차단 대기 (750ms)
    unsigned long start = millis();
    while(millis() - start < 750) { 
        if(Serial.available()) command_process(Serial.read()); 
        yield(); 
    }
    
    // 3. 데이터 읽기
    if (ow->reset() != OneWireNg::EC_SUCCESS) return NAN;
    ow->writeByte(0xCC); 
    ow->writeByte(0xBE);

    uint8_t data[9];
    for (int i = 0; i < 9; i++) data[i] = ow->readByte();

    // 4. CRC 검사
    if (OneWireNg::crc8(data, 8) != data[8]) {
        return NAN; 
    }

    // 5. 온도 계산 및 범위 체크
    int16_t raw = (data[1] << 8) | data[0];
    float temp = (float)raw / 16.0;

    if (temp < -55.0 || temp > 125.0) return NAN;

    return temp;
}

  void read_sensors(){
    temp_heat = readDS18B20(owExt);
    temp_air  = readDS18B20(owSpace);
    if (sht31.begin(0x44)) {
      temperature = sht31.readTemperature();
      humidity    = sht31.readHumidity();
    } else {
      temperature = NAN;
      humidity    = NAN;
      Serial.println("SHT31 missing");
    }
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
  bool    eep_change   = false;
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
  }else if(cmd_text=="ssid"){
    able_wifi = false;
    WiFi.disconnect(true);
    Serial.print("ssid: ");
    if(temp_text.length() > 0){
      for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
        if(index < temp_text.length()){
          Serial.print(temp_text[index]);
          ssid[index] = temp_text[index];
          EEPROM.write(index, byte(temp_text[index]));
        }else{
          ssid[index] = 0x00;
          EEPROM.write(index, byte(0x00));
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
          EEPROM.write(EEPROM_SIZE_CONFIG+index, byte(temp_text[index]));
        }else{
          password[index] = 0x00;
          EEPROM.write(EEPROM_SIZE_CONFIG+index, byte(0x00));
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
  able_wifi = false;  // 초기값을 false로 설정
  serial_wifi_config(ssid, password);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long wifi_config_update = millis();
  
  // [Fix] 시인성 개선
  Serial.print("Connecting to WiFi [");
  Serial.print(ssid);
  Serial.print("] ");

  // 타임아웃 처리 개선
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // 0.5초 대기 (모뎀 안정화 및 UX)
    Serial.print("."); // [Fix] 점 찍기

    unsigned long update_time = millis();
    config_update_check();
    if(update_time - wifi_config_update > WIFI_WAIT * SECONDE){
      Serial.println("\nWIFI connection failed");
      WiFi.disconnect(true);
      return false;  // 연결 실패 반환
    }
  }
  
  Serial.println("\nWIFI connected");
  
  // MAC 주소를 디바이스 ID로 변환 (범위 체크 추가)
  String mac = WiFi.macAddress();
  int j = 0;
  for (int i = 0; i < mac.length() && j < 17; i++) {
    if (mac[i] == ':') {
      deviceID[j++] = '_';
    } else {
      deviceID[j++] = mac[i];
    }
  }
  deviceID[17] = '\0';  // 문자열 종료 문자 추가
  
  Serial.print("deviceID: ");
  Serial.println(deviceID);
  
  able_wifi = true;  // 연결 성공 후 true로 설정
  return true;
}

////---------------------------------------------------////
void config_update_check(){
  // [Fix] Active Low 반전: 버튼 눌림(!digitalRead) 감지
  if(!digitalRead(PIN_CONFIG)){
    serial_command_help();
    while (!digitalRead(PIN_CONFIG)){
      if(Serial.available()) command_process(Serial.read());
      yield();
    }
  }
}

/*********************************************************/
void setup() {
  // [Fix] 시리얼 무조건 초기화 (부팅 로그 확인용)
  Serial.begin(115200);

  // Pins
  pinMode(PIN_LED_STATUS, OUTPUT);
  pinMode(PIN_SSR_HEATER, OUTPUT);
  pinMode(PIN_AC_DETECT, INPUT_PULLUP);
  pinMode(PIN_CONFIG, INPUT_PULLUP);
  digitalWrite(PIN_SSR_HEATER, LOW);

  if( bootCount++%30==0 || digitalRead(PIN_AC_DETECT)){
    Wire.begin(PIN_SDA, PIN_SCL);
    
    able_maxlipo = maxlipo.begin(); // MAX17048 센서 초기화
    if (!able_maxlipo) {
      Serial.println("MAX17048 missing");
    }

    // 1-Wire
    owExt   = new OneWireNg_CurrentPlatform(PIN_DS_EXT, false);
    owSpace = new OneWireNg_CurrentPlatform(PIN_DS_SPACE, false);
    
    // 모든 배열 초기화 - 메모리 오류 방지
    temperature = 0.00f;
    humidity    = 0.00f;
    memset(ssid, 0, sizeof(ssid));
    memset(password, 0, sizeof(password));
    memset(command_buf, 0, sizeof(command_buf));
    
    // EEPROM 초기화
    if (!EEPROM.begin(EEPROM_SIZE_CONFIG * 2)){
      Serial.println("Failed to initialise eeprom");
      Serial.println("Restarting...");
      delay(1000);
      ESP.restart();
    }
    
    // EEPROM에서 데이터 읽기
    for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
      ssid[index] = EEPROM.read(index);
      password[index] = EEPROM.read(EEPROM_SIZE_CONFIG+index);
    }
    Serial.println(" times online");
    bool wifi_connected = wifi_connect();

    if(able_maxlipo) maxlipo.reset();
    
    //온습도 센서 읽기
    read_sensors();
    //센서 업로드
    if(able_wifi){
      sensor_upload();
    }
    Serial.print("Firmware version: ");
    Serial.println(firmwareVersion);
    //외부전원 확인 => 상시 전환
    loop_ac();
  }

  // 타이머 wake-up 설정 10초
  esp_sleep_enable_timer_wakeup(10 * uS_TO_S_FACTOR); 
  // 딥슬립 시작
  Serial.println("Deep Sleep Start");
  Serial.flush(); // 로그 전송 완료 대기
  esp_deep_sleep_start();
}
/*********************************************************/
void loop_ac() {
  // [Fix] Active Low 적용: 핀이 LOW(0)인 동안 동작
  // [Fix] 비어있던 좀비 루프에 동작 로직(센싱+업로드) 주입
  
  if(digitalRead(PIN_AC_DETECT)) Serial.println("[AC MODE] Started");

  while (digitalRead(PIN_AC_DETECT)){
    
    // 1. 동작 수행
    Serial.println("[AC MODE] Run...");
    read_sensors();
    
    // WiFi 재연결 로직 추가 (끊겼을 경우)
    if(WiFi.status() != WL_CONNECTED) wifi_connect();
    
    sensor_upload();

    // 2. 대기 및 시리얼 명령 처리 (10초 대기)
    for(int i=0; i<100; i++){ // 100ms * 100 = 10초
        if(Serial.available()) command_process(Serial.read());
        if(!digitalRead(PIN_AC_DETECT)) break; // AC 빠지면(HIGH) 즉시 탈출
        delay(100);
    }
    yield();
  }
}


/*********************************************************/
void loop() {
  // ESP 딥슬립으로 인해 실행되지 않음
  delay(1000);
}

/*********************************************************/
void sensor_upload(){
  if (!able_wifi) return;  // WiFi 연결 확인
  
  String jsonData = sensor_json();
  Serial.println("Sending data to server: " + jsonData);
  
  String response = httpPOSTRequest(http_server_addr, jsonData);
  Serial.println("Server response: " + response);
}

String httpPOSTRequest(String server_url, String send_data) {
  String response = "";
  if(able_wifi){
    WiFiClient client;
    HTTPClient http;
    
    http.begin(client, server_url);
    http.addHeader("Content-Type", "application/json");
    int response_code = http.POST(send_data);
    
    if (response_code > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(response_code);
      response = http.getString();
    } else {
      Serial.print("HTTP Error: ");
      Serial.println(response_code);
      response = "HTTP Error: " + String(response_code);
    }
    
    http.end();
  } else {
    response = "WiFi not connected";
  }
  return response;
}

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
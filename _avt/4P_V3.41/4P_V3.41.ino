#include <Update.h>
#include <EEPROM.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_MAX1704X.h>
#include <OneWireNg_CurrentPlatform.h>

String firmwareVersion = "0.0.1"; // 버전만 수정됨 표시

#define uS_TO_S_FACTOR  1000000  //Conversion factor for micro seconds to seconds
#define SLEEP_WAKE      10
#define UPLOAD_PERIOD   180
#define SECONDE         1000L
#define WIFI_WAIT       10
#define COMMAND_LENGTH  32
////--------------------- PID -------------------------////
#define MAX_BEE_TEMP    35
#define MAX_HEAT        50
#define PEAK_COUNT      10    // 10번의 피크 = 5번의 전체 주기
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
#define EEPROM_HEAT_USE     48
#define EEPROM_HEAT_GOAL    49
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

String server = "http://yc.beetopia.kro.kr/";
// String server = "http://192.168.4.2:3002/";
char  deviceID[18] = {0};  // 초기화 추가
/*********************************************************/
bool wifi_able  = false;
bool heat_use   = false;
////--------------------- PID -------------------------////
uint8_t temp_goal = 0; // 목표값
float   temp_heater;
float   Kp = 0, Ki = 0, Kd = 0;
// 자동 튜닝용 변수
bool  isTuning = false;
float Ku = 0, Pu = 0;
float lastInput;
float ITerm = 0;
int   oscillationCount = 0;
unsigned long lastTime, startTime;
// 추가 또는 수정이 필요한 전역 변수들
float peakMax = -1e6;
float peakMin = 1e6;
float amplitudeSum = 0;
float periodSum = 0;
unsigned long lastPeakTime = 0;
int   peakCount = 0;
bool  isHigh = false; // 현재 온도가 temp_heater보다 높은지 여부
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
unsigned long update_pid = 0UL;

uint16_t working_total  = 0;
uint16_t heater_working = 0;
/*********************************************************/

// 함수 프로토타입 선언 추가
void config_upload();
void sensor_upload();
bool wifi_connect();
void WIFI_scan(bool wifi_state);
void serial_wifi_config(char *ssid, char *pass);
void config_update_check();
void serial_command_help();
void serial_err_msg(char *msg);

String sensor_json(){
  String response = (String)"{\"DVC\":\""+String(deviceID)+"\",\"HM\":[\"NAN\",\"NAN\",";
  response += isnan(humidity) ? "\"NAN\"" : String(humidity);
  response += "],\"TP\":[";  
  response += isnan(temp_heat) ? "\"NAN\"" :String(temp_heat);
  response += ",";
  response += isnan(temp_air) ? "\"NAN\"" : String(temp_air);
  response += ",";
  response += isnan(temperature) ? "\"NAN\"" : String(temperature);
  response += "],\"bat\":[";
  response += able_maxlipo ? String(maxlipo.cellPercent())+","+String(maxlipo.cellVoltage()) : "\"NAN\",\"NAN\"";
  response += "],\"WK\":"+String(heater_working) + ",\"GAP"+"\":"+String(working_total)+"}";
  heater_working = 0;
  working_total  = 0;
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
////--------------------- PID -------------------------////
float constraint(float val) {
  if (val < 0) return 0;
  if (val > MAX_HEAT) return MAX_HEAT;
  return val;
}

void run_pid() {
  unsigned long now = millis();
  float timeChange = (float)(now - lastTime) / SECONDE; // 초 단위

  if (timeChange <= 0) return;

  float error = temp_goal - temp_air;
  ITerm += (Ki * error * timeChange);
  // Anti-windup (출력 제한)
  ITerm = constraint(ITerm);

  float dInput = (temp_air - lastInput) / timeChange;

  temp_heater = Kp * error + ITerm - Kd * dInput;
  temp_heater = constraint(temp_heater);

  lastInput = temp_air;
  lastTime = now;
}

void pid_tune() {
  // 1. 릴레이 제어 (On/Off 제어)
  if (temp_air < temp_goal) {
    temp_heater = MAX_HEAT; // Max Power
    if (isHigh) { // 위에서 아래로 내려오는 순간 (하강 교차)
      isHigh = false;
    }
  } else {
    temp_heater = 0; // Min Power
    if (!isHigh) { // 아래에서 위로 올라가는 순간 (상승 교차)
      isHigh = true;
    }
  }

  // 2. 피크값(최대/최소) 갱신
  if (temp_air > peakMax) peakMax = temp_air;
  if (temp_air < peakMin) peakMin = temp_air;

  // 3. 제로 크로싱(temp_goal 통과) 시점에 피크 기록
  // 여기서는 단순히 방향이 바뀔 때 peak를 계산하는 방식으로 설명합니다.
  static bool lastDirection = false; // true: 상승중, false: 하강중
  bool currentDirection = (temp_air > lastInput);
  
  // 방향이 바뀌었을 때 (피크 도달)
  if (lastDirection != currentDirection) {
    unsigned long now = millis();
    
    // 유효한 진동인지 확인 (노이즈 방지를 위해 작은 임계값 추가 가능)
    if (peakCount > 0) {
      // 주기 누적 (피크와 피크 사이의 시간)
      periodSum += (now - lastPeakTime);
    }
    
    lastPeakTime = now;
    peakCount++;
        
    if (peakCount >= PEAK_COUNT) { 
      float averageAmplitude = (peakMax - peakMin) / 2.0;
      float averagePeriod = (periodSum / (peakCount - 1)) / 1000.0; // 초 단위

      // Ku = (4 * RelayOutput) / (pi * Amplitude)
      // 여기서 RelayOutput은 MAX_HEAT(전체 가동 범위)
      Ku = (4.0 * MAX_HEAT) / (3.14159 * averageAmplitude);
      Pu = averagePeriod;

      // Ziegler-Nichols 공식 적용
      Kp = 0.6 * Ku;
      Ki = 2.0 * Kp / Pu;
      Kd = Kp * Pu / 8.0;

      isTuning = true;
      Serial.println("--- 튜닝 완료 ---");
      Serial.print("Ku: "); Serial.println(Ku);
      Serial.print("Pu: "); Serial.println(Pu);
      Serial.print("Kp: "); Serial.println(Kp);
      Serial.print("Ki: "); Serial.println(Ki);
      Serial.print("Kd: "); Serial.println(Kd);
      
      lastTime = millis(); // PID 시작 시간 초기화
    }
  }
  lastInput = temp_air;
}

void loop_pid(){
  unsigned long now = millis();
  if(now - update_pid > SECONDE){
    update_pid = now;
    if (sht31.begin(0x44)) {
      temperature = sht31.readTemperature();
    } else {
      temperature = NAN;
      Serial.println("SHT31 missing");
    }
    if(!isnan(temperature) && temperature<MAX_BEE_TEMP){ //봉구온도가 35도보다 낮을경우만 작동
      temp_heat = readDS18B20(owExt);
      temp_air  = readDS18B20(owSpace);
      if (!isTuning) {
        pid_tune();
      } else {
        run_pid();
      }
      working_total++;
      if(temp_heat<temp_heater){
        heater_working++;
        digitalWrite(PIN_SSR_HEATER, true);
      }else{
        digitalWrite(PIN_SSR_HEATER, false);
      }
    }
  }
}
/*********************************************************/
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
  if(wifi_able){
    wifi_connect();
  }else{
    WiFi.disconnect(true);
  }
}

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
void command_service(){
  bool    eep_change  = false;
  uint8_t check_index = 0;
  String cmd_text  = String_slice(&check_index, command_buf, 0x20);
  String temp_text = String_slice(&check_index, command_buf, 0x20);
  ////cmd start
  if(cmd_text=="reboot"){
    ESP.restart();
  }else if(cmd_text=="sensor"){
    read_sensors();
    Serial.print("Heater: ");
    Serial.println(temp_heat);
    Serial.print("Air: ");
    Serial.println(temp_air);
    Serial.print("HIVE_T: ");
    Serial.println(temperature);
    Serial.print("HIVE_H: ");
    Serial.println(humidity);
  }else if(cmd_text=="run"){
    bool change_flage = false;
    if(temp_text=="on" && !heat_use){
      EEPROM.write(EEPROM_HEAT_USE, true);
      heat_use = true;
      change_flage = true;
    }else if(heat_use){
      EEPROM.write(EEPROM_HEAT_USE, false);
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
      EEPROM.write(EEPROM_HEAT_GOAL, set_value);
      temp_goal = set_value;
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
    Serial.print("homeothermy mode : ");
    if(heat_use) Serial.println("ON");
    else Serial.println("OFF");
    Serial.print("temperature setup ");
    Serial.print(temp_goal);
    Serial.println(" ℃");

  }else if(cmd_text=="ssid"){
    wifi_able = false;
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
          EEPROM.write(EEPROM_SIZE_CONFIG+index, byte(temp_text[index]));
        }else{
          password[index] = 0x00;
          EEPROM.write(EEPROM_SIZE_CONFIG+index, byte(0x00));
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
    serial_command_help();
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
  wifi_able = false;  // 초기값을 false로 설정
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
  
  wifi_able = true;  // 연결 성공 후 true로 설정
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
  digitalWrite(PIN_SSR_HEATER, false);

  if( bootCount++%UPLOAD_PERIOD==0 || !digitalRead(PIN_AC_DETECT)){
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
    if (!EEPROM.begin(EEPROM_HEAT_GOAL+1)){
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
    heat_use  = EEPROM.read(EEPROM_HEAT_USE);
    temp_goal = EEPROM.read(EEPROM_HEAT_GOAL);

    bool wifi_connected = wifi_connect();

    if(able_maxlipo) maxlipo.reset();
    
    //온습도 센서 읽기
    read_sensors();
    //센서 업로드
    sensor_upload();

    Serial.print("Firmware version: ");
    Serial.println(firmwareVersion);
    //외부전원 확인 => 상시 전환
    loop_ac();
  }

  // 타이머 wake-up 설정
  esp_sleep_enable_timer_wakeup(SLEEP_WAKE * uS_TO_S_FACTOR); 
  // 딥슬립 시작
  Serial.println("Deep Sleep Start");
  Serial.flush(); // 로그 전송 완료 대기
  // esp_deep_sleep_start();
}
/*********************************************************/
void loop_ac() {
  // [Fix] Active Low 적용: 핀이 false(0)인 동안 동작
  // [Fix] 비어있던 좀비 루프에 동작 로직(센싱+업로드) 주입
  unsigned long pre_update_post = millis();
  if(!digitalRead(PIN_AC_DETECT)) Serial.println("[AC MODE] Started");
  while (!digitalRead(PIN_AC_DETECT)){
    if(Serial.available()) command_process(Serial.read());
    if(millis()-pre_update_post > SECONDE*WIFI_WAIT*UPLOAD_PERIOD){
      pre_update_post = millis();
      if(WiFi.status() != WL_CONNECTED) wifi_connect();
      read_sensors();
      sensor_upload();
    }
    if(heat_use) loop_pid();
    yield();
  }
}


/*********************************************************/
void loop() {
  // ESP 딥슬립으로 인해 실행되지 않음
  delay(1000);
}

/*********************************************************/
void config_upload(){
  String set_data = "{\"DVC\":\"" + String(deviceID) + "\",\"TMP\":" + String(temp_goal) + ",\"RUN\":" + String(heat_use) + "}";
  String response = httpPOSTRequest(server+"device/set_25",set_data);
  Serial.print("http:");
  Serial.println(response);
}
void sensor_upload(){
  if (!wifi_able) return;  // WiFi 연결 확인 
  String response = httpPOSTRequest(server+"device/log_25", sensor_json());
  Serial.print("http:");
  Serial.println(response);
  //여기서 설정 변경
  uint8_t check_index = 0;
  String cmd_text = String_slice(&check_index,response, 0x2C);
  if (cmd_text == "set"){
    uint8_t set_value = 0;
    bool change_flage = false;
    
    set_value = String_slice(&check_index,response, 0x2C).toInt();
    if(temp_goal != set_value){
      Serial.println(" O");
      EEPROM.write(EEPROM_HEAT_GOAL, set_value);
      temp_goal = set_value;
      change_flage = true;
    }

    set_value = String_slice(&check_index,response, 0x2C).toInt();
    if((set_value == 1 && !heat_use)||(set_value == 0 && heat_use)){
      EEPROM.write(EEPROM_HEAT_USE, set_value);
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

String httpPOSTRequest(String server_url, String send_data) {
  String response = "";
  if(wifi_able){
    WiFiClient client;
    HTTPClient http;
    
    http.begin(client, server_url);
    http.addHeader("Content-Type", "application/json");
    int response_code = http.POST(send_data);
    
    Serial.println(server_url);
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
  Serial.println("run      * heater run");
  Serial.println("set      * temperature goal set");
  Serial.println("config   * setting view");
  Serial.println("ssid     * ex)ssid your ssid");
  Serial.println("pass     * ex)pass your password");
  Serial.println("wifi     * WIFI connet");
  Serial.println("   scan  * WIFI scan");
  Serial.println("   stop  * WIFI disconnet");
  Serial.println("************* help *************");
}

void serial_wifi_config(char *ssid, char *pass){
  Serial.println("********* wifi config *********");
  Serial.print("your ssid: "); Serial.println(ssid);
  Serial.print("your pass: "); Serial.println(pass);
  Serial.println("********* wifi config *********");
}

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
    String httpRequestData = (String)"{\"DVC\":\""+String(deviceID)+"\",\"KIND\":\"device_25\",\"ver\":\"" + firmwareVersion + "\"}";
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
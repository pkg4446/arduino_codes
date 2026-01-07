#include <EEPROM.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_SHT31.h>

String firmwareVersion = "0.0.1";

#define uS_TO_S_FACTOR            1000000  //Conversion factor for micro seconds to seconds
#define SECONDE                   1000L
#define TCAADDR                   0x70
#define TOTAL_TEMPERATURE_SENSOR  4
#define EEPROM_SIZE_CONFIG        24
#define COMMAND_LENGTH            32
#define UPLOAD_PERIOD             30
#define PIN_BUTTON1               2
#define PIN_BUTTON2               3


const   String server = "http://array.beetopia.kro.kr/";
char    deviceID[18];
char    command_buf[COMMAND_LENGTH];
int8_t  command_num;
////--------------------- EEPROM ----------------------////
const uint8_t eep_ssid = 0;
const uint8_t eep_pass = 24;
char  ssid[EEPROM_SIZE_CONFIG];
char  password[EEPROM_SIZE_CONFIG];
////--------------------- EEPROM ----------------------////
////--------------------- Flage -----------------------////
bool    wifi_able     = false;
////--------------------- Flage -----------------------////
////--------------------- temperature sensor ----------////
Adafruit_SHT31 sht31 = Adafruit_SHT31();
float sensor_temperature[TOTAL_TEMPERATURE_SENSOR] = {0.00f,};
float sensor_humidity[TOTAL_TEMPERATURE_SENSOR]    = {0.00f,};
////--------------------- temperature sensor ----------////
////--------------------- Serial command --------------////
void Serial_command(){ if(Serial.available()) command_process(Serial.read()); }
////--------------------- Serial command --------------////
////--------------------- setup() ---------------------////
void setup()
{
  Serial.begin(115200); 
  Wire.begin();
  pinMode(PIN_BUTTON1, INPUT_PULLUP);

  Serial.print("ver:");
  Serial.println(firmwareVersion);
    
  // 모든 배열 초기화 - 메모리 오류 방지
  memset(ssid, 0, sizeof(ssid));
  memset(password, 0, sizeof(password));
  memset(command_buf, 0, sizeof(command_buf));
  if (!EEPROM.begin((EEPROM_SIZE_CONFIG*2) + 1)){
    Serial.println("Failed to initialise eeprom");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
  for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
    ssid[index]     = EEPROM.read(eep_ssid+index);
    password[index] = EEPROM.read(eep_pass+index);
  }

  while (digitalRead(PIN_BUTTON1))
  {
    Serial_command();
  }

  wifi_connect();
  for (uint8_t index = 0; index < 17; index++) {
    if(WiFi.macAddress()[index]==':'){
      deviceID[index] = '_';
    }else{
      deviceID[index] = WiFi.macAddress()[index];
    }
  }
  sensor_upload();

  esp_sleep_enable_timer_wakeup(UPLOAD_PERIOD * 60 * uS_TO_S_FACTOR);
  Serial.println(" Going to deep sleep now");
  esp_deep_sleep_start();
}
////--------------------- setup() ---------------------////
////--------------------- loop() ----------------------////
void loop(){delay(10);}
////--------------------- loop() ----------------------////
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
////--------------------- command ---------------------////
void serial_command_help(){
  Serial.println("************* help *************");
  Serial.println("reboot   * system reboot");
  Serial.println("sensor   * sensor read");
  Serial.println("ssid     * ex)ssid your ssid");
  Serial.println("pass     * ex)pass your password");
  Serial.println("wifi     * WIFI connet");
  Serial.println("   scan  * WIFI scan");
  Serial.println("   stop  * WIFI disconnet");
  Serial.println("************* help *************");
}
////---------------------------------------------------////
void command_service(){
  bool    eep_change  = false;
  uint8_t check_index = 0;
  String cmd_text  = String_slice(&check_index, command_buf, 0x20);
  String temp_text = String_slice(&check_index, command_buf, 0x20);
  ////cmd start
  if(cmd_text=="reboot"){
    ESP.restart();
  }else if(cmd_text=="sensor"){
    temperature_sensor_read();
    for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++){
      Serial.print(index);
      Serial.print(" IC:");
      Serial.print(sensor_temperature[index]);
      Serial.print("℃,");
      Serial.print(sensor_humidity[index]);
      Serial.println("%");
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
  }else{
    serial_command_help();
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
void serial_wifi_config(char *ssid, char *pass){
  Serial.println("********* wifi config *********");
  Serial.print("your ssid: "); Serial.println(ssid);
  Serial.print("your pass: "); Serial.println(pass);
  Serial.println("********* wifi config *********");
}
////---------------------------------------------------////
void wifi_connect() {
  wifi_able = true;
  serial_wifi_config(ssid,password);
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
  //
  String response = (String)"{\"dvid\":\""+String(deviceID)+",\"kind\":\"mini_v3\"," + res_array[0] + "," + res_array[1] + "}";
  return response;
}

void sensor_upload(){
  temperature_sensor_read();
  String response = httpPOSTRequest(server+"device/log",sensor_json());
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
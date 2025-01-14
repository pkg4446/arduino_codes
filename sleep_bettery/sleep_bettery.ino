#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>

String firmwareVersion = "0.0.1";

#define uS_TO_S_FACTOR      1000000  //Conversion factor for micro seconds to seconds
#define SECONDE             1000L
#define EEPROM_SIZE_CONFIG  24
#define COMMAND_LENGTH      32

RTC_DATA_ATTR uint32_t bootCount = 0;
Adafruit_SHT31 sht31 = Adafruit_SHT31();

const uint8_t interval = 5;
// const   String server  = "http://yc.beetopia.kro.kr/";
const   String server  = "http://192.168.1.36:3003/";
char    deviceID[18];
char    command_buf[COMMAND_LENGTH];
int8_t  command_num;
////--------------------- EEPROM ----------------------////
const uint8_t eep_ssid = 0;
const uint8_t eep_pass = 24;
char  ssid[EEPROM_SIZE_CONFIG];
char  password[EEPROM_SIZE_CONFIG];
////--------------------- EEPROM ----------------------////
////--------------------- Pin out ---------------------////
const uint8_t pin_config = 21;
////--------------------- Pin out ---------------------////

void setup(){
  Serial.begin(115200);
  // I2C config: SDA=22, SCL=23
  Wire.begin(22,23);
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pin_config, INPUT);
  //Increment boot number and print it every reboot
  ++bootCount;
  delay(700);
  Serial.print("ver:");
  Serial.println(firmwareVersion);

  if (!EEPROM.begin((EEPROM_SIZE_CONFIG*2))){
    Serial.println("Failed to initialise eeprom");
    Serial.println("Restarting...");
    ESP.restart();
  }

  for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
    ssid[index]     = EEPROM.read(eep_ssid+index);
    password[index] = EEPROM.read(eep_pass+index);
  }

  bool wifi_able = wifi_connect();
  
  if(wifi_able){
    for (uint8_t index = 0; index < 17; index++) {
      if(WiFi.macAddress()[index]==':'){
        deviceID[index] = '_';
      }else{
        deviceID[index] = WiFi.macAddress()[index];
      }
    }
    httpPOSTRequest(server);
    WiFi.disconnect(true);
  }
  // 타이머 wake-up 설정
  esp_sleep_enable_timer_wakeup(interval * uS_TO_S_FACTOR);
  // 딥슬립 시작
  esp_deep_sleep_start();
}

void loop(){}

////Send Data//////////////////////////////////////
String send_data(){
  String httpRequestData = (String)"{\"DVC\":\""+String(deviceID)+"\",\"data\":{";
  httpRequestData += "\"count\":"+String(bootCount)+"";
  httpRequestData += ",\"sht31\":[\"";
  if(sht31.begin(0x44)) httpRequestData += String(sht31.readTemperature())+"\",\""+String(sht31.readHumidity());
  else httpRequestData += "NaN\",\"NaN";
  httpRequestData += "\"]}}";
  return httpRequestData;
}
////---------------------------------------------------////
void httpPOSTRequest(String serverUrl) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverUrl);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(send_data());
  Serial.println(serverUrl);
  Serial.println(httpResponseCode);
  http.end();           // Free resources
}////httpPOSTRequest_End

////---------------------------------------------------////
void WIFI_scan(bool wifi_state){
  bool wifi_able = wifi_state;
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
bool wifi_connect() {
  bool wifi_able = true;
  serial_wifi_config(ssid,password);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long wifi_config_update  = millis();
  while (WiFi.status() != WL_CONNECTED) {
    unsigned long update_time = millis();
    config_update_check();
    if(update_time - wifi_config_update > SECONDE*10){
      wifi_able = false;
      break;
    }
  }
  if(wifi_able) Serial.println("wifi on");
  else Serial.println("not connected");
  return wifi_able;
}
////---------------------------------------------------////
void config_update_check(){
  if(digitalRead(pin_config)){
    serial_command_help();
    unsigned long led_shift = millis();
    bool led_state = true;
    while (digitalRead(pin_config)){
      if(millis() - led_shift > SECONDE/3){
        led_shift = millis();
        led_state = !led_state;
        digitalWrite(LED_BUILTIN, led_state);
      }
      Serial_command();
    }
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
////---------------------------------------------------////
void command_service(){
  bool    eep_change  = false;
  uint8_t check_index = 0;
  String cmd_text  = String_slice(&check_index, command_buf, 0x20);
  String temp_text = String_slice(&check_index, command_buf, 0x20);
  ////cmd start
  if(cmd_text=="reboot"){
    ESP.restart();
  }else if(cmd_text=="ssid"){
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
////---------------------------------------------------////
void Serial_command(){ if(Serial.available()) command_process(Serial.read()); }
////---------------------------------------------------////
void serial_command_help() {
  Serial.println("************* help *************");
  Serial.println("reboot  * system reboot");
  Serial.println("ssid    * ex)ssid your ssid");
  Serial.println("pass    * ex)pass your password");
  Serial.println("wifi    * WIFI connet");
  Serial.println("   scan * WIFI scan");
  Serial.println("   stop * WIFI disconnet");
  Serial.println("********************************");
}
void serial_wifi_config(char *ssid, char *pass){
  Serial.println("********* wifi config *********");
  Serial.print("your ssid: "); Serial.println(ssid);
  Serial.print("your pass: "); Serial.println(pass);
  Serial.println("********* wifi config *********");
}
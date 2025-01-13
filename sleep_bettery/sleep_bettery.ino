#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include "uart_print.h"
// UART 관련 헤더 추가
#include "driver/uart.h"
#include "driver/gpio.h"
// UART 핀 정의 (XIAO ESP32 C6 기준)
#define UART_PIN_TXD GPIO_NUM_16
#define UART_PIN_RXD GPIO_NUM_17

String firmwareVersion = "0.0.1";

#define uS_TO_S_FACTOR      1000000  //Conversion factor for micro seconds to seconds
#define SECONDE             1000L
#define EEPROM_SIZE_CONFIG  24
#define COMMAND_LENGTH      32

RTC_DATA_ATTR uint32_t bootCount = 0;
Adafruit_SHT31 sht31 = Adafruit_SHT31();

const uint8_t interval = 5;
// const   String server  = "http://yc.beetopia.kro.kr/";
const   String server  = "http://192.168.1.36/";
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

// UART 출력 함수
void uart_print(const char* str) {
    uart_write_bytes(UART_NUM_0, str, strlen(str));
}

void uart_println(const char* str) {
    uart_write_bytes(UART_NUM_0, str, strlen(str));
    uart_write_bytes(UART_NUM_0, "\r\n", 2);
}

void setup(){
  // UART 초기화 및 설정
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };
  
  // UART 파라미터 설정
  ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_PIN_TXD, UART_PIN_RXD, 
                              UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  
  // UART 드라이버 설치
  ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0));
  

  // Serial.begin(115200);
  //I2C config: SDA=22, SCL=23
  Wire.begin(22,23);
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pin_config, INPUT);
  //Increment boot number and print it every reboot
  ++bootCount;

  uart_println("UART initialized");
  // Serial.print("ver:");
  // Serial.println(firmwareVersion);

  bool wifi_able = wifi_connect();
  config_update_check();
  
  if(wifi_able){
    for (uint8_t index = 0; index < 17; index++) {
      if(WiFi.macAddress()[index]==':'){
        deviceID[index] = '_';
      }else{
        deviceID[index] = WiFi.macAddress()[index];
      }
    }
    httpPOSTRequest(server);
  }
  // UART wake-up 소스 설정
  esp_sleep_enable_uart_wakeup(UART_NUM_0);
  // RTC 주변장치 전원 도메인 켜기
  ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON));
  // UART 핀 설정 유지
  gpio_hold_en(UART_PIN_RXD);
  gpio_hold_en(UART_PIN_TXD);
  // 풀업 저항 설정
  gpio_set_pull_mode(UART_PIN_RXD, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(UART_PIN_TXD, GPIO_PULLUP_ONLY);
  // 타이머 wake-up 설정
  esp_sleep_enable_timer_wakeup(interval * uS_TO_S_FACTOR);
  // 딥슬립 시작
  esp_deep_sleep_start();
}

void loop(){}

////Send Data//////////////////////////////////////
String send_data(){
  String httpRequestData = (String)"{\"DVC\":\""+String(deviceID)+"\",\"data\":{";
  httpRequestData += (String)"\"count\":"+String(bootCount)+"";
  if(sht31.begin(0x44)) httpRequestData += (String)",\"sht31\":[\""+sht31.readTemperature()+"\",\""+sht31.readHumidity()+"\"]";
  httpRequestData += (String)"}}";
  return httpRequestData;
}
////---------------------------------------------------////
void httpPOSTRequest(String serverUrl) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverUrl);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(send_data());
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
      break;
    }
  }
  return wifi_able;
}
////---------------------------------------------------////
void config_update_check(){
  if(digitalRead(pin_config)){
    serial_command_help(&Serial);
    unsigned long led_shift = millis();
    bool led_state = true;
    while (digitalRead(pin_config)){
      if(millis() - led_shift > SECONDE/3){
        led_shift = millis();
        led_state != led_state;
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
////---------------------------------------------------////
void Serial_command(){ if(Serial.available()) command_process(Serial.read()); }
////---------------------------------------------------////

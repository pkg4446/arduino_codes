#include <EEPROM.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "uart_print.h"

#define TOTAL_RELAY 6
//#define TOTAL_RELAY 10
#define EEPROM_SIZE_CONFIG  24
#define EEPROM_SIZE_VALUE   2
#define COMMAND_LENGTH  32
#define UPDATE_INTERVAL 60000L

HardwareSerial nxSerial(2);
bool    nextion_shift = false;
uint8_t nextion_page  = 0;
bool    uart_type     = false;
/***************EEPROM*********************/
const uint8_t eep_ssid[EEPROM_SIZE_CONFIG] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
const uint8_t eep_pass[EEPROM_SIZE_CONFIG] = {24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47};
const uint8_t eep_var[EEPROM_SIZE_VALUE]   = {48,49};
/***************EEPROM*********************/
/***************PIN_CONFIG*****************/
const int8_t Relay[TOTAL_RELAY] = {12,13,14,15,17,23};
//const int8_t Relay[TOTAL_RELAY] = {2,4,5,12,13,23,27,26,25,33};
/***************PIN_CONFIG*****************/
/***************Interval_timer*************/
unsigned long prevUpdateTime = 0L;
/***************Interval_timer*************/
/***************Variable*******************/
char      ssid[EEPROM_SIZE_CONFIG];
char      password[EEPROM_SIZE_CONFIG];
/***************Variable*******************/
bool      wifi_able = false;
bool      operation = false;
uint32_t  countdown = 0;
uint32_t  runtime   = 0;
/***************Variable*******************/
char      command_buf[COMMAND_LENGTH];
int8_t    command_num;
/******************************************/
void wifi_config() {
  if(uart_type){
    serial_wifi_config(&Serial,ssid,password);
  }else{}
}
/******************************************/
void WIFI_scan(bool wifi_state){
  wifi_able = wifi_state;
  WiFi.disconnect(true);
  nextion_print(&nxSerial,"page 1");//nextion page 이동
  nextion_page == 1;
  nextion_print(&nxSerial,"page_wifi.t_wifi.txt=\"WIFI scanning...\"");
  if(uart_type) Serial.println("WIFI Scanning…");
  uint8_t networks = WiFi.scanNetworks();
  if (networks == 0) {
    nextion_print(&nxSerial,"page_wifi.t_wifi.txt=\"WIFI not found!\"");
    if(uart_type) Serial.println("WIFI not found!");
  }else {
    nextion_print(&nxSerial,"page_wifi.t_wifi.txt=\"<= WIFI list are here!\\rscroll & select\"");
    if(uart_type){
      Serial.print(networks);
      Serial.println(" networks found");
      Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    }
    String wifi_list ="";
    for (int index = 0; index < networks; ++index) {
      // Print SSID and RSSI for each network found
      if(uart_type){
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
        if(uart_type) Serial.println(wifi_encryptionType);
      }
      if(nextion_page == 1){
        wifi_list += WiFi.SSID(index);
        if(index<networks-1)wifi_list += "\\r";
        if(index<10) nextion_print(&nxSerial,"page_wifi.wifi"+String(index)+".txt=\""+WiFi.SSID(index)+"\"");
      }
    }
    if(uart_type) Serial.println("");
    if(nextion_page == 1) nextion_print(&nxSerial,"page_wifi.list.path=\""+wifi_list+"\"");
  }
  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();
  if(wifi_able){
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
  if(uart_type){
    Serial.print("cmd: ");
    Serial.print(cmd_text);
    Serial.print(", ");
    Serial.println(temp_text);
  }

  if(cmd_text=="manual"){
    uint8_t relay_num = temp_text.toInt();
    if(relay_num < TOTAL_RELAY){
      String cmd_select = "";
      for(uint8_t index_check=check_index; index_check<COMMAND_LENGTH; index_check++){
        if(command_buf[index_check] == 0x20 || command_buf[index_check] == 0x00){
          check_index = index_check+1;
          break;
        }
        cmd_select += command_buf[index_check];
      }
      if(cmd_select == "on")  digitalWrite(Relay[relay_num], true);
      else  digitalWrite(Relay[relay_num], false);
    }
  }else if(cmd_text=="reboot"){
    ESP.restart();
  }else if(cmd_text=="page"){
    nextion_shift = true;
    nextion_page  = temp_text.toInt();
    if(nextion_page == 0){
      nextion_display("wifi",wifi_able,&nxSerial);
      nextion_display("min",EEPROM.read(eep_var[0]),&nxSerial);
      nextion_display("sec",EEPROM.read(eep_var[1]),&nxSerial);
      nextion_display("operation",operation,&nxSerial);
    }
  }else if(cmd_text=="send"){
    nextion_print(&nxSerial,temp_text);
  }else if(cmd_text=="run"){
    countdown = total_time();
    operation = true;
    for (uint8_t index = 0; index < TOTAL_RELAY; index++)
    {
      digitalWrite(Relay[index], true);  //plasma run here
    }
    prevUpdateTime = millis();
    nextion_display("operation",operation,&nxSerial);
  }else if(cmd_text=="stop"){
    for (uint8_t index = 0; index < TOTAL_RELAY; index++)
    {
      digitalWrite(Relay[index], false);  //plasma run here
    }
    operation = false;
    nextion_display("operation",operation,&nxSerial);
  }else if(cmd_text=="memo"){
    httpPOSTRequest("http://plasma.smarthive.kr/plasma/insecticide",temp_text);//http post bug dead
    nextion_print(&nxSerial,"page 0");
  }else if(cmd_text=="refresh"){
    httpPOSTRequest("http://plasma.smarthive.kr/plasma/refresh","null");//http post for getting setup data
    nextion_print(&nxSerial,"page 0");
    nextion_display("min",EEPROM.read(0),&nxSerial);
    nextion_display("sec",EEPROM.read(1),&nxSerial);
  }else if(cmd_text=="minute"){
    eep_change = true;
    EEPROM.write(eep_var[0],temp_text.toInt());
    nextion_print(&nxSerial,"page 0");
  }else if(cmd_text=="seconde"){
    eep_change = true;
    EEPROM.write(eep_var[1],temp_text.toInt());
    nextion_print(&nxSerial,"page 0");
  }
  /*****OFF_LINE_CMD*****/
  else if(uart_type){
    if(cmd_text=="ssid"){
      wifi_able = false;
      WiFi.disconnect(true);
      if(uart_type) Serial.print("ssid: ");
      if(temp_text.length() > 0){
        for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
          if(index < temp_text.length()){
            if(uart_type) Serial.print(temp_text[index]);
            ssid[index] = temp_text[index];
            EEPROM.write(eep_ssid[index], byte(temp_text[index]));
          }else{
            ssid[index] = 0x00;
            EEPROM.write(eep_ssid[index], byte(0x00));
          }
        }
        eep_change = true;
      }
      if(uart_type) Serial.println("");
    }else if(cmd_text=="pass"){
      wifi_able = false;
      WiFi.disconnect(true);
      if(uart_type) Serial.print("pass: ");
      if(temp_text.length() > 0){
        for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
          if(index < temp_text.length()){
            if(uart_type) Serial.print(temp_text[index]);
            password[index] = temp_text[index];
            EEPROM.write(eep_pass[index], byte(temp_text[index]));
          }else{
            password[index] = 0x00;
            EEPROM.write(eep_pass[index], byte(0x00));
          }
        }
        eep_change = true;
      }
      if(uart_type) Serial.println("");
    }else if(cmd_text=="wifi"){
      if(temp_text=="stop"){
        wifi_able = false;
        if(uart_type) Serial.print("WIFI disconnect");
        WiFi.disconnect(true);
      }else if(temp_text=="scan"){
        WIFI_scan(WiFi.status() == WL_CONNECTED);
      }else{
        wifi_connect();
      }
    }else if(uart_type && cmd_text=="help"){
      serial_command_help(&Serial);
    }else{ serial_err_msg(&Serial, command_buf); }
  }
  /*****OFF_LINE_CMD*****/
  else{ serial_err_msg(&Serial, command_buf); }
  if(eep_change){
    EEPROM.commit();
  }
}
void command_process(char ch, bool type_uart) {
  //uart_type = type_uart;
  uart_type=true;
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
  wifi_able = true;
  wifi_config();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  if(nextion_page==1) nextion_print(&nxSerial,"page_wifi.t_wifi.txt=\"Try to WIFI connect...\"");
  WiFi.begin(ssid, password);
  unsigned long wifi_config_update  = millis();
  while (WiFi.status() != WL_CONNECTED) {
    unsigned long update_time = millis();
    if(update_time - wifi_config_update > 5000){
      wifi_able = false;
      if(uart_type) Serial.println("WIFI fail");
      WiFi.disconnect(true);
      break;
    }
  }
  if(nextion_page==0)  nextion_display("page_main.wifi",wifi_able,&nxSerial);
  else if(nextion_page==1){
    if(wifi_able) nextion_print(&nxSerial,"page_wifi.t_wifi.txt=\"WIFI connected!\"");
    else          nextion_print(&nxSerial,"page_wifi.t_wifi.txt=\"password wrong!\\rpw: "+String(password)+"\"");
  }
}
/******************************************/
uint32_t total_time(){
  uint32_t time_hour = EEPROM.read(eep_var[0]);
  uint32_t time_min = EEPROM.read(eep_var[1]);
  if(time_min>=60) time_min = 59;
  return (time_hour*60) + time_min;
}
/***************Functions******************/
void setup() {
  Serial.begin(115200);
  nxSerial.begin(115200, SERIAL_8N1, 18, 19);
  //nxSerial.begin(115200, SERIAL_8N1, 16, 17);

  for (uint8_t index = 0; index < TOTAL_RELAY; index++)
  {
    pinMode(Relay[index], OUTPUT);
    digitalWrite(Relay[index], false);
  }

  if (!EEPROM.begin((EEPROM_SIZE_CONFIG*2) + (EEPROM_SIZE_VALUE))){
    if(uart_type){
      Serial.println("Failed to initialise eeprom");
      Serial.println("Restarting...");
    }
    delay(1000);
    ESP.restart();
  }
  for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
    ssid[index]     = EEPROM.read(eep_ssid[index]);
    password[index] = EEPROM.read(eep_pass[index]);
  }
  wifi_connect();
  serial_command_help(&Serial);
  Serial.println("System online");
  nextion_print(&nxSerial,"page 0");
}

// the loop function runs over and over again forever
void loop() {
  if (Serial.available()) command_process(Serial.read(), true);
  if (nxSerial.available()) command_process(nxSerial.read(), false);
  system_ctr(millis());
  page_change();
}

void page_change(){
  if(nextion_shift){
    nextion_shift = false;
    Serial.print("page "); Serial.println(nextion_page);
    if(nextion_page == 0){
      nextion_display("page_main.wifi",wifi_able,&nxSerial);
    }else if(nextion_page == 2){
      
    }
  }
}

void system_ctr(unsigned long millisec){
  if(millisec > prevUpdateTime + UPDATE_INTERVAL){
    prevUpdateTime = millisec;
    if(operation && countdown>0){
      countdown -=1;
      runtime   +=1;
      if(nextion_page == 0) nextion_display("progress",map(runtime, 0, total_time(), 0, 100),&nxSerial);
    }else if(runtime>0){
      operation = false;
      for (uint8_t index = 0; index < TOTAL_RELAY; index++)
      {
        digitalWrite(Relay[index], false);  //plasma run here
      }
      httpPOSTRequest("http://plasma.smarthive.kr/plasma/runtime",String(runtime));//http post runtime
      runtime=0;
      if(nextion_page == 0) nextion_display("operation",operation,&nxSerial);
    }
  } //routine
}

String httpPOSTRequest(String server_url, String send_data) {
  String response = "";
  if(wifi_able){
    WiFiClient client;
    HTTPClient http;
    http.begin(client, server_url);
    http.addHeader("Content-Type", "application/json");
    int response_code = http.POST("{\"data\":\""+send_data+"\"");
    response          = http.getString();
    http.end();
  }
  return response;
}////httpPOSTRequest_End

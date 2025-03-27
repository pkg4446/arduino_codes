#include <WiFi.h>
#include <HTTPClient.h>
#define SERIAL_MAX  128
HardwareSerial rootDvice(2);

#include "EEPROM.h"
#define   EEPROM_SIZE 24

/****************** EEPROM ******************/
const uint8_t eep_ssid[EEPROM_SIZE] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
const uint8_t eep_pass[EEPROM_SIZE] = {24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47};
/****************** EEPROM ******************/
char ssid[EEPROM_SIZE];
char password[EEPROM_SIZE]; //#234567!

unsigned long timer_restart = 0;
uint8_t       restart_count = 0;

char          deviceID[18];

//// ----------- Command  -----------
char    command_Buf[2][SERIAL_MAX];
uint8_t command_Num[2];
////--------------------- make_json -------------------////
String data_json(String mac,String type,String api,String data) {
  String response = "{\"HUB\":\""  + String(deviceID);
  response += "\",\"DVC\":\""      + mac;
  response += "\",\"TYPE\":\""     + type;
  response += "\",\"API\":\""      + api;
  response += "\",\"DATA\":"       + data;
  response += "}";
  return response;
}
////--------------------- httpPOSTRequest -------------////
void httpPOSTRequest(String httpRequestData,String mac) {
  String serverUrl = "http://192.168.1.36:3010/log/hub";   //API adress
  HTTPClient http;
  WiFiClient http_client;
  http.begin(http_client, serverUrl);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);
  
  int httpResponseCode = http.POST(httpRequestData);
  String response = http.getString();
  http.end(); // Free resources

  if(httpResponseCode == 200){
    String ack = String(mac) + " ACK\n";
    rootDvice.print(ack);
    if(response.length()>0)rootDvice.print(response+"\n");
  }else{
    Serial.print("HTTP code: ");
    Serial.println(httpResponseCode);
    ESP.restart();
  }
}
////--------------------- httpPOSTRequest -------------////
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
////--------------------- command service -------------////
////--------------------- service help ----------------////
void help() {
  Serial.println("************* help *************");
  Serial.println("reboot  * system reboot");
  Serial.println("ssid    * ex)ssid your ssid");
  Serial.println("pass    * ex)pass your password");
  Serial.println("wifi    * WIFI connet");
  Serial.println("********************************");
}
////--------------------- service serial --------------////
void service_serial(String cmd) {
  Serial.print("cmd: ");
  Serial.println(cmd);
  uint8_t cmd_index = 0;
  String command  = String_slice(&cmd_index, cmd, 0x20);
  String value    = String_slice(&cmd_index, cmd, 0x20);
  ////cmd start
  if(command=="reboot" || command=="wifi"){
    ESP.restart();
  }else if(command=="reg"){
    
  }else if(command=="ssid"){
    WiFi.disconnect(true);
    Serial.print("ssid: ");
    for (int index = 0; index < EEPROM_SIZE; index++) {
      if(index < value.length()){
        Serial.print(value[index]);
        ssid[index] = value[index];
        EEPROM.write(eep_ssid[index], byte(value[index]));
      }else{
        ssid[index] = 0x00;
        EEPROM.write(eep_ssid[index], byte(0x00));
      }      
    }
    Serial.println("");
    EEPROM.commit();
  }else if(command=="pass"){
    WiFi.disconnect(true);
    Serial.print("pass: ");
    for (int index = 0; index < EEPROM_SIZE; index++) {
      if(index < value.length()){
        Serial.print(value[index]);
        password[index] = value[index];
        EEPROM.write(eep_pass[index], byte(value[index]));
      }else{
        password[index] = 0x00;
        EEPROM.write(eep_pass[index], byte(0x00));
      }      
    }
    Serial.println("");
    EEPROM.commit();
  }else{
    help();
  }
}
////--------------------- service device --------------////
void service_device(String cmd) {
  Serial.print("mesh: ");
  Serial.println(cmd);

  uint8_t cmd_index = 0;
  String mac  = String_slice(&cmd_index, cmd, 0x20);
  String command = String_slice(&cmd_index, cmd, 0x20);
  String type = String_slice(&cmd_index, cmd, 0x20);
  String api  = String_slice(&cmd_index, cmd, 0x20);
  String data = String_slice(&cmd_index, cmd, 0x20);

  if (command == "post"){
    String httpRequestData = data_json(mac,type,api,data);
    httpPOSTRequest(httpRequestData,mac);
  }else{

  }
}
////--------------------- command call ----------------////
void command_process(bool device, char ch) {
  if (ch == '\n') {
    command_Buf[device][command_Num[device]] = 0x00;
    if(device)service_device(command_Buf[device]);
    else service_serial(command_Buf[device]);
    command_Num[device] = 0;
  }else if (ch != '\r' && (command_Num[device] < SERIAL_MAX - 1)){
    command_Buf[device][command_Num[device]++] = ch;
  }
}
////--------------------- httpPOSTRequest -------------////
////--------------------- setup -----------------------////
void setup() {
  Serial.begin(115200);
  rootDvice.begin(115200, SERIAL_8N1, 32, 33);

  if (!EEPROM.begin(EEPROM_SIZE*2)){
    Serial.println("Failed to initialise eeprom");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }

  for (int index = 0; index < EEPROM_SIZE; index++) {
    ssid[index]     = EEPROM.read(eep_ssid[index]);
    password[index] = EEPROM.read(eep_pass[index]);
  }

  Serial.println("------- wifi config -------");
  Serial.print("ssid: "); Serial.println(ssid);
  Serial.print("pass: "); Serial.println(password);
  Serial.println("---------------------------");

  Serial.println("ver 1.0.1");
  
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long wifi_config_update = 0UL;
  bool scan_flage = false;
  while (WiFi.status() != WL_CONNECTED) {
    if(Serial.available()) command_process(false,Serial.read());
    unsigned long update_time = millis();
    if(update_time - wifi_config_update > 8000){
      wifi_config_update = update_time;
      Serial.println("Connecting to scan");
      //auto connect try
      WiFi.disconnect(true);
      uint8_t networks = WiFi.scanNetworks();
      for (int index = 0; index < networks; ++index) {
        // Print SSID and RSSI for each network found
        String SSID = WiFi.SSID(index);
        //CNR_L580W_ 
        if(SSID.length()>9 && SSID[0] == 'C' && SSID[1] == 'N' && SSID[2] == 'R' && SSID[3] == '_' && SSID[4] == 'L' && SSID[5] == '5' && SSID[6] == '8' && SSID[7] == '0' && SSID[8] == 'W' && SSID[9] == '_'){
          WiFi.scanDelete();
          Serial.println("Found SSID : "); Serial.println(SSID);
          for (int index = 0; index < EEPROM_SIZE; index++) {
            if(index < SSID.length()){
              EEPROM.write(eep_ssid[index], byte(SSID[index]));
            }else{
              EEPROM.write(eep_ssid[index], byte(0x00));
            }      
          }
          String pass_value = "#234567!";
          for (int index = 0; index < EEPROM_SIZE; index++) {
            if(index < pass_value.length()){
              EEPROM.write(eep_pass[index], byte(pass_value[index]));
            }else{
              EEPROM.write(eep_pass[index], byte(0x00));
            }    
          }
          EEPROM.commit();
          delay(100);
          ESP.restart();
        }
      }

      if(scan_flage){
        Serial.println("------- WIFI lists -------");
        for (int index = 0; index < networks; ++index) {
          Serial.print("\t");Serial.print(index);Serial.print(": ");Serial.println(WiFi.SSID(index));
        }
        Serial.println("---------------------------");
        WiFi.scanDelete();
        update_time = millis();
        help();
        while (true)
        {
          if(millis()-update_time > 1000*60*3){ESP.restart();}
          if(Serial.available()) command_process(false,Serial.read());
        }
        break;
      }
      scan_flage = true;
    }
  }
  Serial.println("Connected to the WiFi network");

  for (uint8_t index = 0; index < 17; index++) {
    if(WiFi.macAddress()[index]==':'){
      deviceID[index] = '_';
    }else{
      deviceID[index] = WiFi.macAddress()[index];
    }
  }
  delay(100);
}//End Of Setup()

void loop() {
  if (rootDvice.available()) command_process(true,rootDvice.read());//post
  if (Serial.available()) command_process(false,Serial.read());
  unsigned long millisec = millis();
  mesh_restart(millisec);
}

void mesh_restart(unsigned long millisec){
  if(millisec - timer_restart > 1000*60){
    timer_restart = millisec;
    if(restart_count++ > 240) ESP.restart();
  }
}
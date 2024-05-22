#include <EEPROM.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient
#include <DS3231.h>

#define TOTAL_RELAY 10
#define EEPROM_SIZE 16
#define COMMAND_LENGTH  32
#define UPDATE_INTERVAL 1000L

DS3231 RTC_DS3231;
HardwareSerial nxSerial(2);
enum Func {
    Water_A = 0,
    Water_B,
    Cooler,
    Heater,
    Lamp_A,
    Lamp_B,
    Lamp_C,
    Circulater,
    Spare_A,
    Spare_B
};
/***************EEPROM*********************/
const uint8_t eep_ssid[EEPROM_SIZE] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
const uint8_t eep_pass[EEPROM_SIZE] = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
/***************EEPROM*********************/
/***************MQTT_CONFIG****************/
const char*     mqttServer    = "smarthive.kr";
const uint16_t  mqttPort      = 1883;
const char*     mqttUser      = "hive";
const char*     mqttPassword  = "hive";
const char*     topic_pub     = "SHS";

WiFiClient    mqtt_client;
PubSubClient  mqttClient(mqtt_client);
uint8_t       mqtt_count      = 0;
/***************MQTT_CONFIG****************/
/***************PIN_CONFIG*****************/
const int8_t Relay[TOTAL_RELAY] = {2,4,5,12,13,23,27,26,25,33};
/***************PIN_CONFIG*****************/
/***************Interval_timer*************/
unsigned long prevUpdateTime = 0L;
/***************Interval_timer*************/
/***************Variable*******************/
char    ssid[EEPROM_SIZE];
char    password[EEPROM_SIZE];
/***************Variable*******************/
typedef struct ctr_var{
    bool    enable;
    bool    state;
    uint8_t run;
    uint8_t stop;
}ctr_var;
/***************Variable*******************/
ctr_var water[2];   // run:동작_초, stop:정지_분
ctr_var temp_ctr;   // run:목표값,  stop:허용치
ctr_var circulate;  // run:동작_분, stop:정지_분
ctr_var lamp[3];    // run:시작시간,stop:정지시간
//이산화탄소 측정 추가
bool    wifi_able;
bool    uart_type = true;
/***************Variable*******************/
char    command_buf[COMMAND_LENGTH];
int8_t  command_num;
/***************Functions******************/
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  for (int index = 0; index < length; index++) {
    command_buf[index] = payload[index];
  }
  command_num = 0;
  Serial.println("MQTT");
  command_service(false);
}
/******************************************/
void send2Nextion(String cmd) {
  nxSerial.print(cmd);
  nxSerial.write(0xFF);
  nxSerial.write(0xFF);
  nxSerial.write(0xFF);
}
void Display(String IDs, uint16_t values) {
  String cmd;
  char buf[8] = {0};
  sprintf(buf, "%d", values);
  cmd = IDs + ".val=";
  cmd += buf;
  send2Nextion(cmd);
}
/******************************************/
void wifi_config() {
  if(uart_type){
    Serial.println("********* wifi config *********");
    Serial.print("your ssid: ");Serial.println(ssid);
    Serial.print("your pass: "); Serial.println(password);
    Serial.println("********* wifi config *********");
  }else{

  }
}
void command_help() {
  if(uart_type){
    Serial.println("************* help *************");
    Serial.println("time    * show system time");
    Serial.println("timeset * change system time");
    Serial.println("show    * wifi scan");
    Serial.println("ssid    * ex)ssid your ssid");
    Serial.println("pass    * ex)pass your password");
    Serial.println("wifi    * wifi connect");
    Serial.println("reboot  * system reboot");
    Serial.println("help    * this text");
    Serial.println("************* help *************");
  }
}
/******************************************/
void WIFI_scan(bool wifi_state){
  wifi_able = wifi_state;
  WiFi.disconnect(true);
  send2Nextion("page 1");//nextion page 이동
  if(uart_type) Serial.println("WIFI Scanning…");
  else{}
  uint8_t networks = WiFi.scanNetworks();
  if (networks == 0) {
      if(uart_type) Serial.println("no networks found");
  }else {
    if(uart_type){
      Serial.print(networks);
      Serial.println(" networks found");
      Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    }else{}
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
      }else{}
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
  }
  if(uart_type) Serial.println("");
  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();
  if(wifi_able){
    WiFi.begin(ssid, password);
  }
}
/******************************************/
void command_service(bool command_type){
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
    if(command_buf[index_check] == 0x00) break;
    temp_text += command_buf[index_check];
  }
  /**********/
  if(cmd_text=="time"){
    time_show();
  }else if(cmd_text=="timeset"){
    time_set();
  }
  /*****OFF_LINE_CMD*****/
  else if(command_type && cmd_text=="ssid"){
    wifi_able = false;
    WiFi.disconnect(true);
    if(uart_type) Serial.print("ssid: ");
    if(temp_text.length() > 0){
      for (int index = 0; index < EEPROM_SIZE; index++) {
        if(index < temp_text.length()){
          if(uart_type) Serial.print(temp_text[index]);
          ssid[index] = temp_text[index];
          EEPROM.write(eep_ssid[index], byte(temp_text[index]));
        }else{
          EEPROM.write(eep_ssid[index], byte(0x00));
        }
      }
      eep_change = true;
    }
    if(uart_type) Serial.println("");
  }else if(command_type && cmd_text=="pass"){
    wifi_able = false;
    WiFi.disconnect(true);
    if(uart_type) Serial.print("pass: ");
    if(temp_text.length() > 0){
      for (int index = 0; index < EEPROM_SIZE; index++) {
        if(index < temp_text.length()){
          if(uart_type) Serial.print(temp_text[index]);
          password[index] = temp_text[index];
          EEPROM.write(eep_pass[index], byte(temp_text[index]));
        }else{
          EEPROM.write(eep_pass[index], byte(0x00));
        }
      }
      eep_change = true;
    }
    if(uart_type) Serial.println("");
  }else if(command_type && cmd_text=="wifi"){
    if(temp_text=="stop"){
      wifi_able = false;
      if(uart_type) Serial.print("WIFI disconnect");
      WiFi.disconnect(true);
    }else{
      wifi_connect();
    }
  }else if(command_type && cmd_text=="scan"){
    WIFI_scan(WiFi.status() == WL_CONNECTED);
  }
  /*****OFF_LINE_CMD*****/
  else if(cmd_text=="reboot"){
    ESP.restart();
  }else if(cmd_text=="help"){
    command_help();
  }else{
    Serial.print("err: ");
    Serial.println(command_buf);
  }
  if(eep_change){
    EEPROM.commit();
  }
}
void command_process(char ch, bool type_uart) {
  uart_type = type_uart;
  if(ch=='\n'){
    command_buf[command_num] = 0x00;
    command_num = 0;
    command_service(true);
    memset(command_buf, 0x00, COMMAND_LENGTH);
  }else if(ch!='\r'){
    command_buf[command_num++] = ch;
    command_num %= COMMAND_LENGTH;
  }
}
/******************************************/
void mqtt_connect() {
  mqttClient.disconnect();
  if(wifi_able){
    //send2Nextion("MAC.txt=\"Try to connect WIFI\"");
    //send2Nextion("btWIFI.val=0");
    mqttClient.setServer(mqttServer, mqttPort);
    mqttClient.setCallback(mqtt_callback);

    char  deviceID[18];
    char  sendID[21]  = "ID=";

    for (int i = 0; i < 17; i++) {
      sendID[i + 3] = WiFi.macAddress()[i];
      deviceID[i]   = sendID[i + 3];
    }

    char* topic_sub = deviceID;
    char* sub_ID    = sendID;

    unsigned long WIFI_wait  = 0UL;
    bool mqtt_connected = true;
    while (!mqttClient.connected()) {
      if (millis() > WIFI_wait + 1000) {
        WIFI_wait = millis();
        if (!wifi_able){
          if(uart_type) Serial.println("WIFI was not connected");
          mqttClient.disconnect();
          return;
        }else if(mqttClient.connect(deviceID, mqttUser, mqttPassword )) {
          if(uart_type) Serial.println("connected");
        } else {
          if(uart_type){
            Serial.print("failed with state ");
            Serial.print(mqttClient.state());
          }
          mqtt_connected = false;
          break;
        }
      }
    }
    if(uart_type) Serial.print("MQTT Connected ");
    if(mqtt_connected){
      mqttClient.subscribe(topic_sub);
      mqttClient.publish(topic_pub, sub_ID);
      if(uart_type) Serial.println(sub_ID);
    }else{
      if(uart_type) Serial.println("fail");
    }
  }
}

void wifi_connect() {
  wifi_able = true;
  wifi_config();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long wifi_config_update  = millis();
  while (WiFi.status() != WL_CONNECTED) {
    unsigned long update_time = millis();
    if(update_time - wifi_config_update > 5000){
      wifi_able = false;
      if(uart_type) Serial.println("WIFI fail");
      break;
    }
  }
}

void time_show(){
  bool century = false;
  bool h12Flag;
  bool pmFlag;
  if(uart_type){
    Serial.print('2');
    if(century)Serial.print('1');
    else Serial.print('0');
    uint8_t year = RTC_DS3231.getYear();
    if(year<10) Serial.print('0');
    Serial.print(year);
    Serial.print('/');
    Serial.print(RTC_DS3231.getMonth(century));
    Serial.print('/');
    Serial.print(RTC_DS3231.getDate());
    Serial.print(' ');
    uint8_t days = RTC_DS3231.getDoW();
    Serial.print(' ');
    Serial.print(RTC_DS3231.getHour(h12Flag, pmFlag));
    Serial.print(':');
    Serial.print(RTC_DS3231.getMinute());
    Serial.print(':');
    Serial.print(RTC_DS3231.getSecond());

    Serial.print(" board temp:");
    Serial.println(RTC_DS3231.getTemperature(), 2);
  }
}

void time_set(){
  /*
  JS CODE
  {
    let now  = new Date();
    let time = `time ${now.getFullYear()} ${now.getMonth()+1} ${now.getDate()} ${now.getUTCDay()} ${now.getHours()} ${now.getMinutes()} ${now.getSeconds()}`;
  }
  DATA
    timeset 2024 5 22 3 15 48 39
  */
  uint8_t time_data[6];
  uint8_t time_index = 0;
  String time_text = "";
  for(uint8_t index=8; index<COMMAND_LENGTH; index++){
    if(command_buf[index] == 0x20 || command_buf[index] == 0x00){
      time_data[time_index++] = (time_text.toInt())%100;
      time_text = "";
      index+=1; //passing to 0x20
      if(time_index > 5) break;
    };
    time_text += command_buf[index];
  }
  if(time_data[3]==0) time_data[3] = 7;
  RTC_DS3231.setYear(time_data[0]);
  RTC_DS3231.setMonth(time_data[1]);
  RTC_DS3231.setDate(time_data[2]);
  RTC_DS3231.setDoW(time_data[3]);
  RTC_DS3231.setHour(time_data[4]);
  RTC_DS3231.setMinute(time_data[5]);
  RTC_DS3231.setSecond(time_data[6]);
}
/***************Functions******************/

void setup() {
  Serial.begin(115200);
  nxSerial.begin(115200, SERIAL_8N1, 18, 19);
  Wire.begin();

  for (uint8_t index = 0; index < TOTAL_RELAY; index++)
  {
    pinMode(Relay[index], OUTPUT);
    digitalWrite(Relay[index], false);
  }

  if (!EEPROM.begin(EEPROM_SIZE*2)){
    if(uart_type){
      Serial.println("Failed to initialise eeprom");
      Serial.println("Restarting...");
    }
    delay(1000);
    ESP.restart();
  }
  for (int index = 0; index < EEPROM_SIZE; index++) {
    ssid[index]     = EEPROM.read(eep_ssid[index]);
    password[index] = EEPROM.read(eep_pass[index]);
  }
  wifi_connect();
  command_help();
  if(uart_type) Serial.println("System online");
  send2Nextion("page 0");
}

// the loop function runs over and over again forever
void loop() {
  if(wifi_able){
    if (mqttClient.connected()){mqttClient.loop();}
    else{mqtt_connect();}
  }
  if (Serial.available()) command_process(Serial.read(),true);
  if (nxSerial.available()) command_process(nxSerial.read(),false);
}
#include <WiFi.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient
#include <HTTPClient.h>
#include <EEPROM.h>

#define EEPROM_SIZE     16
#define COMMAND_LENGTH  128
#define SERIAL_LENGTH   24
HardwareSerial rootDvice(2);

/******************EEPROM******************/
const uint8_t eep_ssid[EEPROM_SIZE] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
const uint8_t eep_pass[EEPROM_SIZE] = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
/******************EEPROM******************/
char ssid[EEPROM_SIZE];
char password[EEPROM_SIZE]; //#234567!

const char* mqttServer    = "smarthive.kr";
const int   mqttPort      = 1883;
const char* mqttUser      = "hive";
const char* mqttPassword  = "hive";
const char* topic_pub     = "SHS";
char        deviceID[18];
char        sendID[21]    = "ID=";

uint8_t mqtt_count = 0;

WiFiClient mqtt_client;
PubSubClient mqttClient(mqtt_client);

struct dataSet {
  String TYPE;
  String MODULE;
  String COMMEND;
  String VALUE1;
  String VALUE2;
  String VALUE3;
  String VALUE4;
};

const uint8_t led_pin[8] = {12,13,14,16,17,25,26,27};
const uint8_t led_sw[2]  = {32,33};
const uint8_t relay[2]   = {22,23};

//// ------------ MQTT Callback ------------
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  char mqtt_buf[COMMAND_LENGTH] = "";
  for (int i = 0; i < length; i++) {
    mqtt_buf[i] = payload[i];
  }
  Serial.print("Message arrived: ");
  rootDvice.print(mqtt_buf);
  Serial.println(mqtt_buf);
}

//// ----------- Command  -----------
void command_helf_wifi() {
  Serial.println("********** help **********");
  Serial.println("help  this text");
  Serial.println("show  wifi scan");
  Serial.println("ssid  ex)ssid your ssid");
  Serial.println("pass  ex)pass your password");
  Serial.println("********** help **********");
  Serial.print("your ssid: "); Serial.println(ssid);
  Serial.print("your pass: "); Serial.println(password);
  Serial.println("------- wifi config -------");
}
void command_helf() {
  Serial.println("********** help **********");
  Serial.println("help  this text");
  Serial.println("show  wifi scan");
  Serial.println("rest  reboot");
  Serial.println("updt  firmware update check");
  Serial.println("********** help **********");
}
/**********************************************/
/**********************************************/
char    command_buf[COMMAND_LENGTH];
int8_t  command_num;

void command_Service() {
  struct dataSet dataSend;
  dataSend.MODULE  = strtok(command_buf, "=");
  dataSend.TYPE    = strtok(NULL, "=");
  dataSend.COMMEND = strtok(NULL, "=");
  dataSend.VALUE1  = strtok(NULL, "=");
  dataSend.VALUE2  = strtok(NULL, "=");
  dataSend.VALUE3  = strtok(NULL, "=");
  dataSend.VALUE4  = strtok(NULL, 0x00);
  if (dataSend.TYPE != "P") {
    httpPOSTRequest(&dataSend);
  }
}//command_service() END

void command_Process(char ch) {
  if(ch=='\n'){
    command_buf[command_num] = 0x00;
    command_num = 0;
    command_Service();
    //memset(command_buf, 0x00, COMMAND_LENGTH);
  }else if(ch!='\r'){
    command_buf[command_num++] = ch;
    command_num %= COMMAND_LENGTH;
  }
}
/**********************************************/
/**********************************************/
void WIFI_scan(){
  WiFi.disconnect(true);
    Serial.println("WIFI Scanning…");
    uint8_t networks = WiFi.scanNetworks();
    if (networks == 0) {
        Serial.println("no networks found");
    } else {
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
        switch (WiFi.encryptionType(index))
        {
        case WIFI_AUTH_OPEN:
            Serial.print("open");
            break;
        case WIFI_AUTH_WEP:
            Serial.print("WEP");
            break;
        case WIFI_AUTH_WPA_PSK:
            Serial.print("WPA");
            break;
        case WIFI_AUTH_WPA2_PSK:
            Serial.print("WPA2");
            break;
        case WIFI_AUTH_WPA_WPA2_PSK:
            Serial.print("WPA+WPA2");
            break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
            Serial.print("WPA2-EAP");
            break;
        case WIFI_AUTH_WPA3_PSK:
            Serial.print("WPA3");
            break;
        case WIFI_AUTH_WPA2_WPA3_PSK:
            Serial.print("WPA2+WPA3");
            break;
        case WIFI_AUTH_WAPI_PSK:
            Serial.print("WAPI");
            break;
        default:
            Serial.print("unknown");
        }
        Serial.println();
        delay(10);
      }
    }
    Serial.println("");
    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();
    WiFi.begin(ssid, password);
}
/**********************************************/
/**********************************************/
char    Serial_buf[SERIAL_LENGTH];
int8_t  Serial_num;

void wifi_config_change() {
  String cmd_text   = "";
  String temp_text  = "";
  bool   eep_change = false;
  for(uint8_t index_check=0; index_check<4; index_check++){
    if(Serial_buf[index_check] == 0x32) break;
    cmd_text += Serial_buf[index_check];
  }
  for(uint8_t index_check=5; index_check<COMMAND_LENGTH; index_check++){
    if(Serial_buf[index_check] == 0x00) break;
    temp_text += Serial_buf[index_check];
  }
  
  if(cmd_text=="help"){
    command_helf_wifi();
  }else if(cmd_text=="show"){
    WIFI_scan();
  }else if(cmd_text=="ssid"){
    WiFi.disconnect(true);
    Serial.print("ssid=");
    if(temp_text.length() > 0){
      for (int index = 0; index < EEPROM_SIZE; index++) {
        if(index < temp_text.length()){
          Serial.print(temp_text[index]);
          ssid[index] = temp_text[index];
          EEPROM.write(eep_ssid[index], byte(temp_text[index]));
        }else{
          EEPROM.write(eep_ssid[index], byte(0x00));
        }
      }
      eep_change = true;
    }
    Serial.println("");

  }else if(cmd_text=="pass"){
    WiFi.disconnect(true);
    Serial.print("pass=");
    if(temp_text.length() > 0){
      for (int index = 0; index < EEPROM_SIZE; index++) {
        if(index < temp_text.length()){
          Serial.print(temp_text[index]);
          password[index] = temp_text[index];
          EEPROM.write(eep_pass[index], byte(temp_text[index]));
        }else{
          EEPROM.write(eep_pass[index], byte(0x00));
        }
      }
      eep_change = true;
    }
    Serial.println("");

  }else{
    Serial.println(Serial_buf);
  }
  if(eep_change){
    EEPROM.commit();
    WiFi.begin(ssid, password);
    if(WiFi.status() == WL_CONNECTED) ESP.restart();
  }
}//wifi_config_change() END
void serail_Process_wifi(char ch) {
  if(ch=='\n'){
    Serial_buf[Serial_num] = 0x00;
    Serial_num = 0;
    wifi_config_change();
    //memset(Serial_buf, 0x00, SERIAL_LENGTH);
  }else if(ch!='\r'){
    Serial_buf[Serial_num++] = ch;
    Serial_num %= SERIAL_LENGTH;
  }
}
/**********************************************/
/**********************************************/
void serial_service() {
  String cmd_text   = "";
  String temp_text  = "";
  bool   eep_change = false;
  for(uint8_t index_check=0; index_check<4; index_check++){
    if(Serial_buf[index_check] == 0x32) break;
    cmd_text += Serial_buf[index_check];
  }
  for(uint8_t index_check=5; index_check<COMMAND_LENGTH; index_check++){
    if(Serial_buf[index_check] == 0x00) break;
    temp_text += Serial_buf[index_check];
  }
  
  if(cmd_text=="help"){
    command_helf();
  }else if(cmd_text=="show"){
    WIFI_scan();
  }else if(cmd_text=="rest"){
    ESP.restart();
  }else{
    Serial.println(Serial_buf);
  }
}//serial_service() END

void serail_Process(char ch) {
  if(ch=='\n'){
    Serial_buf[Serial_num] = 0x00;
    Serial_num = 0;
    serial_service();
    //memset(Serial_buf, 0x00, SERIAL_LENGTH);
  }else if(ch!='\r'){
    Serial_buf[Serial_num++] = ch;
    Serial_num %= SERIAL_LENGTH;
  }
}

void setup() {
  Serial.begin(115200);
  rootDvice.begin(115200, SERIAL_8N1, 18, 19);

  for(uint8_t index=0; index<8; index++){
    pinMode(led_pin[index], OUTPUT);
    digitalWrite(led_pin[index], true);
  }
  for(uint8_t index=0; index<2; index++){
    pinMode(led_sw[index], OUTPUT);
    pinMode(relay[index], OUTPUT);
    digitalWrite(led_sw[index], true);
    digitalWrite(relay[index], false);
  }

  if (!EEPROM.begin(EEPROM_SIZE*2)){
    Serial.println("Failed to initialise eeprom");
    Serial.println("Restarting...");
    digitalWrite(led_pin[0], false);
    delay(1000);
    ESP.restart();
  }

  for (int index = 0; index < EEPROM_SIZE; index++) {
    ssid[index]     = EEPROM.read(eep_ssid[index]);
    password[index] = EEPROM.read(eep_pass[index]);
  }
  
  command_helf_wifi();

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long wifi_config_update = 0UL;
  while (WiFi.status() != WL_CONNECTED) {
    if (Serial.available()) serail_Process_wifi(Serial.read());
    /*
    unsigned long update_time = millis();
    if(update_time - wifi_config_update > 3000){
      digitalWrite(led_pin[6], false);
      wifi_config_update = update_time;
      Serial.println("Connecting to WiFi..");
    }
    */
  }
  digitalWrite(led_pin[6], true);
  Serial.println("Connected to the WiFi network");

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqtt_callback);

  for (int i = 0; i < 17; i++) {
    sendID[i + 3] = WiFi.macAddress()[i];
    deviceID[i]   = sendID[i + 3];
  }
  
  char* topic_sub = deviceID;
  char* sub_ID    = sendID;
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect(deviceID, mqttUser, mqttPassword )) {
      Serial.println("connected");
      digitalWrite(led_pin[3], true);
      mqtt_count = 0;
    } else {
      digitalWrite(led_pin[3], false);
      Serial.print("failed with state ");
      Serial.print(mqttClient.state());
      delay(2000);
      if(mqtt_count++ > 240) ESP.restart();
    }
  }

  mqttClient.subscribe(topic_sub);
  mqttClient.publish(topic_pub, sub_ID);

  Serial.print("subscribe: ");
  Serial.print(topic_sub);
  Serial.println(" - MQTT Connected");
  Serial.println("ver 1.0.0");

  command_helf();
}//End Of Setup()

void reconnect(){
  char* topic_sub = deviceID;
  char* sub_ID    = sendID;
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect(deviceID, mqttUser, mqttPassword )) {
      Serial.println("connected");
      digitalWrite(led_pin[3], true);
      mqtt_count = 0;
    } else {
      digitalWrite(led_pin[3], false);
      Serial.print("failed with state ");
      Serial.println(mqttClient.state());
      delay(1000);
      if(mqtt_count++ > 240) ESP.restart();
    }
  }
}

void loop() {
  if (mqttClient.connected()){mqttClient.loop();}
  else{reconnect();}
  if (rootDvice.available()) command_Process(rootDvice.read());//post
  if (Serial.available()) serail_Process(Serial.read());
  unsigned long millisec = millis();
  mesh_restart(millisec);
}

void httpPOSTRequest(struct dataSet *ptr) {
  String serverUrl = "http://dev.smarthive.kr/reg/hive/";   //API adress
  HTTPClient http;
  WiFiClient http_client;
  http.begin(http_client, serverUrl);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String httpRequestData = (String)"FARM="  + deviceID     +
                           "&MODULE="       + ptr->MODULE  +
                           "&TYPE="         + ptr->TYPE    +
                           "&COMMEND="      + ptr->COMMEND +
                           "&VALUE1="       + ptr->VALUE1  +
                           "&VALUE2="       + ptr->VALUE2  +
                           "&VALUE3="       + ptr->VALUE3  +
                           "&VALUE4="       + ptr->VALUE4;

  int httpResponseCode = http.POST(httpRequestData);
  Serial.print(httpRequestData);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  http.end();           // Free resources
}////httpPOSTRequest_End

unsigned long timer_restart = 0;
uint8_t restart_count       = 0;
void mesh_restart(unsigned long millisec){
  if(millisec - timer_restart > 1000*60){
    timer_restart = millisec;
    if(restart_count++ > 240) ESP.restart();
  }
}
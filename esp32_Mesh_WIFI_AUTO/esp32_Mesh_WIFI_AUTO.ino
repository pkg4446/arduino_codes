#include <WiFi.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient
#include <HTTPClient.h>
#define SERIAL_MAX  128
HardwareSerial rootDvice(2);

#include "EEPROM.h"
#define   EEPROM_SIZE 24

/******************EEPROM******************/
const uint8_t eep_ssid[EEPROM_SIZE] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
const uint8_t eep_pass[EEPROM_SIZE] = {24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47};
/******************EEPROM******************/
char ssid[EEPROM_SIZE];
char password[EEPROM_SIZE]; //#234567!

unsigned long timer_restart = 0;
uint8_t     restart_count   = 0;

const char* mqttServer    = "smarthive.kr";
const int   mqttPort      = 1883;
const char* mqttUser      = "hive";
const char* mqttPassword  = "hive";
const char* topic_pub     = "SHS";
char        deviceID[18];
char        sendID[21]    = "ID=";

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

//// ------------ MQTT Callback ------------
void callback(char* topic, byte* payload, unsigned int length) {
  char mqtt_buf[SERIAL_MAX] = "";
  for (int i = 0; i < length; i++) {
    mqtt_buf[i] = payload[i];
  }
  Serial.print("Message arrived: ");
  rootDvice.print(mqtt_buf);
  Serial.println(mqtt_buf);
}

//// ----------- Command  -----------
char    command_Buf[SERIAL_MAX];
int8_t  command_Num;

void command_Service() {
  struct dataSet dataSend;
  dataSend.MODULE  = strtok(command_Buf, "=");
  dataSend.TYPE    = strtok(NULL, "=");
  dataSend.COMMEND = strtok(NULL, "=");
  dataSend.VALUE1  = strtok(NULL, "=");
  dataSend.VALUE2  = strtok(NULL, "=");
  dataSend.VALUE3  = strtok(NULL, "=");
  dataSend.VALUE4  = strtok(NULL, ";");
  if (dataSend.TYPE != "P") {
    httpPOSTRequest(&dataSend);
  }
}//Command_service() END

void command_Process() {
  char ch;
  ch = rootDvice.read();
  switch (ch) {
    case ';':
      command_Buf[command_Num] = ';';
      command_Buf[command_Num+1] = 0x00;
      Serial.println(command_Buf);
      command_Service();
      command_Num = 0;
      break;
    default:
      command_Buf[command_Num++] = ch;
      command_Num %= SERIAL_MAX;
      break;
  }
}

char    Serial_buf[SERIAL_MAX];
int8_t  Serial_num;

void wifi_config_change() {
  String at_cmd     = strtok(Serial_buf, "=");
  String ssid_value = strtok(NULL, "=");
  String pass_value = strtok(NULL, ";");
  if(at_cmd=="AT+WIFI"){
    Serial.print("ssid_value=");
    for (int index = 0; index < EEPROM_SIZE; index++) {
      if(index < ssid_value.length()){
        Serial.print(ssid_value[index]);
        EEPROM.write(eep_ssid[index], byte(ssid_value[index]));
      }else{
        EEPROM.write(eep_ssid[index], byte(0x00));
      }      
    }
    Serial.println("");
    Serial.print("pass_value=");
    for (int index = 0; index < EEPROM_SIZE; index++) {
      if(index < pass_value.length()){
        Serial.print(pass_value[index]);
        EEPROM.write(eep_pass[index], byte(pass_value[index]));
      }else{
        EEPROM.write(eep_pass[index], byte(0x00));
      }    
    }
    Serial.println("");
    EEPROM.commit();
    ESP.restart();
  }else{
    Serial.println(Serial_buf);
  }
}//Command_service() END

void Serial_process() {
  char ch;
  ch = Serial.read();
  switch ( ch ) {
    case ';':
      Serial_buf[Serial_num] = 0x00;
      wifi_config_change();
      Serial_num = 0;
      break;
    default :
      Serial_buf[ Serial_num ++ ] = ch;
      Serial_num %= SERIAL_MAX;
      break;
  }
}

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
    if (Serial.available()) Serial_process();
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
        Serial.println("------- wifi change -------");
        Serial.println("    ;AT+WIFI=SSID=PASS;");
        Serial.println("---------------------------");
        update_time = millis();
        while (true)
        {
          if(millis()-update_time > 1000*60*3){ESP.restart();}
          if(Serial.available()) Serial_process();
        }
        break;
      }
      scan_flage = true;
    }
  }
  Serial.println("Connected to the WiFi network");

  for (int i = 0; i < 17; i++) {
    sendID[i + 3] = WiFi.macAddress()[i];
    deviceID[i]   = sendID[i + 3];
  }
  mqtt_connect();
}//End Of Setup()

void mqtt_connect(){
  mqttClient.disconnect();

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  char* topic_sub = deviceID;
  char* sub_ID    = sendID;
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect(deviceID, mqttUser, mqttPassword )) {
      Serial.println("connected");
      ESP.restart();
    } else {
      Serial.print("failed with state ");
      Serial.print(mqttClient.state());
      delay(1000);
      if(restart_count++ > 60) ESP.restart();
    }
  }
  mqttClient.subscribe(topic_sub);
  mqttClient.publish(topic_pub, sub_ID);
  Serial.print("subscribe: ");
  Serial.print(topic_sub);
  Serial.println(" - MQTT Connected");
}

void loop() {
  if (mqttClient.connected()){mqttClient.loop();}
  else{mqtt_connect();}
  if (rootDvice.available()) command_Process();//post
  if (Serial.available()) Serial_process();
  unsigned long millisec = millis();
  mesh_restart(millisec);
}

void httpPOSTRequest(struct dataSet *ptr) {
  String serverUrl = "http://dev.smarthive.kr/reg/hive/";   //API adress
  HTTPClient http;
  WiFiClient http_client;
  http.begin(http_client, serverUrl);

  http.addHeader("Content-Type", "application/json");
  String httpRequestData = "{\"FARM\":\"";
  httpRequestData += deviceID;
  httpRequestData += "\",\"MODULE\":\"";
  httpRequestData += ptr->MODULE;
  httpRequestData += "\",\"TYPE\":\"";
  httpRequestData += ptr->TYPE;
  httpRequestData += "\",\"COMMEND\":\"";
  httpRequestData += ptr->COMMEND;
  httpRequestData += "\",\"VALUE1\":\"";
  httpRequestData += ptr->VALUE1;
  httpRequestData += "\",\"VALUE2\":\"";
  httpRequestData += ptr->VALUE2;
  httpRequestData += "\",\"VALUE3\":\"";
  httpRequestData += ptr->VALUE3;
  httpRequestData += "\",\"VALUE4\":\"";
  httpRequestData += ptr->VALUE4; 
  httpRequestData += "\"}";
  
  int httpResponseCode = http.POST(httpRequestData);
  Serial.print(httpRequestData);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  http.end();           // Free resources
}////httpPOSTRequest_End

void mesh_restart(unsigned long millisec){
  if(millisec - timer_restart > 1000*60){
    timer_restart = millisec;
    if(restart_count++ > 240) ESP.restart();
  }
}
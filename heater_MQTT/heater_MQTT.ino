#include  <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
#include  <PubSubClient.h> //https://github.com/knolleary/pubsubclient
#include  "EEPROM.h"
#define   EEPROM_SIZE 2
#define   SERIAL_MAX  32

#include <SHT1x.h>
#define dataPin  23   //MOSI
#define clockPin 18   //SCK
SHT1x sht1x(dataPin, clockPin);

unsigned long prevUpdateSensor  = 0UL;
//const int8_t Relay[6]   = {5, 17, 16, 4, 2, 15};
//const int8_t Button[4]  = {13, 12, 14, 27};
//const int8_t ADC[5]     = {33, 32, 34, 39, 35};
//const int8_t DAC[2]     = {25, 26};

//// ------------ Pin out D ------------
const int8_t  Relay_P       = 17;

//// ------------ MQTT ------------
const char*   mqttServer    = "smarthive.kr";
const int     mqttPort      = 1883;
const char*   mqttUser      = "hive";
const char*   mqttPassword  = "hive";
const char*   topic_pub     = "server";
char          deviceID[18];

//// ------------ EEPROM ------------
const uint8_t HEAT  = 1;

//// ------------ VALUE ------------
const uint8_t gap   = 1;
uint8_t temp        = 30;

//// ------------ Flage ------------
boolean operate     = false;

//// ------------ Count ------------
uint8_t tempCount   = 235;

WiFiClient    espClient;
PubSubClient  mqttClient(espClient);

//// ------------ ATCode ------------
void ATCode(char* commend, char* value) {
  String strCommend = commend;
  String intValue   = value;
  uint8_t commend_value = 0;

  if (strCommend == "AT+HON" || strCommend == "AT+sTPD" || strCommend == "AT+sTPR") commend_value = intValue.toInt();
  if (commend_value > 40) commend_value = 40;
  //// ------------ Monitering ------------
  if (strCommend == "AT+HON" || strCommend == "AT+HOFF" || strCommend == "AT+sTPD" || strCommend == "AT+sTPR" ) {
    if (strCommend == "AT+HON") {
      if (commend_value != temp) {
        EEPROM.write(HEAT, commend_value);
        EEPROM.commit();
        temp = commend_value;
      }
      mqttClient.publish(topic_pub, "AT+ON");
      commend = "AT+Heat";
      operate = true;//on
    } else if (strCommend == "AT+HOFF") {
      mqttClient.publish(topic_pub, "AT+OFF");
      operate = false; //off
    }

    uint8_t MQTT_COUNT = 0;
    char MQTT_TEXT[64] = "";
    for (int i = 0; i < 17; i++) {
      MQTT_TEXT[MQTT_COUNT++] = deviceID[i];
    }
    MQTT_TEXT[MQTT_COUNT++] = '=';
    for (int i = 0; i < strlen(commend); i++) {
      MQTT_TEXT[MQTT_COUNT++] = commend[i];
    }
    if (commend_value) {
      MQTT_TEXT[MQTT_COUNT++] = '=';
      for (int i = 0; i < strlen(value); i++) {
        MQTT_TEXT[MQTT_COUNT++] = value[i];
      }
    } else {
      MQTT_TEXT[MQTT_COUNT++] = '=';
      MQTT_TEXT[MQTT_COUNT++] = '0';
    }
    Serial.print("MQTT_TEXT: ");
    Serial.println(MQTT_TEXT);
    mqttClient.publish(topic_pub, MQTT_TEXT);
  }

}//// ------------ End Of ATCode ------------

//// ------------ MQTT Callback ------------
void callback(char* topic, byte * payload, unsigned int length) {
  char mqtt_buf[SERIAL_MAX] = "";
  for (int i = 0; i < length; i++) {
    mqtt_buf[i] = payload[i];
  }
  char* StrCommend = strtok(mqtt_buf, "=");
  char* StrValue   = strtok(NULL, " ");
  ATCode(StrCommend, StrValue);
}
//// ------------ End Of MQTT Callback ------------

//// ------------ wifiConnect ------------
void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  wm.setConfigPortalTimeout(120);
  bool res;
  res = wm.autoConnect("SmartHiveHeater");
  if (!res) {
    Serial.println("Failed to connect");
    wm.resetSettings();
    return;
  }
  Serial.println("WiFi Connected");

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  unsigned long WIFI_TXT  = 0UL;
  while (!mqttClient.connected()) {
    if (millis() > WIFI_TXT + 1000) {
      WIFI_TXT = millis();
      if (mqttClient.connect("ESP32Client", mqttUser, mqttPassword )) {
        Serial.println("connected");
      } else {
        Serial.print("failed with state ");
        Serial.print(mqttClient.state());
      }
    }
  }
  char sendID[20]   = "ID=";
  for (int i = 0; i < 17; i++) {
    sendID[i + 3] = WiFi.macAddress()[i];
    deviceID[i]   = sendID[i + 3];
  }
  char* topic_sub = deviceID;
  char* sub_ID    = sendID;
  //// ------------ LCD ------------
  String LCD_MAC  = "MAC.txt=\"";

  //// ------------ MQTT Communicate ------------
  mqttClient.subscribe(topic_sub);
  mqttClient.publish(topic_pub, sub_ID);

  Serial.println("MQTT Connected");
}//// ------------ End Of wifiConnect() ------------


//// ------------ setup ------------
void setup() {
  Serial.begin(115200);

  if (!EEPROM.begin(EEPROM_SIZE)) Serial.println("failed to initialise EEPROM");

  //// ------------ Pin Out ------------
  pinMode(Relay_P,   OUTPUT);

  //// ------------ EEPROM ------------
  temp  = byte(EEPROM.read(HEAT));

  Serial.println("System Boot");
  char buff[3] = {int(temp / 10) + '0', int(temp % 10) + '0', ';'};
  wifiConnect();
  ATCode("AT+HON", strtok(buff, ";"));
  Serial.println("System all green");
}//// ------------ End Of Setup() ------------

//// ------------ loop ------------
void loop() {
  mqttClient.loop();
  Sensor(millis());
}//// ------------ End Of loop() ------------

//// ------------ Sensor ------------
void Sensor(unsigned long currentTime) {
  if (currentTime > prevUpdateSensor + 1000) {
    prevUpdateSensor = currentTime;
    const uint16_t readTemp = sht1x.readTemperatureC();
    const uint16_t readHumi = sht1x.readHumidity();
    Serial.print("operate-");
    Serial.print(operate);
    Serial.print("    TemperatureC: ");
    Serial.print(readTemp);
    Serial.print("    Humidity: ");
    Serial.print(readHumi);
    Serial.print("    setTemp: ");
    Serial.print(temp);
    Serial.print("    tempCount: ");
    Serial.println(tempCount);
    if (tempCount >= 240 ) {
      char sTPD[3] = {int(readHumi / 10) + '0', int(readHumi % 10) + '0', ';'};
      char sTPR[3] = {int(readTemp / 10) + '0', int(readTemp % 10) + '0', ';'};
      ATCode("AT+sTPD", strtok(sTPD, ";"));
      ATCode("AT+sTPR", strtok(sTPR, ";"));
      tempCount = 0;
    } else {
      tempCount++;
    }
    if (operate) {
      if (readTemp < temp - gap) {
        digitalWrite(Relay_P, true);
      } else if (readTemp > temp + gap) {
        digitalWrite(Relay_P, false);
      }
    } else {
      digitalWrite(Relay_P, false);
    }
  }
}

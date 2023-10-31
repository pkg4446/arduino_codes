#include <WiFiManager.h>   //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>  //https://github.com/knolleary/pubsubclient
#include "EEPROM.h"
#define EEPROM_SIZE 28
#define SERIAL_MAX 32
HardwareSerial nxSerial(2);

unsigned long prevUpdatePlasma  = 0UL;

//const int8_t Relay[6]  = {5, 17, 16, 4, 2, 15};
//const int8_t Button[4] = {13, 12, 14, 27};
//const int8_t ADC[5]    = {33, 32, 34, 39, 35};
//const int8_t DAC[2]    = {25, 26};

//// ------------ Pin out D ------------
const int8_t Relay_PA = 17;
const int8_t Relay_PB = 16;
const int8_t Relay_A  = 4;
const int8_t Relay_B  = 2;
const int8_t Relay_C  = 15;
//// ------------ MQTT ------------
const char* mqttServer   = "smarthive.kr";
const int   mqttPort     = 1883;
const char* mqttUser     = "hive";
const char* mqttPassword = "hive";
const char* topic_pub    = "WAREHOSE";
char deviceID[18];

//// ------------ EEPROM ------------
const uint8_t WIFI  = 1;
const uint8_t T_C   = 10;
const uint8_t T_ON  = 11;

//// ------------ Value ------------
uint8_t TimeCount, onTime;
uint16_t offTime;

//// ------------ Flage ------------
boolean wifiSet   = false;
boolean wifiOnOff = false;

boolean plasmaRun   = false;
boolean PlasmaOnOff = false;

//// ------------ Timer ------------
uint8_t HOUR  = 0;
uint8_t MIN   = 0;
uint8_t SEC   = 0;

uint16_t DoorCount  = 0;
uint32_t PlasmaOn   = true;
uint32_t PlasmaOff  = true;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

////AT Code for chage the Something//////////////////////////
char Serial_buf[SERIAL_MAX];
int16_t Serial_num;
//// ------------ ATCode ------------
void ATCode(char* commend, char* value) {
  String strCommend = commend;
  String intValue = value;
  uint8_t commend_value = 0;
  if (strCommend != "AT+test") commend_value = intValue.toInt();
  //// ------------ Monitering ------------
  if (wifiOnOff && commend) {
    if (strCommend == "AT+ON" || strCommend == "AT+OFF" || strCommend == "AT+TC" || strCommend == "AT+TO" || strCommend == "AT+FAN") {
      uint8_t MQTT_COUNT = 0;
      char MQTT_TEXT[64] = "";
      for (int i = 0; i < 17; i++) {
        MQTT_TEXT[MQTT_COUNT++] = deviceID[i];
      }
      MQTT_TEXT[MQTT_COUNT++] = '=';
      for (int i = 0; i < strlen(commend); i++) {
        MQTT_TEXT[MQTT_COUNT++] = commend[i];
      }
      MQTT_TEXT[MQTT_COUNT++] = '=';
      if (commend_value) {
        for (int i = 0; i < strlen(value); i++) {
          MQTT_TEXT[MQTT_COUNT++] = value[i];
        }
      } else {
        MQTT_TEXT[MQTT_COUNT++] = '0';
      }
      Serial.print("MQTT_TEXT: ");
      Serial.println(MQTT_TEXT);
      mqttClient.publish(topic_pub, MQTT_TEXT);
    }
  }
  if (strCommend) {
    Serial.print("commend: ");
    Serial.print(strCommend);
    Serial.print("=");
    Serial.println(value);
  }
  //// ------------ Monitering ------------

  if (strCommend == "AT+start") {
    send2Nextion("vaSet.val=1");
    Display("btON", plasmaRun);
    Display("btOFF", !plasmaRun);
    Display("timeCount", TimeCount);
    Display("timeOn", onTime);
  } else if (strCommend == "AT+WIFI") {
    wifiOnOff = commend_value;
    if (wifiOnOff) {
      wifiConnect();
    } else {
      if (wifiSet) {
        EEPROM.write(WIFI, false);
      }
    }
  }

  else if (strCommend == "AT+ON") {
    plasmaRun = true;
    PlasmaOnOff = plasmaRun;
    Display("btON", PlasmaOnOff);
    Display("btOFF", !PlasmaOnOff);
    send2Nextion("onoff.val=0");
  } else if (strCommend == "AT+OFF") {
    plasmaRun = false;
    PlasmaOnOff = plasmaRun;
    Display("btON", PlasmaOnOff);
    Display("btOFF", !PlasmaOnOff);
    send2Nextion("onoff.val=0");
  } else if (strCommend == "AT+TC") {
    TimeCount = commend_value;
    EEPROM.write(T_C, commend_value);
    Display("timeCount", TimeCount);
    PlasmaOn = true;
  } else if (strCommend == "AT+TO") {
    onTime = commend_value;
    offTime = (TimeCount * 60) - onTime;
    EEPROM.write(T_ON, commend_value / 5);
    Display("timeOn", onTime);
    PlasmaOff = true;
    send2Nextion("btConfirm.val=0");
  }

  else if (strCommend == "AT+RO") {
    if (commend_value == 1) {
      digitalWrite(Relay_A, true);
    } else if (commend_value == 2) {
      digitalWrite(Relay_B, true);
    } else if (commend_value == 3) {
      digitalWrite(Relay_C, true);
    } else {
      digitalWrite(Relay_A, true);
      digitalWrite(Relay_B, true);
      digitalWrite(Relay_C, true);
    }
  } else if (strCommend == "AT+RC") {
    if (commend_value == 1) {
      digitalWrite(Relay_A, false);
    } else if (commend_value == 2) {
      digitalWrite(Relay_B, false);
    } else if (commend_value == 3) {
      digitalWrite(Relay_C, false);
    } else {
      digitalWrite(Relay_A, false);
      digitalWrite(Relay_B, false);
      digitalWrite(Relay_C, false);
    }
  }

  else if (strCommend == "AT+MAC") {
    char sendID[21] = "ID=";
    for (int i = 0; i < 17; i++) {
      sendID[i + 3] = WiFi.macAddress()[i];
    }
    char* sub_ID = sendID;
    String LCD_MAC = "passTXT.txt=\"";
    LCD_MAC += sub_ID;
    LCD_MAC += "\"";
    send2Nextion(LCD_MAC);
  }

  else if (strCommend == "AT+MAIN") {
    pageMain();
  }

  //// ------------ from Online ------------
  else if (strCommend == "AT+Refresh") {
    pageMain();
  }
  EEPROM.commit();
}  //// ------------ End Of ATCode ------------

void Serial_process() {
  char ch;
  ch = nxSerial.read();
  switch (ch) {
    case ';':
      Serial_buf[Serial_num] = 0x00;
      Serial_service();
      Serial_num = 0;
      break;
    default:
      Serial_buf[Serial_num++] = ch;
      Serial_num %= SERIAL_MAX;
      break;
  }
}

void Serial_service() {
  char* StrCommend = strtok(Serial_buf, "=");
  char* StrValue = strtok(0x00, " ");
  ATCode(StrCommend, StrValue);
}  //Serial_service

//// ------------ MQTT Callback ------------
void callback(char* topic, byte* payload, unsigned int length) {
  char mqtt_buf[SERIAL_MAX] = "";
  for (int i = 0; i < length; i++) {
    mqtt_buf[i] = payload[i];
  }
  char* StrCommend = strtok(mqtt_buf, "=");
  char* StrValue = strtok(0x00, " ");
  ATCode(StrCommend, StrValue);
}
//// ------------ End Of MQTT Callback ------------

//// ------------ wifiConnect ------------
void wifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  wm.setConfigPortalTimeout(120);
  bool res;
  res = wm.autoConnect("SmartHivePlasma");
  if (!res) {
    Serial.println("Failed to connect");
    if (wifiSet) {
      wm.resetSettings();
      EEPROM.write(WIFI, false);
      EEPROM.commit();
    }
    return;
  }
  Serial.println("WiFi Connected");
  wifiOnOff = true;

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  unsigned long WIFI_TXT = 0UL;

  char sendID[20] = "ID=";
  for (int i = 0; i < 17; i++) {
    sendID[i + 3] = WiFi.macAddress()[i];
    deviceID[i] = sendID[i + 3];
  }

  while (!mqttClient.connected()) {
    DisplayConnect();
    if (millis() > WIFI_TXT + 1000) {
      WIFI_TXT = millis();
      if (!wifiOnOff) return;
      if (mqttClient.connect(deviceID, mqttUser, mqttPassword)) {
        Serial.println("connected");
      } else {
        Serial.print("failed with state ");
        Serial.print(mqttClient.state());
      }
    }
  }

  char* topic_sub = deviceID;
  char* sub_ID = sendID;
  //// ------------ MQTT Communicate ------------
  mqttClient.subscribe(topic_sub);
  mqttClient.subscribe("serverTime");
  mqttClient.publish(topic_pub, sub_ID);
  if (!wifiSet) {
    EEPROM.write(WIFI, true);
    EEPROM.commit();
  }
  send2Nextion("WIFI.val=1");
  Serial.println("MQTT Connected");
}  //// ------------ End Of wifiConnect() ------------


//// ------------ setup ------------
void setup() {
  prevUpdatePlasma = millis();
  Serial.begin(115200);

  if (!EEPROM.begin(EEPROM_SIZE)) Serial.println("failed to initialise EEPROM");

  //// ------------ Pin Out ------------
  pinMode(Relay_PA, OUTPUT);
  pinMode(Relay_PB, OUTPUT);
  pinMode(Relay_A, OUTPUT);
  pinMode(Relay_B, OUTPUT);
  pinMode(Relay_C, OUTPUT);

  //// ------------ EEPROM ------------
  wifiSet = byte(EEPROM.read(WIFI));
  TimeCount = byte(EEPROM.read(T_C));
  onTime = byte(EEPROM.read(T_ON)) * 5;
  offTime = (TimeCount * 60) - onTime;

  PlasmaOn = onTime * 60;
  PlasmaOff = offTime * 60;

  nxSerial.begin(115200, SERIAL_8N1, 18, 19);
  Serial.println("System Boot");
  SystemBoot();
}  //// ------------ End Of Setup() ------------

//// ------------ loop ------------
void loop() {
  DisplayConnect();
  if (wifiOnOff) mqttClient.loop();
  PlasmaSchedule(millis());
}  //// ------------ End Of loop() ------------

//// ------------ Plasma ------------
void PlasmaSchedule(unsigned long currentTime) {
  if (currentTime > prevUpdatePlasma + 1000) {
    prevUpdatePlasma = currentTime;
    if (plasmaRun) {
      //// ------------ Plasma On ------------
      if (PlasmaOnOff) {
        if (PlasmaOn < 1) {
          PlasmaOnOff = false;
          PlasmaOn = onTime * 60;
        }
        PlasmaOn--;
        PlasmaRelay(PlasmaOnOff);
      }
      //// ------------ Plasma Off ------------
      else {
        if (PlasmaOff < 1) {
          PlasmaOnOff = true;
          PlasmaOff = offTime * 60;
        }
        PlasmaOff--;
        PlasmaRelay(PlasmaOnOff);
      }
      Serial.print("state:");
      Serial.print(PlasmaOnOff);
      Serial.print(", on:");
      Serial.print(PlasmaOn);
      Serial.print(", off:");
      Serial.println(PlasmaOff);
    } else {
      PlasmaOnOff = true;
      PlasmaOn = onTime * 60;
      PlasmaOff = offTime * 60;
      PlasmaRelay(!PlasmaOnOff);
    }
  }  //millis()
}  //plasma

//// ------------ Plasma Relay ------------
void PlasmaRelay(boolean onoffPlasmaRelay) {
  digitalWrite(Relay_PA, onoffPlasmaRelay);
  digitalWrite(Relay_PB, onoffPlasmaRelay);
}
//// ------------ Display ------------
//// ------------ Initializing ------------
void SystemBoot() {
  Display("WIFI", wifiSet);
  if (wifiSet) wifiConnect();  //WIFI
  Serial.println("System all green");
  pageMain();
}  //// ------------ End Of Initializing ------------

void pageMain() {
  send2Nextion("page 0");
  Display("WIFI", wifiSet);
  Display("timeCount", TimeCount);
  Display("timeOn", onTime);
}

void DisplayConnect() {
  if (nxSerial.available()) {
    Serial_process();
  }
}

void Display(String IDs, uint16_t values) {
  String cmd;
  char buf[8] = { 0 };
  sprintf(buf, "%d", values);
  cmd = IDs + ".val=";
  cmd += buf;
  send2Nextion(cmd);
}

void send2Nextion(String cmd) {
  nxSerial.print(cmd);
  nxSerial.write(0xFF);
  nxSerial.write(0xFF);
  nxSerial.write(0xFF);
}
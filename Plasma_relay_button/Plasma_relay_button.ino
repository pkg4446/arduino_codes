 #include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient
#include <SoftwareSerial.h>//https://github.com/plerup/espsoftwareserial/
#include <HTTPClient.h>
#include "EEPROM.h"
#define  EEPROM_SIZE 28
#define  SERIAL_MAX  32
HardwareSerial nxSerial(2);
SoftwareSerial ZE03serial;
uint8_t ZE03[9] = {0,};
uint8_t ZE03Index = 0;

const String masterKey = "76047982";

unsigned long prevUpdateDoor    = 0UL;
unsigned long prevUpdateDoorST  = 0UL;
unsigned long prevUpdatePlasma  = 0UL;
unsigned long prevUpdateSensorV = 0UL;
unsigned long prevUpdateSensor  = 0UL;
unsigned long prevUpdateActor   = 0UL;
unsigned long prevUpdateTime    = 0UL;

//const int8_t Relay[6]  = {5, 17, 16, 4, 2, 15};
//const int8_t Button[4] = {13, 12, 14, 27};
//const int8_t ADC[5]    = {33, 32, 34, 39, 35};
//const int8_t DAC[2]    = {25, 26};

//// ------------ Pin out D ------------
const int8_t  Relay_Door  = 5;
const int8_t  Relay_PA    = 17;
const int8_t  Relay_PB    = 16;
const int8_t  Relay_F     = 2;
//// ------------ Pin out B ------------
const int8_t  DoorPin  = 27;
const int8_t  s_Water  = 14;
//// ------------ Pin out A ------------
const int8_t  s_AmpA   = 33;
const int8_t  s_AmpB   = 32;

//// ------------ MQTT ------------
const char*   mqttServer   = "smarthive.kr";
const int     mqttPort     = 1883;
const char*   mqttUser     = "hive";
const char*   mqttPassword = "hive";
const char*   topic_pub    = "WAREHOSE";
char          deviceID[18];

//// ------------ EEPROM ------------
const uint8_t WIFI    = 1;
const uint8_t T_C     = 10;
const uint8_t T_ON    = 11;
const uint8_t PW[8]   = {20, 21, 22, 23, 24, 25, 26, 27};

//// ------------ Value ------------
uint8_t   TimeCount , onTime;
uint16_t  offTime;
uint16_t  ppm_O3 = 0;

//// ------------ Flage ------------
boolean wifiSet     = false;
boolean wifiOnOff   = false;

boolean doorPost    = false;

boolean O3sensor    = false;

boolean plasmaRun   = false;
boolean PlasmaOnOff = false;

boolean FanOnOff    = false;
boolean FanCommend  = false;

boolean f_AmpA      = true;
boolean f_AmpB      = false;

//// ------------ Timer ------------
uint8_t HOUR        = 0;
uint8_t MIN         = 0;
uint8_t SEC         = 0;

uint16_t DoorCount   = 0;
uint32_t PlasmaOn    = true;
uint32_t PlasmaOff   = true;

WiFiClient    espClient;
PubSubClient  mqttClient(espClient);

////AT Code for chage the Something//////////////////////////
char    Serial_buf[SERIAL_MAX];
int16_t Serial_num;
//// ------------ ATCode ------------
void ATCode(char* commend, char* value) {
  String strCommend = commend;
  String intValue   = value;
  uint8_t commend_value = 0;
  if (strCommend != "AT+pw" || strCommend != "AT+pwM" || strCommend != "AT+pwC") commend_value = intValue.toInt();
  //// ------------ Monitering ------------
  if (wifiOnOff && commend) {
    if (strCommend == "AT+ON"  || strCommend == "AT+OFF"  ||
        strCommend == "AT+TC"  || strCommend == "AT+TO"   ||
        strCommend == "AT+FAN") {
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
    Display("btON",   plasmaRun);
    Display("btOFF", !plasmaRun);
    Display("timeCount",  TimeCount);
    Display("timeOn",     onTime);
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

  else if (strCommend == "AT+pw") {
    if (passCheck(value) || masterKey == intValue) {
      plasmaRun = false;
      digitalWrite(Relay_PA, plasmaRun);
      digitalWrite(Relay_PB, plasmaRun);
      send2Nextion("passCompere.val=5");
      doorDelay();
    } else {
      send2Nextion("passCompere.val=2");
      send2Nextion("pass.txt=\"\"");
    }
  } else if (strCommend == "AT+pwM") {
    if (passCheck(value) || masterKey == intValue) {
      send2Nextion("passCompere.val=3");
      send2Nextion("passMode.val=1");
    } else {
      send2Nextion("passCompere.val=2");
    }
    send2Nextion("pass.txt=\"\"");
  } else if (strCommend == "AT+pwC") {
    for (int i = 0; i < 8; i++) {
      char chackPW;
      if (i < strlen(value)) {
        chackPW = value[i];
      } else {
        chackPW = '0';
      }
      EEPROM.write(PW[i], chackPW);
    }
    EEPROM.commit();
    send2Nextion("pass.txt=\"\"");
    send2Nextion("passMode.val=0");
    send2Nextion("passCompere.val=4");
  }

  else if (strCommend == "AT+ON") {
    plasmaRun   = true;
    PlasmaOnOff = plasmaRun;
    Display("btON",  PlasmaOnOff);
    Display("btOFF", !PlasmaOnOff);
    send2Nextion("onoff.val=0");
  } else if (strCommend == "AT+OFF") {
    plasmaRun   = false;
    PlasmaOnOff = plasmaRun;
    Display("btON",  PlasmaOnOff);
    Display("btOFF", !PlasmaOnOff);
    send2Nextion("onoff.val=0");
  } else if (strCommend == "AT+FAN") {
    FanOnOff = commend_value;
    FanCommend = commend_value;
    Display("btFAN", FanOnOff);
  }
  else if (strCommend == "AT+TC") {
    TimeCount = commend_value;
    EEPROM.write(T_C, commend_value);
    Display("timeCount", TimeCount);
    PlasmaOn = true;
  } else if (strCommend == "AT+TO") {
    onTime  = commend_value;
    offTime = (TimeCount * 60) - onTime;
    EEPROM.write(T_ON, commend_value / 5);
    Display("timeOn", onTime);
    PlasmaOff = true;
    send2Nextion("btConfirm.val=0");
  }

  else if (strCommend == "AT+MAC") {
    char sendID[21]   = "ID=";
    for (int i = 0; i < 17; i++) {
      sendID[i + 3] = WiFi.macAddress()[i];
    }
    char* sub_ID    = sendID;
    String LCD_MAC  = "passTXT.txt=\"";
    LCD_MAC += sub_ID;
    LCD_MAC += "\"";
    send2Nextion(LCD_MAC);
  }

  else if (strCommend == "AT+MAIN") {
    pageMain();
  }

  //// ------------ from Online ------------
  else if (strCommend == "AT+Door") {
    plasmaRun = false;
    send2Nextion("page 1");
    digitalWrite(Relay_PA, plasmaRun);
    digitalWrite(Relay_PB, plasmaRun);
    send2Nextion("passCompere.val=5");
    doorDelay();
  } else if (strCommend == "AT+Refresh") {
    pageMain();
  }
  EEPROM.commit();
}//// ------------ End Of ATCode ------------

void Serial_process() {
  char ch;
  ch = nxSerial.read();
  switch ( ch ) {
    case ';':
      Serial_buf[Serial_num] = NULL;
      Serial_service();
      Serial_num = 0;
      break;
    default :
      Serial_buf[ Serial_num ++ ] = ch;
      Serial_num %= SERIAL_MAX;
      break;
  }
}

void Serial_service() {
  char* StrCommend = strtok(Serial_buf, "=");
  char* StrValue   = strtok(NULL, " ");
  ATCode(StrCommend, StrValue);
}//Serial_service

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

  unsigned long WIFI_TXT  = 0UL;

  char sendID[20]   = "ID=";
  for (int i = 0; i < 17; i++) {
    sendID[i + 3] = WiFi.macAddress()[i];
    deviceID[i]   = sendID[i + 3];
  }
  
  while (!mqttClient.connected()) {
    DisplayConnect();
    if (millis() > WIFI_TXT + 1000) {
      WIFI_TXT = millis();
      if (!wifiOnOff) return;
      if (mqttClient.connect(deviceID, mqttUser, mqttPassword )) {
        Serial.println("connected");
      } else {
        Serial.print("failed with state ");
        Serial.print(mqttClient.state());
      }
    }
  }
  
  char* topic_sub = deviceID;
  char* sub_ID    = sendID;
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
}//// ------------ End Of wifiConnect() ------------


//// ------------ setup ------------
void setup() {
  prevUpdatePlasma  = millis();
  Serial.begin(115200);
  ZE03serial.begin(9600, SWSERIAL_8N1, 23, -1, false, 256);

  if (!EEPROM.begin(EEPROM_SIZE)) Serial.println("failed to initialise EEPROM");

  //// ------------ Pin Out ------------
  pinMode(Relay_Door, OUTPUT);
  pinMode(Relay_PA, OUTPUT);
  pinMode(Relay_PB, OUTPUT);
  pinMode(Relay_F, OUTPUT);
  pinMode(DoorPin, INPUT_PULLUP);

  //// ------------ EEPROM ------------
  wifiSet     = byte(EEPROM.read(WIFI));
  TimeCount   = byte(EEPROM.read(T_C));
  onTime      = byte(EEPROM.read(T_ON)) * 5;
  offTime = (TimeCount * 60) - onTime;

  PlasmaOn = onTime * 60;
  PlasmaOff = offTime * 60;

  nxSerial.begin(115200, SERIAL_8N1, 18, 19);
  Serial.println("System Boot");
  SystemBoot();
}//// ------------ End Of Setup() ------------

//// ------------ loop ------------
void loop() {
  DisplayConnect();
  if (wifiOnOff)  mqttClient.loop();
  PlasmaSchedule(millis());
  doorState(millis());
  doorOpen(millis());
  Sensor_O3(millis());
  Actor (millis());
}//// ------------ End Of loop() ------------


//// ------------ passCheck ------------
boolean passCheck(char* password) {
  boolean returnCheck = true;
  for (int i = 0; i < 8; i++) {
    const char checkEEPROM = byte(EEPROM.read(PW[i]));
    char chackPW;
    if (password) {
      if (i < strlen(password)) {
        chackPW = password[i];
      } else {
        chackPW = '0';
      }
    } else {
      chackPW = '0';
    }
    if (chackPW != checkEEPROM) returnCheck = false;
  }
  return returnCheck;
}//// ------------ End Of passCheck ------------

//// ------------ doorOpen ------------
void doorDelay() {
  FanOnOff = true;
  digitalWrite(Relay_F, true);
  ////오존 농도에 따라서 팬 작동 시간.
  if (O3sensor) {
    DoorCount = 3 + ppm_O3 * 1;
  } else {
    DoorCount = 30;
  }
}//// ------------ End Of doorOpen ------------

//// ------------ doorOpen ------------
void doorState(unsigned long currentTime) {
  if (currentTime > prevUpdateDoorST + 1000) {
    prevUpdateDoorST = currentTime;  
    if (doorPost && !digitalRead(DoorPin)) {
      Serial.println("Door Close");
      doorPost = false;
      httpPOSTRequest("DOOR", "CLOSE");
    } else if (!doorPost && digitalRead(DoorPin)) {      
      Serial.println("Door Open");
      doorPost = true;
      httpPOSTRequest("DOOR", "OPEN");
    }
  }
}//// ------------ End Of doorOpen ------------

//// ------------ doorOpen ------------
void doorOpen(unsigned long currentTime) {
  if (currentTime > prevUpdateDoor + 1000) {
    prevUpdateDoor = currentTime;
    if (DoorCount == 2) {
      if (!FanCommend)FanOnOff = false;
      digitalWrite(Relay_F, FanOnOff);
      digitalWrite(Relay_Door, true);
      pageMain();
    } else if (DoorCount == 1) {
      digitalWrite(Relay_Door, false);
    }
    if (DoorCount > 0) {
      String DC_Send = "DC.txt=\"";
      DC_Send += String(DoorCount - 2);
      DC_Send += "\"";
      send2Nextion(DC_Send);
      DoorCount--;
    }
  }
}//// ------------ End Of doorOpen ------------
boolean rfFan = false;
//// ------------ Actor ------------
void Actor(unsigned long currentTime) {
  if (currentTime > prevUpdateActor + 500) {
    prevUpdateActor = currentTime;
    if (plasmaRun == true) {
      if (digitalRead(DoorPin)) {
        PlasmaOnOff = false;
        Display("btON",  PlasmaOnOff);
        Display("btOFF", !PlasmaOnOff);
      }
    }
    if (rfFan != FanOnOff) {
      rfFan = FanOnOff;
      digitalWrite(Relay_F, FanOnOff);
    }
  }//millis()
}//Actor

//// ------------ Sensor_O3 ------------
void Sensor_O3(unsigned long currentTime) {
  if (ZE03serial.available()) {
    if (!O3sensor) {
      O3sensor = true;
    }
    else {
      O3sensor = false;
    }    
    if (currentTime > prevUpdateSensorV + 300) {
      uint8_t reciveSensor = ZE03serial.read();
      prevUpdateSensorV = currentTime;
      if (reciveSensor == 255) {
        ZE03Index = 0;
        ZE03[ZE03Index++] = reciveSensor;
      } else if (ZE03Index >= 9) {
        ZE03Index = 0;        
        Serial.println("ZE03_ERR");
        send2Nextion("O3.val=505");
      } else {
        ZE03[ZE03Index++] = reciveSensor;
        if (ZE03Index == 8) {
          Serial.print("ROWDATA: ");
          for (uint8_t Index = 0; Index < 9; Index++) {
            Serial.print(ZE03[Index], HEX);
            Serial.print(",");
          }          
          Serial.print(" O3_PPM: ");
          ppm_O3 = ZE03[2] * 256 + ZE03[3];
          Serial.println(ppm_O3);
          Display("O3",ppm_O3);
        }
      }
    }
    if (currentTime > prevUpdateSensor + 1000 * 60 * 5) {
      prevUpdateSensor = currentTime;
      httpPOSTRequest("O3", String(ppm_O3));
    }
  } else {
    if (currentTime > prevUpdateSensor + 1000 * 60 * 6) {
      prevUpdateSensor = currentTime;
      Serial.print(" O3_err: ");
      Serial.println(ppm_O3);
      httpPOSTRequest("O3", String(ppm_O3));
      O3sensor = false;
    }
  }

}//Sensor_O3()

boolean plasmaPost = false;
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
          PlasmaOff   = offTime * 60;
        }
        PlasmaOff--;
        PlasmaRelay(PlasmaOnOff);
      }
      if (plasmaPost != PlasmaOnOff) {
        plasmaPost = PlasmaOnOff;
        httpPOSTRequest("PLZ", String(PlasmaOnOff));
      }
    } else {
      if (plasmaPost != false) {
        plasmaPost = false;
        httpPOSTRequest("PLZ", String(false));
      }
      PlasmaOnOff = true;
      PlasmaOn = onTime * 60;
      PlasmaOff = offTime * 60;
      PlasmaRelay(!PlasmaOnOff);
    }
  }//millis()
}//plasma

//// ------------ Plasma Relay ------------
void PlasmaRelay(boolean onoffPlasmaRelay) {
  if (f_AmpA) {
    digitalWrite(Relay_PA, onoffPlasmaRelay);
  } else {
    digitalWrite(Relay_PA, false);
  }
  if (f_AmpB) {
    digitalWrite(Relay_PB, onoffPlasmaRelay);
  } else {
    digitalWrite(Relay_PB, false);
  }
}

//// ------------ Display ------------
//// ------------ Initializing ------------
void SystemBoot() {
  Display("WIFI", wifiSet);
  if (wifiSet)  wifiConnect();//WIFI
  Serial.println("System all green");
  pageMain();
}//// ------------ End Of Initializing ------------

void pageMain() {
  send2Nextion("page 0");
  Display("WIFI",       wifiSet);
  Display("timeCount",  TimeCount);
  Display("timeOn",     onTime);
}

void DisplayConnect() {
  if (nxSerial.available()) {
    Serial_process();
  }
}

void Display(String IDs, uint16_t values) {
  String cmd;
  char buf[8] = {0};
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

void httpPOSTRequest(String TYPE, String DATA) {
  if (wifiOnOff) {
    String serverUrl = "http://smarthive.kro.kr/reg/save/"; //API adress
    HTTPClient http;
    WiFiClient http_client;
    http.begin(http_client, serverUrl);

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String httpRequestData = (String)"MODULE="  + deviceID    +
                             "&TYPE="         + TYPE +
                             "&DATA="         + DATA;

    int httpResponseCode = http.POST(httpRequestData);
    Serial.print(httpRequestData);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    http.end();           // Free resources
  }
}////httpPOSTRequest_End

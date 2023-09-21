#include  <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
#include  <PubSubClient.h> //https://github.com/knolleary/pubsubclient
#include  "EEPROM.h"
#define   EEPROM_SIZE 28
#define   SERIAL_MAX  32
HardwareSerial nxSerial(2);

const String masterKey = "76047982";

unsigned long prevUpdateDoor    = 0UL;
unsigned long prevUpdatePlasma  = 0UL;
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
const int8_t  Relay_M     = 4;
const int8_t  Relay_F     = 2;
const int8_t  Relay_H     = 15;
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
const char*   topic_pub    = "server";
char          deviceID[18];

//// ------------ EEPROM ------------
const uint8_t WIFI    = 1;
const uint8_t A_S_H   = 2;
const uint8_t A_S_M   = 3;
const uint8_t A_E_H   = 4;
const uint8_t A_E_M   = 5;
const uint8_t P_S_H   = 6;
const uint8_t P_S_M   = 7;
const uint8_t P_E_H   = 8;
const uint8_t P_E_M   = 9;
const uint8_t T_ON    = 10;
const uint8_t T_OFF   = 11;
const uint8_t P_D     = 12;
const uint8_t P_T     = 13;
const uint8_t S_CO2   = 14;
const uint8_t S_NH3   = 15;
const uint8_t S_H2S   = 16;
const uint8_t PW[8]   = {20, 21, 22, 23, 24, 25, 26, 27};

//// ------------ Value ------------
uint8_t AM_S_H, AM_S_M, AM_E_H, AM_E_M;
uint8_t PM_S_H, PM_S_M, PM_E_H, PM_E_M;
uint8_t onTime, offTime, pumpDelay, pumpTime ;
uint8_t CO2, NH3, H2S;

//// ------------ Flage ------------
boolean wifiSet     = false;
boolean wifiOnOff   = false;

boolean able        = false;
boolean plasmaRun   = true;
boolean PlasmaOnOff = false;

boolean FanOnOff    = false;
boolean PumpOnOff   = false;
boolean HeaterOnOff = false;

boolean f_AmpA      = false;
boolean f_AmpB      = false;
boolean f_water     = false;

//// ------------ Timer ------------
uint8_t HOUR        = 0;
uint8_t MIN         = 0;
uint8_t SEC         = 0;

uint16_t DoorCount   = 0;
uint16_t pumpCountD  = 0;
uint16_t PumpCount   = 0;
uint16_t PlasmaCount = 0;
uint16_t PlasmaOn    = true;
uint16_t PlasmaOff   = true;

//// ------------ Display lamp ------------
boolean lampDay     = false;
boolean lampNight   = false;

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
    if (strCommend == "AT+RUN"      || strCommend == "AT+FAN" ||
        strCommend == "AT+HEAT"     ||
        strCommend == "AT+sTshD"    || strCommend == "AT+sTsmD" ||
        strCommend == "AT+sTehD"    || strCommend == "AT+sTemD" ||
        strCommend == "AT+sTshN"    || strCommend == "AT+sTsmN" ||
        strCommend == "AT+sTehN"    || strCommend == "AT+sTemN" ||
        strCommend == "AT+sTon"     || strCommend == "AT+sToff" ||
        strCommend == "AT+sTPD"     || strCommend == "AT+sTPR"  ||
        strCommend == "AT+CO2"      || strCommend == "AT+NH3"   ||
        strCommend == "AT+H2S")
    {
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
    send2Nextion("boot.val=1");
    send2Nextion("run.val=0");
    send2Nextion("btON.val=1");
    send2Nextion("btOFF.val=0");
    send2Nextion("btF.val=0");
    send2Nextion("btP.val=0");
    send2Nextion("btH.val=0");
    send2Nextion("Timer.val=1");
    send2Nextion("lampF.val=0");
    send2Nextion("lampM.val=0");
    send2Nextion("lampH.val=0");
    SystemBoot();
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

  else if (strCommend == "AT+RUN") {
    plasmaRun   = commend_value;
    PlasmaOnOff = plasmaRun;
    if (PlasmaOnOff) {
      PumpOnOff   = false;
      send2Nextion("btP.val=0");
    }
    if (!PlasmaOnOff) {
      send2Nextion("lampMD.val=0");
      send2Nextion("lampMO.val=0");
      send2Nextion("lampM.val=0");
      digitalWrite(Relay_M, false);
    }
    Display("btON", plasmaRun);
    Display("btOFF", !plasmaRun);
    send2Nextion("run.val=0");
  } else if (strCommend == "AT+PUMP") {
    PumpOnOff = commend_value;
    Display("btP", PumpOnOff);
    PumpCount = pumpTime * 2;
    send2Nextion("run.val=0");
  } else if (strCommend == "AT+FAN") {
    FanOnOff = commend_value;
    Display("btF", FanOnOff);
    send2Nextion("run.val=0");
  } else if (strCommend == "AT+HEAT") {
    HeaterOnOff = commend_value;
    Display("btH", HeaterOnOff);
    send2Nextion("run.val=0");
  }

  else if (strCommend == "AT+Hour") {
    HOUR  = commend_value;
  } else if (strCommend == "AT+Min") {
    MIN   = commend_value;
  } else if (strCommend == "AT+Sec") {
    SEC   = commend_value;
  }

  else if (strCommend == "AT+sTshD") {
    AM_S_H = commend_value;
    EEPROM.write(A_S_H, commend_value);
  } else if (strCommend == "AT+sTsmD") {
    AM_S_M = commend_value;
    EEPROM.write(A_S_M, commend_value);
  } else if (strCommend == "AT+sTehD") {
    AM_E_H = commend_value;
    EEPROM.write(A_E_H, commend_value);
  } else if (strCommend == "AT+sTemD") {
    AM_E_M = commend_value;
    EEPROM.write(A_E_M, commend_value);
  }
  else if (strCommend == "AT+sTshN") {
    PM_S_H = commend_value;
    EEPROM.write(P_S_H, commend_value);
  } else if (strCommend == "AT+sTsmN") {
    PM_S_M = commend_value;
    EEPROM.write(P_S_M, commend_value);
  } else if (strCommend == "AT+sTehN") {
    PM_E_H = commend_value;
    EEPROM.write(P_E_H, commend_value);
  } else if (strCommend == "AT+sTemN") {
    PM_E_M = commend_value;
    EEPROM.write(P_E_M, commend_value);
  }
  else if (strCommend == "AT+sTon") {
    onTime = commend_value;
    EEPROM.write(T_ON, commend_value);
    PlasmaOn = true;
  } else if (strCommend == "AT+sToff") {
    offTime = commend_value;
    EEPROM.write(T_OFF, commend_value);
    PlasmaOff = true;
  } else if (strCommend == "AT+sTPD") {
    pumpDelay = commend_value;
    EEPROM.write(P_D, commend_value);
  } else if (strCommend == "AT+sTPR") {
    pumpTime = commend_value;
    EEPROM.write(P_T, commend_value);
  }
  else if (strCommend == "AT+CO2") {
    CO2 = commend_value;
    EEPROM.write(S_CO2, commend_value);
  } else if (strCommend == "AT+NH3") {
    NH3 = commend_value;
    EEPROM.write(S_NH3, commend_value);
  } else if (strCommend == "AT+H2S") {
    H2S = commend_value;
    EEPROM.write(S_H2S, commend_value);
    send2Nextion("btSave.val=0");
    send2Nextion("page 0");
    pageMain();
  }

  //// ------------ from Online ------------
  else if (strCommend == "AT+Door") {
    plasmaRun = false;
    send2Nextion("page 3");
    digitalWrite(Relay_PA, plasmaRun);
    digitalWrite(Relay_PB, plasmaRun);
    send2Nextion("passCompere.val=5");
    doorDelay();
  } else if (strCommend == "AT+Refresh") {
    pageMain();
    pageConfig();
  } else if (strCommend == "AT+ClockY") {
    send2Nextion("rtc0=" + intValue + "\0");
  } else if (strCommend == "AT+ClockN") {
    send2Nextion("rtc1=" + intValue + "\0");
  } else if (strCommend == "AT+ClockD") {
    send2Nextion("rtc2=" + intValue + "\0");
  } else if (strCommend == "AT+ClockH") {
    send2Nextion("rtc3=" + intValue + "\0");
  } else if (strCommend == "AT+ClockM") {
    send2Nextion("rtc4=" + intValue + "\0");
  } else if (strCommend == "AT+ClockS") {
    send2Nextion("rtc5=" + intValue + "\0");
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
  //// ------------ LCD ------------
  send2Nextion("MAC.txt=\"Try to connect WIFI\"");
  //// ------------ LCD ------------
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
    //// ------------ LCD ------------
    send2Nextion("MAC.txt=\"Failed to connect WIFI\"");
    send2Nextion("btWIFI.val=0");
    //// ------------ LCD ------------
    return;
  }
  Serial.println("WiFi Connected");
  wifiOnOff = true;

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  //// ------------ LCD ------------
  send2Nextion("MAC.txt=\"Try to connect MQTT\"");
  //// ------------ LCD ------------
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
  //// ------------ LCD ------------
  String LCD_MAC  = "MAC.txt=\"";
  LCD_MAC += sub_ID;
  LCD_MAC += "\"";
  send2Nextion(LCD_MAC);
  //// ------------ MQTT Communicate ------------
  mqttClient.subscribe(topic_sub);
  mqttClient.subscribe("serverTime");
  mqttClient.publish(topic_pub, sub_ID);
  if (!wifiSet) {
    EEPROM.write(WIFI, true);
    EEPROM.commit();
  }
  Serial.println("MQTT Connected");
}//// ------------ End Of wifiConnect() ------------


//// ------------ setup ------------
void setup() {
  prevUpdatePlasma  = millis();
  Serial.begin(115200);

  if (!EEPROM.begin(EEPROM_SIZE)) Serial.println("failed to initialise EEPROM");

  //// ------------ Pin Out ------------
  pinMode(Relay_Door, OUTPUT);
  pinMode(Relay_PA, OUTPUT);
  pinMode(Relay_PB, OUTPUT);
  pinMode(Relay_M, OUTPUT);
  pinMode(Relay_F, OUTPUT);
  pinMode(Relay_H, OUTPUT);
  pinMode(DoorPin, INPUT_PULLUP);

  //// ------------ EEPROM ------------
  wifiSet     = byte(EEPROM.read(WIFI));
  AM_S_H      = byte(EEPROM.read(A_S_H));
  AM_S_M      = byte(EEPROM.read(A_S_M));
  AM_E_H      = byte(EEPROM.read(A_E_H));
  AM_E_M      = byte(EEPROM.read(A_E_M));
  PM_S_H      = byte(EEPROM.read(P_S_H));
  PM_S_M      = byte(EEPROM.read(P_S_M));
  PM_E_H      = byte(EEPROM.read(P_E_H));
  PM_E_M      = byte(EEPROM.read(P_E_M));
  onTime      = byte(EEPROM.read(T_ON));
  offTime     = byte(EEPROM.read(T_OFF));
  pumpDelay   = byte(EEPROM.read(P_D));
  pumpTime    = byte(EEPROM.read(P_T));
  CO2         = byte(EEPROM.read(S_CO2));
  NH3         = byte(EEPROM.read(S_NH3));
  H2S         = byte(EEPROM.read(S_H2S));

  PlasmaOn = onTime * 60;
  PlasmaOff = offTime * 60;
  pumpCountD  = pumpDelay * 60;
  PumpCount   = pumpTime;

  nxSerial.begin(115200, SERIAL_8N1, 18, 19);
  send2Nextion("page 0");
  send2Nextion("boot.val=0");

  Serial.println("System Boot");
}//// ------------ End Of Setup() ------------

//// ------------ loop ------------
void loop() {
  DisplayConnect();
  if (wifiOnOff)  mqttClient.loop();
  if (f_AmpA || f_AmpB) {
    PlasmaSchedule(millis());
  }
  doorOpen(millis());
  Actor (millis());
  Sensor(millis());
  timer (millis());
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
  digitalWrite(Relay_F, true);
  ////오존 농도에 따라서 팬 작동 시간.
  DoorCount = 10;
}//// ------------ End Of doorOpen ------------

//// ------------ doorOpen ------------
void doorOpen(unsigned long currentTime) {
  if (currentTime > prevUpdateDoor + 1000) {
    prevUpdateDoor = currentTime;
    if (DoorCount == 2) {
      digitalWrite(Relay_F, false);
      digitalWrite(Relay_Door, true);
      send2Nextion("page 0");
      send2Nextion("btON.val=0");
      send2Nextion("btOFF.val=1");
      PlasmaLamp(true, false);
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

//// ------------ Actor ------------
boolean rfPump = false;
boolean rfFan  = false;
boolean rfHeat = false;
void Actor(unsigned long currentTime) {
  if (currentTime > prevUpdateActor + 500) {

    prevUpdateActor = currentTime;

    if (digitalRead(DoorPin)) {
      PlasmaOnOff = false;
      send2Nextion("btON.val=0");
      send2Nextion("btOFF.val=1");
    }

    if (rfPump != PumpOnOff) {
      rfPump = PumpOnOff;
      digitalWrite(Relay_M, PumpOnOff);
      Display("lampM", PumpOnOff);
      Display("btP", PumpOnOff);
    }
    if (rfFan != FanOnOff) {
      rfFan = FanOnOff;
      digitalWrite(Relay_F, FanOnOff);
      Display("lampF", FanOnOff);
    }
    if (rfHeat != HeaterOnOff) {
      rfHeat = HeaterOnOff;
      digitalWrite(Relay_H, HeaterOnOff);
      Display("lampH", HeaterOnOff);
    }

    if (PumpOnOff) {
      if (PumpCount) {
        PumpCount--;
        Serial.print("PumpCount: ");
        Serial.println(PumpCount);
      } else {
        PumpOnOff = false;
      }
    }
  }//millis()
}//Actor


//// ------------ Plasma ------------
void PlasmaSchedule(unsigned long currentTime) {
  if (currentTime > prevUpdatePlasma + 1000) {
    prevUpdatePlasma = currentTime;

    if (able && plasmaRun) {
      //// ------------ Plasma On ------------
      if (PlasmaOnOff) {
        if (PlasmaOn < 1) {
          PlasmaOnOff = false;
          PlasmaOn = onTime * 60;
        }
        PlasmaOn--;
        send2Nextion("lampMD.val=0");
        send2Nextion("lampMO.val=0");
        digitalWrite(Relay_M, false);
        PlasmaLamp(true, PlasmaOnOff);
        PlasmaRelay(PlasmaOnOff);
      }
      //// ------------ Plasma Off ------------
      else {
        if (PlasmaOff < 1) {
          PlasmaOnOff = true;
          PlasmaOff   = offTime * 60;
          pumpCountD  = pumpDelay * 60;
          PumpCount   = pumpTime;
        }
        PlasmaOff--;
        if (pumpCountD > 1) {
          send2Nextion("lampMD.val=1");
          pumpCountD--;
        } else {
          send2Nextion("lampMD.val=0");
          if (PumpCount > 1 && PumpCount + 9 <= PlasmaOff) {
            PumpCount--;
            send2Nextion("lampM.val=1");
            send2Nextion("lampMO.val=1");
            digitalWrite(Relay_M, true);
          } else {
            if (PumpCount + 10 > PlasmaOff) {
              if (PlasmaOff - 10 > 11) {
                PumpCount = PlasmaOff - 10;
              }
            }
            send2Nextion("lampM.val=0");
            send2Nextion("lampMO.val=0");
            digitalWrite(Relay_M, false);
          }
        }
        PlasmaLamp(true, PlasmaOnOff);
        PlasmaRelay(PlasmaOnOff);
      }
    } else {
      PlasmaOnOff = true;
      PlasmaOn = onTime * 60;
      PlasmaOff = offTime * 60;
      pumpCountD  = pumpDelay * 60;
      PumpCount   = pumpTime;
      PlasmaLamp(false, !PlasmaOnOff);
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

//// ------------ Plasma Lamp ------------
void PlasmaLamp(boolean selectPlasma, boolean lampOnOff) {
  if (f_AmpA) {
    Display("lampPa", lampOnOff);
  } else {
    send2Nextion("lampPa.val=0");
  }
  if (f_AmpB) {
    Display("lampPb", lampOnOff);
  } else {
    send2Nextion("lampPb.val=0");
  }
  Display("lampO",  lampOnOff);
  if (selectPlasma) {
    Display("lampC",  !lampOnOff);
  }
  else            {
    Display("lampC",  lampOnOff);
  }
}

//// ------------ Sensor ------------
void Sensor(unsigned long currentTime) {
  if (currentTime > prevUpdateSensor + 1000) {
    prevUpdateSensor = currentTime;

    uint16_t s_ADCa  = analogRead( s_AmpA );
    uint16_t s_ADCb  = analogRead( s_AmpB );

    if (s_ADCa > 0 || s_ADCb > 0) {
      if (s_ADCa > 0) {
        f_AmpA = true;
      } else {
        f_AmpA = false;
      }
      if (s_ADCb > 0) {
        f_AmpB = true;
      } else {
        f_AmpB = false;
      }
    } else {
      f_AmpA = false;
      f_AmpB = false;
      PlasmaLamp(false, false);
      digitalWrite(Relay_PA, f_AmpA);
      digitalWrite(Relay_PB, f_AmpB);
      send2Nextion("btR.val=0");
    }
    Display("sAmpA", s_ADCa);
    Display("sAmpB", s_ADCb);
    if (f_AmpA && f_AmpB) {
      Display("sensorA", true);
    } else {
      Display("sensorA", false);
    }
    Display("sensorPa", f_AmpA);
    Display("sensorPb", f_AmpB);
  }
}

//// ------------ Display ------------

//// ------------ Clock ------------
void timer(unsigned long currentTime) {
  if (currentTime > prevUpdateTime + 1000) {
    prevUpdateTime = currentTime;
    SEC++;
    if (SEC >= 60) {
      SEC = 0;
      MIN++;
      if (MIN >= 60) {
        MIN = 0;
        HOUR++;
        if (HOUR >= 24)HOUR = 0;
      }
      send2Nextion("Timer.val=1");
    }
    if ((HOUR > AM_S_H || (HOUR == AM_S_H && MIN >= AM_S_M)) &&
        (HOUR < AM_E_H || (HOUR == AM_E_H && MIN < AM_E_M))) {
      // ------------ run moning ------------
      if (!lampDay) {
        PlasmaOnOff = true;
        lampDay     = true;
        send2Nextion("lampD.val=1");
        send2Nextion("lampN.val=0");
      }
      able = true;
    } else if ((HOUR > PM_S_H + 12 || (HOUR == PM_S_H + 12 && MIN >= PM_S_M)) &&
               (HOUR < PM_E_H + 12 || (HOUR == PM_E_H + 12 && MIN < PM_E_M))) {
      // ------------ run afternoon ------------
      if (!lampNight) {
        PlasmaOnOff = true;
        lampNight   = true;
        send2Nextion("lampD.val=0");
        send2Nextion("lampN.val=1");
      }
      able = true;
    } else {
      // ------------ stop ------------
      if (lampDay) {
        lampDay   = false;
        send2Nextion("lampD.val=0");
      }
      if (lampNight) {
        lampNight = false;
        send2Nextion("lampN.val=0");
      }
      PlasmaOnOff = false;
      able        = false;
    }
  }
}//// ------------ End Of Clock ------------

//// ------------ Initializing ------------
void SystemBoot() {
  send2Nextion("page 1");
  pageConfig();
  Display("btWIFI", wifiSet);
  if (wifiSet)  wifiConnect();//WIFI
  send2Nextion("page 0");
  pageMain();

  Serial.println("System all green");
}//// ------------ End Of Initializing ------------

void pageMain() {
  Display("TshD",   AM_S_H);
  Display("TsmD",   AM_S_M);
  Display("TehD",   AM_E_H);
  Display("TemD",   AM_E_M);
  Display("TshN",   PM_S_H);
  Display("TsmN",   PM_S_M);
  Display("TehN",   PM_E_H);
  Display("TemN",   PM_E_M);
  Display("Ton",    onTime);
  Display("Toff",   offTime);
  Display("TPD",    pumpDelay);
  Display("TPR",    pumpTime);
}

void pageConfig() {
  Display("sTshD",  AM_S_H);
  Display("sTsmD",  AM_S_M);
  Display("sTehD",  AM_E_H);
  Display("sTemD",  AM_E_M);
  Display("sTshN",  PM_S_H);
  Display("sTsmN",  PM_S_M);
  Display("sTehN",  PM_E_H);
  Display("sTemN",  PM_E_M);
  Display("sTon",   onTime);
  Display("sToff",  offTime);
  Display("sTPD",   pumpDelay);
  Display("sTPR",   pumpTime);
  Display("CO2",    CO2);
  Display("NH3",    NH3);
  Display("H2S",    H2S);
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

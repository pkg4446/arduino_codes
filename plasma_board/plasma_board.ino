#include "EEPROM.h"
#define EEPROM_SIZE 16
#define SERIAL_MAX  8

#include <SoftwareSerial.h>//https://github.com/plerup/espsoftwareserial/
SoftwareSerial ZE03serial;
#define ZE03_RX 25
#define ZE03_TX 26

HardwareSerial nxSerial(2);
#define RX2 18
#define TX2 19

#define UPDATE_INTERVAL 1000UL

#define DEBUG
#define DEBUG_ZE03

/******************** ZE03-O3 ********************/
uint8_t ZE03[9] = {0,};
uint8_t ZE03Index = 0;
/******************** ZE03-O3 ********************/

/******************** Pin Out ********************/
const int8_t Relay[6] = {13,12,14,16,17,23};
const int8_t led[3]   = {27,32,33};
/******************** Pin Out ********************/

/******************** Routine ********************/
unsigned long prevUpdateTime = 0UL;
/******************** Routine ********************/

/******************** Variable ********************/
uint8_t EEP_Total = 0;
uint8_t EEP_run   = 0;
uint8_t EEP_pause = 0;
/******************** Variable ********************/

/******************** Function ********************/
char Serial_buf[SERIAL_MAX];
int16_t Serial_num;
void Serial_process() {
  char ch;
  ch = nxSerial.read();
  if(ch==0x03){
    Serial_buf[Serial_num] = 0x00;
    Serial.println(Serial_buf);
    //command_pros(Serial_buf);
    Serial_num = 0;
  }else if(ch==0x02){
    Serial_num = 0;
  }else{
    Serial_buf[ Serial_num++ ] = ch;
    Serial_num %= SERIAL_MAX;
  }
}

void DisplayConnect() {
  if (nxSerial.available()) {
    Serial_process();
  }
}

void send2Nextion(String cmd) {
  nxSerial.print(cmd);
  nxSerial.write(0xFF);
  nxSerial.write(0xFF);
  nxSerial.write(0xFF);
}

void Display(String IDs, uint16_t values) {
  String cmd;
  char buf[8] = { 0 };
  sprintf(buf, "%d", values);
  cmd = IDs + ".val=";
  cmd += buf;
  send2Nextion(cmd);
}

void ZE03_O3(){
  if (ZE03serial.available()){
    uint8_t reciveSensor = ZE03serial.read();
    if (reciveSensor == 0xFF) {
      ZE03Index = 0;
      ZE03[ZE03Index++] = reciveSensor;
    } else if (ZE03Index >= 9) {
      ZE03Index = 0;
    } else {
      ZE03[ZE03Index++] = reciveSensor;
      if (ZE03Index == 8) {
        const uint16_t O3_PPM = ZE03[2] * 256 + ZE03[3];
        #ifdef DEBUG_ZE03
          Serial.print("ROWDATA: ");
          for (uint8_t Index = 0; Index < 9; Index++) {
            Serial.print(ZE03[Index],HEX);
            Serial.print(",");
          }
          Serial.print(" O3_PPM/10: ");
          Serial.println(O3_PPM);
        #endif
        Display("n_O3", O3_PPM);
      }
    }
  }
}
/******************** Function ********************/

void setup() {
  Serial.begin(115200);
  if (!EEPROM.begin(EEPROM_SIZE)){
    #ifdef DEBUG
      Serial.println("failed to initialise EEPROM");
    #endif
  }
  ZE03serial.begin(9600, SWSERIAL_8N1, ZE03_RX, ZE03_TX);
  nxSerial.begin(115200, SERIAL_8N1, RX2, TX2);
  for(uint8_t index=0; index<6; index++){
    pinMode(Relay[index], OUTPUT);
  }
  for(uint8_t index=0; index<3; index++){
    pinMode(led[index], OUTPUT);
    digitalWrite(led[index], true);
  }
  Serial.println("System all green");
}
void loop() {
  ZE03_O3();
  DisplayConnect();
}

void relayOnOff(int8_t pinNumber) {
  digitalWrite(Relay[pinNumber], true);
  delay(1000);
  digitalWrite(Relay[pinNumber], false);
  delay(1000);
}

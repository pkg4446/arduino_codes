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
//#define DEBUG_ZE03


/******************** EEP ROM ********************/
uint8_t EEP_Total[2] = {0,1};
uint8_t EEP_run[2]   = {2,3};
uint8_t EEP_pause[2] = {4,5};
/******************** EEP ROM ********************/
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
uint16_t runtime_total_fix = 0;
uint16_t runtime_run_fix   = 0;
uint16_t runtime_pause_fix = 0;
uint16_t runtime_total = 0;
uint16_t runtime_run   = 0;
uint16_t runtime_pause = 0;
/******************** Variable ********************/
/******************** Function ********************/
bool runtime   = false;
bool run_phase = false;
void plasma_run(){
  if(runtime){
    if(run_phase){

    }else{
      
    }
  }
}

char Serial_buf[SERIAL_MAX];
int16_t Serial_num;
void Serial_process() {
  char ch;
  ch = nxSerial.read();
  if(ch==0x03){
    Serial_buf[Serial_num] = 0x00;
    Serial.println(Serial_buf);
    command_pros();
    Serial_num = 0;
  }else if(ch==0x02){
    Serial_num = 0;
  }else{
    Serial_buf[ Serial_num++ ] = ch;
    Serial_num %= SERIAL_MAX;
  }
}

void command_pros(){
  if(Serial_buf[0] == 'C' && Serial_buf[1] == 'M' && Serial_buf[2] == 'D' ){
    if(Serial_buf[4] == 'R' && Serial_buf[5] == 'U' && Serial_buf[6] == 'N' ){
      
    }else if(Serial_buf[4] == 'P' && Serial_buf[5] == 'U' && Serial_buf[6] == 'S' ){
      runtime_total = runtime_total_fix;
      runtime_run   = runtime_run_fix;
      runtime_pause = runtime_pause_fix;
      Display("n_t", runtime_total_fix);
      Display("n_r", runtime_run_fix);
      Display("n_p", runtime_pause_fix);
    }
  }
  /*
  EEPROM.write(a, b);
  EEPROM.commit();
  */
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
/********************* SET UP *********************/
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
  runtime_total = EEPROM.read(EEP_Total[0]);
  runtime_run   = EEPROM.read(EEP_run[0]);
  runtime_pause = EEPROM.read(EEP_pause[0]);
  runtime_total_fix = runtime_total*256 + EEPROM.read(EEP_Total[1]);
  runtime_run_fix   = runtime_run*256   + EEPROM.read(EEP_run[1]);
  runtime_pause_fix = runtime_pause*256 + EEPROM.read(EEP_pause[1]);
  Display("n_t", runtime_total_fix);
  Display("n_r", runtime_run_fix);
  Display("n_p", runtime_pause_fix);

  Serial.println("System all green");
}
/******************** SET UP ********************/
/********************* LOOP *********************/
void loop() {
  ZE03_O3();
  DisplayConnect();
}
/********************* LOOP *********************/
void relayOnOff(int8_t pinNumber) {
  digitalWrite(Relay[pinNumber], true);
  delay(1000);
  digitalWrite(Relay[pinNumber], false);
  delay(1000);
}

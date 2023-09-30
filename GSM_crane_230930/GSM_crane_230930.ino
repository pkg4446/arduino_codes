#include "EEPROM.h"
#define  EEPROM_SIZE 28
#define  SERIAL_MAX  32

unsigned long prevUpdate  = 0UL;

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
  //service run fn
}//Serial_service


void setup() {
  prevUpdate = millis();
  Serial2.begin(115200);
  if (!EEPROM.begin(EEPROM_SIZE)) Serial.println("failed to initialise EEPROM");
}

void loop() {
  // put your main code here, to run repeatedly:

}

void pageMain() {
  send2Nextion("page 0");
  Display("connect", 1);
}

void DisplayConnect() {
  if (Serial2.available()) {
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
  Serial2.print(cmd);
  Serial2.write(0xFF);
  Serial2.write(0xFF);
  Serial2.write(0xFF);
}
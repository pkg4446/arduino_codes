#include <SoftwareSerial.h>//https://github.com/plerup/espsoftwareserial/
HardwareSerial nxSerial(2);
SoftwareSerial board;

#define BOARD_TX 18
#define BOARD_RX 19

void setup() {
  Serial.begin(115200);
  nxSerial.begin(115200, SERIAL_8N1, 16, 17);
  board.begin(115200, SWSERIAL_8N1, BOARD_RX, BOARD_TX, false);
}

void loop() {
  if (Serial.available()) {
    nxSerial.print(Serial.read());
  }
  /*
  if (nxSerial.available()) {
    
  }
  */
  if (board.available()) {
    Serial.print(board.read());
  }
}

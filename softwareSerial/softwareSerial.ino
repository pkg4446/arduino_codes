#include <SoftwareSerial.h>//https://github.com/plerup/espsoftwareserial/
SoftwareSerial ZE03serial;
uint8_t ZE03[9] = {0,};
uint8_t ZE03Index = 0;

void setup() {
  Serial.begin(115200);
  ZE03serial.begin(9600, SWSERIAL_8N1, 23, -1, false, 256);
}

void loop() {
  if (ZE03serial.available()) {
    uint8_t reciveSensor = ZE03serial.read();
    if (reciveSensor == 255) {
      ZE03Index = 0;
      ZE03[ZE03Index++] = reciveSensor;
    } else if (ZE03Index >= 9) {
      ZE03Index = 0;
      Serial.println("ZE03_ERR");
    } else {
      ZE03[ZE03Index++] = reciveSensor;
      if (ZE03Index == 8 && ZE03[8] == 79) {
        Serial.print("ROWDATA: ");
        for (uint8_t Index = 0; Index < 9; Index++) {
          Serial.print(ZE03[Index],HEX);
          Serial.print(",");
        }
        Serial.print(" O3_PPM: ");
        Serial.println(ZE03[2] * 256 + ZE03[3]);
      }
    }
  }


}

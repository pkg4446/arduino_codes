/**
   TCA9548 I2CScanner.ino -- I2C bus scanner for Arduino

   Based on https://playground.arduino.cc/Main/I2cScanner/

*/

#include "Wire.h"
#define TCAADDR 0x70

#include  "Adafruit_SHT31.h"
Adafruit_SHT31 sht31 = Adafruit_SHT31();

void tcaselect(uint8_t i) {
  if (i > 7) return;

  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

// standard Arduino setup()
void setup()
{
  Serial.begin(115200);
  Wire.begin();
}

void loop()
{
  delay(1000);
  Serial.println("\nTCAScanner ready!");
  for (uint8_t t = 0; t < 8; t++) {
    tcaselect(t);
    Serial.print("TCA Port #"); Serial.println(t);

//176,180

    for (uint8_t addr = 0; addr < 255; addr++) {
      if (addr == TCAADDR) continue;

      Wire.beginTransmission(addr);
      if (!Wire.endTransmission()) {
        Serial.print("Found I2C = "); 
        Serial.print(addr);  
        Serial.print(" = 0x");
        Serial.println(addr, HEX);
        
        if (addr == 68 && sht31.begin(0x44)){
          uint8_t Temperature = sht31.readTemperature();
          uint8_t Humidity    = sht31.readHumidity();
          Serial.print("Temperature: ");
          Serial.print(Temperature);
          Serial.print(" ,Humidity: ");
          Serial.println(Humidity);
        }
      }
    }
  }
  Serial.println("\ndone");
}

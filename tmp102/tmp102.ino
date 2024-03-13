/*
Read the temperature from an LM75-derived temperature sensor, and display it
in Celcius every 250ms. Any LM75-derived temperature should work.
*/

#include <Temperature_LM75_Derived.h>
#define SDA2 33
#define SCL2 32
TwoWire I2Ctwo = TwoWire(1);

// The Generic_LM75 class will provide 9-bit (±0.5°C) temperature for any
// LM75-derived sensor. More specific classes may provide better resolution.
Generic_LM75 temperature(&I2Ctwo,0x48);

void setup() {
  Serial.begin(115200);

  I2Ctwo.begin(SDA2,SCL2,400000);
}

void loop() {
  Serial.print("Temperature = ");
  Serial.print(temperature.readTemperatureC());
  Serial.print(" C");

  I2Ctwo.requestFrom(0x48,1);
  uint16_t raw_value = I2Ctwo.read() << 8;
  Serial.print(", rawData = ");
  Serial.print(raw_value);
  Serial.print(" / 256 = ");
  Serial.println(raw_value/256);
  I2Ctwo.endTransmission();
  delay(1000);
}

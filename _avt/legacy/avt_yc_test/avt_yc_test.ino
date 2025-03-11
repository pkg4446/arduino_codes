// this example is public domain. enjoy!
// https://learn.adafruit.com/thermocouple/

#include <max6675.h>
#define TOTAL_TEMPERATURE_SENSOR  4

const uint8_t thermoCS    = 15;
const uint8_t thermoCLK   = 14;
const uint8_t thermoDO[TOTAL_TEMPERATURE_SENSOR] = {32,33,26,27};

MAX6675 thermocouple[TOTAL_TEMPERATURE_SENSOR] = {
  MAX6675(thermoCLK, thermoCS, thermoDO[0]),
  MAX6675(thermoCLK, thermoCS, thermoDO[1]),
  MAX6675(thermoCLK, thermoCS, thermoDO[2]),
  MAX6675(thermoCLK, thermoCS, thermoDO[3])
};

void setup() {
  Serial.begin(115200);

  Serial.println("MAX6675 test");
  // wait for MAX chip to stabilize
}

void loop() {
  // basic readout test, just print the current temp
  for (uint8_t index = 0; index < TOTAL_TEMPERATURE_SENSOR; index++){
    Serial.print(index);
    Serial.print(" ");
    Serial.println(thermocouple[index].readCelsius());
  }
   // For the MAX6675 to update, you must delay AT LEAST 250ms between reads!
   delay(1000);
}
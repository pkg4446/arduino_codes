// this example is public domain. enjoy!
// https://learn.adafruit.com/thermocouple/

#include <max6675.h>
#define TOTAL_TEMPERATURE_SENSOR  5

const uint8_t thermoCS    = 5;
const uint8_t thermoCLK   = 18;
const uint8_t thermoDO[TOTAL_TEMPERATURE_SENSOR] = {33,34,35,36,39};

MAX6675 thermocouple0(thermoCLK, thermoCS, thermoDO[0]);
MAX6675 thermocouple1(thermoCLK, thermoCS, thermoDO[1]);
MAX6675 thermocouple2(thermoCLK, thermoCS, thermoDO[2]);
MAX6675 thermocouple3(thermoCLK, thermoCS, thermoDO[3]);
MAX6675 thermocouple4(thermoCLK, thermoCS, thermoDO[4]);

void setup() {
  Serial.begin(115200);

  Serial.println("MAX6675 test");
  // wait for MAX chip to stabilize
}

void loop() {
  // basic readout test, just print the current temp
  
   Serial.print("1 "); 
   Serial.println(thermocouple0.readCelsius());
 
   Serial.print("2 "); 
   Serial.println(thermocouple1.readCelsius());
 
   Serial.print("3 "); 
   Serial.println(thermocouple2.readCelsius());
 
   Serial.print("4 "); 
   Serial.println(thermocouple3.readCelsius());
 
   Serial.print("5 "); 
   Serial.println(thermocouple4.readCelsius());
 
   // For the MAX6675 to update, you must delay AT LEAST 250ms between reads!
   delay(1000);
}
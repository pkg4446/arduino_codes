#include <Arduino.h>
#include <Wire.h>
#include "PCA9555.h"

#define UPDATE_INTERVAL 1000L
unsigned long prevUpdateTime = 0L;

#define TOTAL_RELAY 16
PCA9555 ioport(0x20);

void setup() {
  Serial.begin(115200);
  Serial.println("Board Test Start");
  // initialize digital pin LED_BUILTIN as an output.
  ioport.begin();
  ioport.setClock(400000);
  
  for (uint8_t index = 0; index < TOTAL_RELAY; index++)
  {
    ioport.pinMode(index, OUTPUT);
  }

  for (uint8_t index = 0; index < TOTAL_RELAY; index++)
  {
    relayOnOff(index);
  }
  Serial.println("Custom Board done.");
}

// the loop function runs over and over again forever
void loop() {
  delay(1);
}

void relayOnOff(int8_t pinNumber) {
  ioport.digitalWrite(pinNumber, true);
  delay(1000);
  ioport.digitalWrite(pinNumber, false);
  delay(1000);
}

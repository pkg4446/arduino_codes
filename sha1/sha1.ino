#include <Arduino.h>
#include "hash.h"

void setup() {
  unsigned long ms;
  Serial.begin(115200);

  // SHA tests
  Serial.print("Result: ");
  ms = micros();
  Sha1.init();
  Sha1.print("abcefg");
  Serial.print(Sha1.result());
  Serial.print(" Hash took : ");
  Serial.print((micros() - ms));
  Serial.println(" micros");

  Serial.print("Result: ");
  ms = micros();
  Sha1.init();
  Sha1.print("abc");
  Sha1.print("efg");
  Serial.print(Sha1.result());
  Serial.print(" Hash took : ");
  Serial.print((micros() - ms));
  Serial.println(" micros");
}

void loop() {
}
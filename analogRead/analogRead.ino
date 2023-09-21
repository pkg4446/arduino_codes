#define  ARRAY 10

uint16_t values[ARRAY] = {0,};
uint8_t  valueIndex = 0;
unsigned long updatePrint  = 0UL;

void setup() {
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  //set the resolution to 12 bits (0-4096)
  analogReadResolution(12);
}

void loop() {
  values[valueIndex++] = analogRead(14);
  if (valueIndex >= ARRAY) valueIndex = 0;

  if (millis() - updatePrint > 100) {
    updatePrint = millis();
    uint16_t analogValue = 0;
    for (uint8_t index = 0; index < ARRAY; index++ ) {
      analogValue += values[index];
    }
    analogValue = analogValue / 10;
    Serial.printf("ADC analog value = %d\n", analogValue);
  }
}

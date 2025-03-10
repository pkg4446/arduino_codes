#define TOTAL_RELAY 10

const int8_t Relay[TOTAL_RELAY] = {2,4,5,12,13,23,27,26,25,33};

#define UPDATE_INTERVAL 1000L
unsigned long prevUpdateTime = 0L;

void setup() {
  Serial.begin(115200);
  Serial.println("Board Test Start");
  // initialize digital pin LED_BUILTIN as an output.
  for (uint8_t index = 0; index < TOTAL_RELAY; index++)
  {
    pinMode(Relay[index], OUTPUT);
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
  digitalWrite(Relay[pinNumber], HIGH);
  delay(1000);
  digitalWrite(Relay[pinNumber], LOW);
  delay(1000);
}

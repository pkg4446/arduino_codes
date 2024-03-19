const int8_t Relay[4] = {12, 13, 14, 16};

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(Relay[0], OUTPUT);
  pinMode(Relay[1], OUTPUT);
  pinMode(Relay[2], OUTPUT);
  pinMode(Relay[3], OUTPUT);
  digitalWrite(Relay[0], true);
  digitalWrite(Relay[1], true);
  digitalWrite(Relay[2], true);
  digitalWrite(Relay[3], true);
}

// the loop function runs over and over again forever
void loop() {
  for (int i = 0; i < 4; i++) {
    relayOnOff(i);
  }
}

void relayOnOff(int8_t pinNumber) {
  digitalWrite(Relay[pinNumber], false);
  delay(2000);
  digitalWrite(Relay[pinNumber], true);
}

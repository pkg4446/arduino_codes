#define UPDATE_INTERVAL 1000L

const int8_t Relay[6] = {13,12,14,16,17,23};
const int8_t led[3]   = {27,32,33};

unsigned long prevUpdateTime = 0L;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(Relay[0], OUTPUT);
  pinMode(Relay[1], OUTPUT);
  pinMode(Relay[2], OUTPUT);
  pinMode(Relay[3], OUTPUT);
  pinMode(Relay[4], OUTPUT);
  pinMode(Relay[5], OUTPUT);

  pinMode(led[0], OUTPUT);
  pinMode(led[1], OUTPUT);
  pinMode(led[2], OUTPUT);
  digitalWrite(led[0], true);
  digitalWrite(led[1], true);
  digitalWrite(led[2], true);

  relayOnOff(0);
  relayOnOff(1);
  relayOnOff(2);
  relayOnOff(3);
  relayOnOff(4);
  relayOnOff(5);
}

// the loop function runs over and over again forever
void loop() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(led[i], false);
    delay(1000);
    digitalWrite(led[i], true);
    delay(1000);
  }
}

void relayOnOff(int8_t pinNumber) {
  digitalWrite(Relay[pinNumber], true);
  delay(1000);
  digitalWrite(Relay[pinNumber], false);
  delay(1000);
}

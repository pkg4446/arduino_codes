const int8_t Relay[6] = {5, 17, 16, 4, 2, 15};
const int8_t Button[4] = {13, 12, 14, 27};

#define UPDATE_INTERVAL 1000L
unsigned long prevUpdateTime = 0L;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(Relay[0], OUTPUT);
  pinMode(Relay[1], OUTPUT);
  pinMode(Relay[2], OUTPUT);
  pinMode(Relay[3], OUTPUT);
  pinMode(Relay[4], OUTPUT);
  pinMode(Relay[5], OUTPUT);

  pinMode(Button[0], INPUT_PULLUP);
  pinMode(Button[1], INPUT_PULLUP);
  pinMode(Button[2], INPUT_PULLUP);
  pinMode(Button[3], INPUT_PULLUP);

  relayOnOff(0);
  relayOnOff(1);
  relayOnOff(2);
  relayOnOff(3);
  relayOnOff(4);
  relayOnOff(5);
}

// the loop function runs over and over again forever
void loop() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(Button[i]) == false) {
      digitalWrite(Relay[i], HIGH);
    }else{
      digitalWrite(Relay[i], LOW);
    }
  }
}

void relayOnOff(int8_t pinNumber) {
  digitalWrite(Relay[pinNumber], HIGH);
  delay(1000);
  digitalWrite(Relay[pinNumber], LOW);
  delay(1000);
}

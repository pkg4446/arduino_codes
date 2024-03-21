const int8_t Relay[4] = {12, 13, 14, 16};

void setup() {
  Serial.begin(115200);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(Relay[0], OUTPUT);
  pinMode(Relay[1], OUTPUT);
  pinMode(Relay[2], OUTPUT);
  pinMode(Relay[3], OUTPUT);
  digitalWrite(Relay[0], true);
  digitalWrite(Relay[1], true);
  digitalWrite(Relay[2], true);
  digitalWrite(Relay[3], false);
}

// the loop function runs over and over again forever
void loop() {


  for (int i = 0; i < 4; i++) {
    relayOnOff(i);
  }
}

void relayOnOff(int8_t pinNumber) {
  bool on = false;
  bool off = true;
  if(pinNumber == 3){
    on = true;
    off = false;
  }
  digitalWrite(Relay[pinNumber], on);
  Serial.print("on:");
  Serial.println(pinNumber);
  delay(2000);
  digitalWrite(Relay[pinNumber], off);
}

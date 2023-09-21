const int8_t Relay[6]  = {5, 17, 16, 4, 2, 15};

void setup() {
  for(int8_t index=0; index<6; index++){
    pinMode(Relay[index], OUTPUT);
  }
}

void loop() {
  for(int8_t index=0; index<6; index++){
    digitalWrite(Relay[index], true);
    delay(500);
    digitalWrite(Relay[index], false);
    delay(500);
  }  
}

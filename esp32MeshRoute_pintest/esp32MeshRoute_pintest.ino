const uint8_t led_pin[8] = {12,13,14,16,17,25,26,27};
/**0*******
*5***1*****
***6*******
*4***2*****
***3***7**/
const uint8_t led_sw[2]  = {32,33};
const uint8_t relay[2]   = {22,23};

void setup() {
  for(uint8_t index=0; index<8; index++){
    pinMode(led_pin[index], OUTPUT);
    digitalWrite(led_pin[index], true);
  }
  for(uint8_t index=0; index<2; index++){
    pinMode(led_sw[index], OUTPUT);
    pinMode(relay[index], OUTPUT);
    digitalWrite(led_sw[index], true);
    digitalWrite(relay[index], true);
  }
}

bool flage_relay = false;
void loop() {
  digitalWrite(relay[0], flage_relay);
  digitalWrite(relay[1], flage_relay);
  for(uint8_t index=0; index<2; index++){
    digitalWrite(led_sw[index], false);
    for(uint8_t number=0; number<8; number++){
      digitalWrite(led_pin[number], false);
      delay(1000);
      digitalWrite(led_pin[number], true);
    }
    digitalWrite(led_sw[index], true); 
  }
  for(uint8_t index=0; index<2; index++){ 
  }
  flage_relay = !flage_relay;
}

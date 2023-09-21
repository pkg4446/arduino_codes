const uint8_t tr_pin[2]      = {8,9};
const uint8_t segment_pin[7] = {1,2,3,4,5,6,7};
bool segment_1st[7] = {false,};
bool segment_2en[7] = {false,};

uint8_t segment_index  = 0;
unsigned long pre_send = 0L;

void setup() {
  Serial.begin(115200);
  for (uint8_t index=0 ; index<7; index++) {
    pinMode(segment_pin[index],INPUT);
  }
  pinMode(tr_pin[0],INPUT_PULLUP);
  pinMode(tr_pin[1],INPUT_PULLUP);
}

uint16_t test1 = 0;
uint16_t test2 = 0;

void loop() {
  
  while (!digitalRead(segment_pin[0])) {
    for (uint8_t index=0 ; index<7; index++) {
      if(digitalRead(segment_pin[index]))segment_1st[index] = true;
    }
    test1++;
  }
  while (!digitalRead(segment_pin[1])) {
    for (uint8_t index=0 ; index<7; index++) {
      if(!digitalRead(segment_pin[index]))segment_2en[index] = true;
    }
    test2++;
  }

  if(millis() > pre_send + 1000){
    Serial.print("1st-");
    for (uint8_t index=0 ; index<7; index++) {
      Serial.print(segment_1st[index]);
    }
    Serial.print(",2nd");
    for (uint8_t index=0 ; index<7; index++) {
      Serial.print(segment_1st[index]);
    }
    Serial.print(",");
    Serial.print(test1);
    Serial.print(",");
    Serial.println(test2);
    pre_send = millis();
    segment_1st[7] = {false,};
    segment_2en[7] = {false,};
    segment_index = 0;
  }
  /*
  ////
  if(digitalRead(segment_pin[segment_index])){
    if(!digitalRead(tr_pin[0])){
      segment_1st[segment_index] = true;
    }else if(!digitalRead(tr_pin[1])){
      segment_2en[segment_index] = true;
    }
  }
  if(segment_index >= 7){
    segment_index = 0;
  }else{
    segment_index += 1;
  }  
  if(millis() > pre_send + 1000){
    Serial.print("1st-");
    for (uint8_t index=0 ; index<7; index++) {
      Serial.print(segment_1st[index]);
    }
    Serial.print(",2nd");
    for (uint8_t index=0 ; index<7; index++) {
      Serial.print(segment_1st[index]);
    }
    Serial.println(",done");
    pre_send = millis();
    segment_1st[7] = {false,};
    segment_2en[7] = {false,};
    segment_index = 0;
  }
  */
}
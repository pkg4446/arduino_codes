/**0*******
*5***1*****
***6*******
*4***2*****
***3***7**/
const uint8_t led_pin[8] = {12,13,14,16,17,25,26,27};
const uint8_t led_sw[2]  = {32,33};
const uint8_t relay[2]   = {22,23};
uint8_t segment_number   = 0;
bool segment_state[8]    = {true,true,true,true,true,true,true,true};

unsigned long segment_update = 0UL;
bool segment_change      = false;
bool segment_cal         = false;
uint8_t segment_interval = 250;

void segment_display(unsigned long millisec){
  if(millisec - segment_update> segment_interval){
    segment_update = millisec;
    if(segment_cal){
      const bool segment[10][8] = {
        {false,false,false,false,false,false,true,true},
        {true,false,false,true,true,true,true,true},
        {false,false,true,false,false,true,false,true},
        {false,false,false,false,true,true,false,true},
        {true,false,false,true,true,false,false,true},
        {false,true,false,false,true,false,false,true},
        {false,true,false,false,false,false,false,true},
        {false,false,false,true,true,false,true,true},
        {false,false,false,false,false,false,false,true},
        {false,false,false,true,true,false,false,true},
      };
      uint8_t one_ten = 0;
      if(segment_change){
        one_ten = segment_number/10;
      }else{
        one_ten = segment_number%10;
      }
      for(uint8_t index=0; index<8; index++){
        if(segment_state[index] != segment[one_ten][index]){
          segment_state[index]   = segment[one_ten][index];
          digitalWrite(led_pin[index], segment_state[index]);
        }
      }
      segment_change = !segment_change;
    }
    if(segment_cal){
      digitalWrite(led_sw[segment_change], true);
      segment_interval = 9;
    }else{
      digitalWrite(led_sw[segment_change], false);
      segment_interval = 0;
    }
    segment_cal = !segment_cal;
  }
}////segment_display end

void setup() {
  Serial.begin(115200);
  for(uint8_t index=0; index<8; index++){
    pinMode(led_pin[index], OUTPUT);
    digitalWrite(led_pin[index], true);
  }
  for(uint8_t index=0; index<2; index++){
    pinMode(led_sw[index], OUTPUT);
    pinMode(relay[index], OUTPUT);
    digitalWrite(led_sw[index], true);
    digitalWrite(relay[index], false);
  }
  segment_number = 35;
}

void loop() {
  segment_display(millis());
}

//const int8_t Relay[6]  = {5, 17, 16, 4, 2, 15};
//const int8_t Button[4] = {13, 12, 14, 27};
//const int8_t ADC[5]    = {33, 32, 34, 39, 35};
//const int8_t DAC[2]    = {25, 26};

uint16_t total_run   = 60*5;
uint8_t interval_on  = 30;
uint8_t interval_off = 30;
//// ------------ Pin out D ---------------
const uint8_t relay_plazma = 15;
const uint8_t relay_buzz   = 5;
//// ------------ Pin out B ---------------
const uint8_t plazma_start = 13;
const uint8_t plazma_stop  = 14;
//// ------------ Value -------------------
uint32_t count_plazma_total = 0;
uint8_t count_plazma_on     = 1;
uint8_t count_plazma_off    = 1;
//// ------------ Flage -------------------
boolean plazma_phaze = true;
boolean plazma_run   = false;
//// ------------ Update Timer ------------
unsigned long prevUpdate = 0UL;

//// ------------ setup -------------------
void setup() {
  prevUpdate = millis();
  Serial.begin(115200);
  //// ------------ Pin Out ---------------
  pinMode(relay_plazma, OUTPUT);
  pinMode(relay_buzz,   OUTPUT);
  pinMode(plazma_start, INPUT_PULLUP);
  pinMode(plazma_stop,  INPUT_PULLUP);
  Serial.println("System Boot");
}//// ------------ End Of Setup() ---------

//// ------------ loop --------------------
void loop() {
  unsigned long time_now = millis();
  simple_run(time_now);
  simple_button();  
}//// ------------ End Of loop() ----------

void simple_run(unsigned long time_now){
  if(plazma_run && ((time_now - prevUpdate) > 1000 )){
    prevUpdate = time_now;
    if(count_plazma_total < total_run){
      count_plazma_total++;
      Serial.print("plazma_total: ");
      Serial.print(count_plazma_total);
      digitalWrite(relay_plazma, plazma_phaze);
      if(plazma_phaze){
        if(count_plazma_on < interval_on){
          count_plazma_on++;
        }else{
          plazma_phaze = !plazma_phaze;
          count_plazma_on = 1;
        }
        Serial.print(", on: ");
        Serial.println(count_plazma_on);
      }else{
        if(count_plazma_off < interval_off){
          count_plazma_off++;
        }else{
          plazma_phaze = !plazma_phaze;
          count_plazma_off = 1;
        }
        Serial.print(", off: ");
        Serial.println(count_plazma_off);
      }
    }else{
      if(plazma_run){
        plazma_run = false;
        for (uint8_t index=0; index<3; index++) {
          digitalWrite(relay_buzz, true);
          delay(1000);
          digitalWrite(relay_buzz, false);
          delay(1000);
        }
        digitalWrite(relay_plazma, false);
        Serial.println("plazma_end");
      }      
    }
  }
}

void simple_button(){
  if(!digitalRead(plazma_start)&&!plazma_run){
    delay(1);
    if(!digitalRead(plazma_start)){
      count_plazma_total = 0;
      plazma_run = true;
      Serial.println("plazma_start");
    }
  }else if(!digitalRead(plazma_stop)&&plazma_run){
    delay(1);
    if(!digitalRead(plazma_stop)){
      digitalWrite(relay_plazma, false);
      plazma_run = false;
      Serial.println("plazma_stop");
    }
  }
}
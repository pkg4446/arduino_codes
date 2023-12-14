#define moter 2

uint8_t DIR[moter] = {43,42};
uint8_t PWM[moter] = {3,6};

uint16_t total_step   = 2000;
uint16_t pwm_interval = 2000;

void setup() {
  for (uint8_t index = 0; index < moter; index++)
  {
    pinMode(DIR[index],OUTPUT);
    pinMode(PWM[index],OUTPUT);
  }
  pinMode(39,OUTPUT);
  digitalWrite(39,true);
}//********** End Of Setup() **********//
bool direction = false;
//********** loop **********//
void loop() {
  direction = !direction;
  digitalWrite(DIR[0],direction);
  digitalWrite(DIR[1],direction);
  for (uint16_t index = 0; index < total_step; index++)
  {
    digitalWrite(PWM[0],true);
    digitalWrite(PWM[1],true);
    delayMicroseconds(4);
    digitalWrite(PWM[0],false);
    digitalWrite(PWM[1],false);
    delayMicroseconds(pwm_interval);
  }  
}//**********End Of loop()**********//

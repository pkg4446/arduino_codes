#define  SERIAL_MAX  32

////AT Code for chage the Something//////////////////////////
char   Serial_buf[SERIAL_MAX] = "hello world!";
int8_t Serial_num;
unsigned long pre_send = 0UL;

void Serial_process() {
  char ch;
  ch = Serial.read();
  switch ( ch ) {
    case ';':
      Serial_buf[Serial_num] = NULL;
      Serial_num = 0;
      break;
    default :
      Serial_buf[ Serial_num ++ ] = ch;
      Serial_num %= SERIAL_MAX;
      break;
  }
}

//// ------------ setup ------------
void setup() {
  pre_send = millis();
  Serial.begin(115200);
}//// ------------ End Of Setup() ------------

//// ------------ loop ------------
void loop() {
  if (Serial.available()) {
    Serial_process();
  }
  if(pre_send+3000 > millis()){
    pre_send = millis();
    Serial.println(Serial_buf);
  }
}//// ------------ End Of loop() ------------

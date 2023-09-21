#define SERIAL_MAX  7

#define NUMBER_OF_SHIFT_CHIPS 2
#define DATA_WIDTH        NUMBER_OF_SHIFT_CHIPS * 8
#define PULSE_WIDTH_USEC  5
// You will need to change the "int" to "long" If the NUMBER_OF_SHIFT_CHIPS is trueer than 2.
#define BYTES_VAL_T unsigned long

//DS=PL2=47, this pin used when using more than two
#define LOAD 49 // PL=PL0=49 //out
#define DIN  48 // Q7=PL1=48 //in
#define CLK  46 // CP=PL3=46 //out
#define CE   16 // CE=PH1=16 //out
////pin set
const uint8_t  p_relay[7]  = {22,23,24,25,26,27,28}; //PA0~6
const uint8_t  p_step[4]   = {10,11,12,13};          //PB4~7
const uint8_t  p_dir[4]    = {35,34,33,32};          //PC2~5

bool     zero_set[4] = {false,}; //step zero set
uint16_t position[4] = {0,}; //step position

//출력 - 브레이크 릴레이 3개, 스탭모터 핀 2(cw,ccw)*3개, 로봇 제어용 릴레이 2개.
//입력 - 컵 유무(정전용량 센서) 3개, 스텝모터 리미트 2*3개

void init_port_base(){
  /*
  DDRA |= 0x3F; //0b00111111
  DDRB |= 0x3C; //0b00111100                          
  DDRC |= 0xF0; //0b11110000
  //All output pins false
  PORTA &= 0xC0; //0b11000000
  PORTB &= 0xC3; //0b11000011
  PORTC &= 0x0F; //0b00001111
  //74HC165
  DDRL |= 0x0B; //0b00001001 //LOAD,CLK Out
  DDRL &= 0xFD; //0b11111101 //DIN In
  DDRH |= 0x02; //0b00000010 //CE Out
  //74HC165 set
  PORTL &= 0xF7; //0b11110111 //CLK false
  PORTL |= 0x01; //0b00000001 //LOAD true
  PORTH &= 0xFD; //0b11111101 //CE false
  */
  for (uint8_t index=0 ; index<7; index++) {
    pinMode(p_relay[index],OUTPUT);
    digitalWrite(p_relay[index], false);
  }
  for (uint8_t index=0 ; index<4; index++) {
    pinMode(p_step[index],OUTPUT);
    pinMode(p_dir[index],OUTPUT);
    digitalWrite(p_step[index], false);
    digitalWrite(p_dir[index], false);
  }
  pinMode(LOAD,OUTPUT);
  pinMode(DIN,INPUT);
  pinMode(CLK,OUTPUT);
  pinMode(CE,OUTPUT);
  digitalWrite(CLK,  false);
  digitalWrite(LOAD, true);
  digitalWrite(CE,   false);
}

BYTES_VAL_T read_shift_regs()
{
    long bitVal;
    BYTES_VAL_T bytesVal = 0;
  //digitalWrite(CE, true);
    digitalWrite(LOAD, false);  //remove_noize
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(LOAD, true);
  //digitalWrite(CE, false);
    for(int i = 0; i < DATA_WIDTH; i++)
    {
        bitVal = digitalRead(DIN);
        bytesVal |= (bitVal << ((DATA_WIDTH-1) - i));
        digitalWrite(CLK, true);
        //delayMicroseconds(PULSE_WIDTH_USEC);
        digitalWrite(CLK, false);
    }
    return(bytesVal);
}

void steper(uint8_t number, bool direction, uint16_t step, uint16_t speed){
  if(direction && zero_set[number]){ //up
    digitalWrite(p_dir[number], true);
    for (uint16_t index=0; index<step; index++) {
      //---------check this---------------------- max hight
      if(position[number] > 1000) break; //if position is higher than maximum hight, stop
      digitalWrite(p_step[number], true);
      position[number] += 1;
      digitalWrite(p_step[number], false);
      delayMicroseconds (speed);
    }
  }else{  //down
    BYTES_VAL_T pinValues = read_shift_regs(); //check the limit sw
    digitalWrite(p_dir[number], false);
    for (uint16_t index=0; index<step; index++) {
      //---------check this----------------------- limit sw pin
      if(pinValues) break; //when push the limit sw, stop
      digitalWrite(p_step[number], true);
      if(position[number] > 0) position[number] -= 1;
      digitalWrite(p_step[number], false);
      delayMicroseconds (speed);
    }
  }
}

////command chage the Something//////////////////////////
char    Serial_buf[SERIAL_MAX];
uint8_t Serial_num;
unsigned long pre_loop_1 = 0UL;

void Serial_process() {
  char ch;
  ch = Serial.read();
  switch ( ch ) {
    case 0x53: //strat
      Serial_buf[Serial_num] = NULL;
      Serial_num += 1;
      break;
    case 0x0A: //end
      Serial_buf[Serial_num] = NULL;
      command_Service();
      Serial_num = 0;
      break;
    default :
      Serial_buf[ Serial_num++ ] = ch;
      Serial_num %= SERIAL_MAX;
      break;
  }
}

void command_Service() {
  //check sum
  //command
  //run
  //response
}//Command_service() END

//// ------------ setup ------------
void setup() {
  Serial.begin(115200);
  init_port_base();
}//// ------------ End Of Setup() ------------

//// ------------ loop ------------
void loop() {
  if (Serial.available()) {
    Serial_process();
  }
  display_pin_values();
  //if need asynchronous, add something
}//// ------------ End Of loop() ------------

unsigned long interval_74HC165 = 0;
void display_pin_values()
{
    BYTES_VAL_T pinValues = read_shift_regs();
    if(interval_74HC165 - millis() > 1000){
      Serial.print("Pin States:\r\n");

      for(int i = 0; i < DATA_WIDTH; i++)
      {
          Serial.print("  Pin-");
          Serial.print(i);
          Serial.print(": ");

          if((pinValues >> i) & 1)
              Serial.print("HIGH");
          else
              Serial.print("LOW");

          Serial.print("\r\n");
      }

      Serial.print("\r\n");
    }
}
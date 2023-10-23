#include "control.h"

MOTOR test;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

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

  test.begin(12);
  //driver.begin(1);
}

void loop() {

  // put your main code here, to run repeatedly:

}

#include "pin_setup.h"
#include "moter_control.h"

#include <SPI.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define SERIAL_MAX 16

MOTOR driver[4];

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
  pinMode(LOAD,OUTPUT);
  pinMode(DIN,INPUT);
  pinMode(CLK,OUTPUT);
  pinMode(CE,OUTPUT);
  digitalWrite(CLK,  false);
  digitalWrite(LOAD, true);
  digitalWrite(CE,   false);
}


//********** setup**********//
void setup() {
  Serial.begin(115200);
  Serial.println("System boot... wait a few seconds.");
  init_port_base();
  for (uint8_t index=0 ; index<4; index++) {
    driver[index].init(stepDriver[index]);
    driver[index].status();
  }
}//********** End Of Setup() **********//

//********** loop **********//
void loop() {
  //if need asynchronous, add something
}//**********End Of loop()**********//

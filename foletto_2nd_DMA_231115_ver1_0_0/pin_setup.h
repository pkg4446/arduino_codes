#pragma once

#include "arduino.h"

#define LOAD 49 // PL=PL0=49 //out
#define DIN  48 // Q7=PL1=48 //in
#define CLK  46 // CP=PL3=46 //out
#define CE   16 // CE=PH1=16 //out

#define BUITIN_EN 39

#define PIN_PL7 42
#define PIN_PL6 43

#define PIN_PH5 8
#define PIN_PH4 7
#define PIN_PH3 6
#define PIN_PE5 3
#define PIN_PE4 2
#define PIN_PE3 5

#define PIN_PC5 32
#define PIN_PC4 33
#define PIN_PC3 34
#define PIN_PC2 35

#define PIN_PB7 13
#define PIN_PB6 12
#define PIN_PB5 11
#define PIN_PB4 10

#define PIN_PE7 32
#define PIN_PE6 33
#define PIN_PH7 34
#define PIN_PE2 35

const uint8_t relay_pin[7]  = {22,23,24,25,26,27,28}; //PA0~6 //여기서 브레이크 선정. 로봇제어 선정
const uint8_t robot_arm[3]  = {26,27,28};
const uint8_t step_break[4] = {22,23,24,25};

typedef struct PACKET_ts{
	uint8_t  svr[4];
	uint8_t  local[4];
	uint8_t  subnet[4];
	uint8_t  gateway[4];
	uint8_t  mac;
	int8_t   motor[10];
}PACKET_ts;

typedef struct STEP_ts{
	int8_t   DIR; // CCW
	int8_t   PWM; // CW
}STEP_ts;
const STEP_ts stepMotor[6]={
   {PIN_PE6,PIN_PH4},//digital pin{null,7} //DDRE |= 0b11000100;
   {PIN_PE7,PIN_PH5},//digital pin{null,8}
   {PIN_PL6,PIN_PE5},//digital pin{43,3}
   {PIN_PL7,PIN_PH3},//digital pin{42,6}
   {PIN_PE2,PIN_PE3},//digital pin{null,5}
   {PIN_PH7,PIN_PE4} //digital pin{null,2} //DDRH |= 0b10000000;
};
const STEP_ts stepDriver[4]={
   {PIN_PC5,PIN_PB7},//digital pin{32,13}
   {PIN_PC4,PIN_PB6},//digital pin{33,12}
   {PIN_PC3,PIN_PB5},//digital pin{34,11}
   {PIN_PC2,PIN_PB4} //digital pin{35,10}
};
/*
typedef struct LOAD_ts{
   int8_t   DOUT;
   int8_t   SCKD;
}LOAD_ts;
const LOAD_ts LOADCELL[16]={
   {PIN_PD5, PIN_PF6},
   {PIN_PJ6, PIN_PK6},
   {PIN_PD4, PIN_PF7},
   {PIN_PJ7, PIN_PK7},
   {PIN_PD7, PIN_PF4},
   {PIN_PJ4, PIN_PK4},
   {PIN_PD6, PIN_PF5},
   {PIN_PJ5, PIN_PK5},
   {PIN_PL4, PIN_PF2},
   {PIN_PJ2, PIN_PK2},
   {PIN_PH2, PIN_PF3},
   {PIN_PJ3, PIN_PK3},
   {PIN_PC0, PIN_PF0},
   {PIN_PJ0, PIN_PK0},
   {PIN_PC1, PIN_PF1},
   {PIN_PJ1, PIN_PK1}
};
const int dout[8] = {
		PIN_PJ7,
		PIN_PJ6,
		PIN_PJ5,
		PIN_PJ4,
		PIN_PJ3,
		PIN_PJ2,
		PIN_PJ1,
		PIN_PJ0};
*/
// const uint8_t SenSeq[]={4,5,6,7,12,13,14,15,0,1,2,3,8,9,10,11};
const uint8_t SenSeq[16]={11,10,9,8,3,2,1,0,15,14,13,12,7,6,5,4};
const int LOADCELL_SCK_PIN = PIN_PB5;
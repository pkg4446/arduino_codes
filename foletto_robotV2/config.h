#ifndef __CONFIG_H__
#define __CONFIG_H__
#include <Arduino.h>

// #define __DEBUG__
#define dfEN			PIN_PG1
#define dfSLEEP      PIN_PG2
#define dfSCKD			PIN_PB5
#define EOT				4
#define L				0
#define H				1
const uint8_t conRELAY[]={28,27,26,25,24,23,22};
typedef struct STEP_ts{
	int 		DIR;
	int 		PWM;
	uint32_t	Position;
}STEP_ts;
typedef union uni_conv{
	float    fval;
	int32_t  ival;
	uint16_t u16[2];
}uni_conv;
typedef struct LOAD_ts{
   int      DOUT;
   int      SCKD;
}LOAD_ts;
typedef struct PACKET_ts{
	uint8_t  svr[4];
	uint8_t  local[4];
	uint8_t  subnet[4];
	uint8_t  gateway[4];
	uint8_t  mac;
	int8_t   motor[10];
}PACKET_ts;

const LOAD_ts LOADCELL[]={
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

typedef struct HC165_ts{
	int MISO;			
	int MOSI;			
	int CLK;			
	int PL;				
	int CE;				
}HC165_ts;
const HC165_ts shiftResister={48,47,46,49,16};

const STEP_ts stepMotor[]={
   {PIN_PE6,PIN_PH4,0},
   {PIN_PE7,PIN_PH5,0},
   {PIN_PL6,PIN_PE5,0},
   {PIN_PL7,PIN_PH3,0},
   {PIN_PE2,PIN_PE3,0},
   {PIN_PH7,PIN_PE4,0},
   {PIN_PC5,PIN_PB7,0},
   {PIN_PC4,PIN_PB6,0},
   {PIN_PC3,PIN_PB5,0},
   {PIN_PC2,PIN_PB4,0},   
};

enum emmoryName{
	eURL,
	eSSID,
	ePASSWORD,
	
};
const int dout[] = {
		PIN_PJ7,
		PIN_PJ6,
		PIN_PJ5,
		PIN_PJ4,
		PIN_PJ3,
		PIN_PJ2,
		PIN_PJ1,
		PIN_PJ0};
// const uint8_t SenSeq[]={4,5,6,7,12,13,14,15,0,1,2,3,8,9,10,11};
const uint8_t SenSeq[]={11,10,9,8,3,2,1,0,15,14,13,12,7,6,5,4};
const int LOADCELL_SCK_PIN = PIN_PB5;
#endif
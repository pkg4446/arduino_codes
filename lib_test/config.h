#include <Arduino.h>

// #define __DEBUG__
#define dfEN			PG1
#define dfSLEEP      PG2
#define dfSCKD			PB5
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
   {PD5, PF6},
   {PJ6, PK6},
   {PD4, PF7},
   {PJ7, PK7},
   {PD7, PF4},
   {PJ4, PK4},
   {PD6, PF5},
   {PJ5, PK5},
   {PL4, PF2},
   {PJ2, PK2},
   {PH2, PF3},
   {PJ3, PK3},
   {PC0, PF0},
   {PJ0, PK0},
   {PC1, PF1},
   {PJ1, PK1}
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
   {PE6,PH4,0},
   {PE7,PH5,0},
   {PL6,PE5,0},
   {PL7,PH3,0},
   {PE2,PE3,0},
   {PH7,PE4,0},
   {PC5,PB7,0},
   {PC4,PB6,0},
   {PC3,PB5,0},
   {PC2,PB4,0},   
};

enum emmoryName{
	eURL,
	eSSID,
	ePASSWORD,
	
};
const int dout[] = {
		PJ7,
		PJ6,
		PJ5,
		PJ4,
		PJ3,
		PJ2,
		PJ1,
		PJ0};
// const uint8_t SenSeq[]={4,5,6,7,12,13,14,15,0,1,2,3,8,9,10,11};
const uint8_t SenSeq[]={11,10,9,8,3,2,1,0,15,14,13,12,7,6,5,4};
const int LOADCELL_SCK_PIN = PB5;
#ifndef _pin_setup_h
#define _pin_setup_h

#define dfEN			PIN_PG1
#define dfSLEEP      PIN_PG2
#define dfSCKD			PIN_PB5
#define EOT				4
#define L				0
#define H				1


#define LOAD 49 // PL=PL0=49 //out
#define DIN  48 // Q7=PL1=48 //in
#define CLK  46 // CP=PL3=46 //out
#define CE   16 // CE=PH1=16 //out

#define PIN_PE4 2
#define PIN_PE5 3
#define PIN_PE3 5
#define PIN_PH3 6
#define PIN_PH4 7
#define PIN_PH5 8

#define PIN_PB4 10
#define PIN_PB5 11
#define PIN_PB6 12
#define PIN_PB7 13
#define PIN_PJ1 14
#define PIN_PJ0 15

#define PIN_PC5 32
#define PIN_PC4 33
#define PIN_PC3 34
#define PIN_PC2 35
#define PIN_PC1 36
#define PIN_PC0 37
#define PIN_PD7 38

#define PIN_PL7 42
#define PIN_PL6 43
#define PIN_PL4 45

#define PIN_PF0 54
#define PIN_PF1 55
#define PIN_PF2 56
#define PIN_PF3 57
#define PIN_PF4 58
#define PIN_PF5 59
#define PIN_PF6 60
#define PIN_PF7 61
#define PIN_PK0 62
#define PIN_PK1 63
#define PIN_PK2 64
#define PIN_PK3 65
#define PIN_PK4 66
#define PIN_PK5 67
#define PIN_PK6 68
#define PIN_PK7 69
#define PIN_PE2 70
#define PIN_PE6 71
#define PIN_PE7 72
#define PIN_PH2 73
#define PIN_PH7 74

#define PIN_PD4 77
#define PIN_PD5 78
#define PIN_PD6 79
#define PIN_PJ2 80
#define PIN_PJ3 81
#define PIN_PJ4 82
#define PIN_PJ5 83
#define PIN_PJ6 84
#define PIN_PJ7 85

const uint8_t p_relay[] = {22,23,24,25,26,27,28}; //PA0~6 //여기서 브레이크 선정. 로봇제어 선정
const uint8_t robot_arm[3]  = {26,27,28};
const uint8_t step_break[4] = {22,23,24,25};

/*
typedef struct STEP_OUT_ts{
	int 		cw;
	int 		ccw;
	uint32_t	Position;
}STEP_ts;
*/

typedef struct STEP_ts{
	int8_t   DIR; // CCW
	int8_t   PWM; // CW
	uint32_t	Position;
   bool     zero;
}STEP_ts;
const STEP_ts stepMotor[]={
   {PIN_PE6,PIN_PH4,0,false},
   {PIN_PE7,PIN_PH5,0,false},
   {PIN_PL6,PIN_PE5,0,false},
   {PIN_PL7,PIN_PH3,0,false},
   {PIN_PE2,PIN_PE3,0,false},
   {PIN_PH7,PIN_PE4,0,false}
};
const STEP_ts stepDriver[]={
   {PIN_PC5,PIN_PB7,0,false},
   {PIN_PC4,PIN_PB6,0,false},
   {PIN_PC3,PIN_PB5,0,false},
   {PIN_PC2,PIN_PB4,0,false}
};

typedef struct LOAD_ts{
   int8_t   DOUT;
   int8_t   SCKD;
}LOAD_ts;
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
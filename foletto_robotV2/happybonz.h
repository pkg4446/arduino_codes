#ifndef __HAPPY_BONZ_H__
#define __HAPPY_BONZ_H__
#include <Arduino.h>
#include <config.h>
#include <Ethernet2.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <ArduinoRS485.h>
#include <ArduinoModbus.h>
const int bufferSize=100;
const int maxTokens=50;
typedef struct happybonz_ts
{
	uint8_t			sen;
	uint8_t			e;
	uint8_t			t;
	uint8_t			cmd;
	uint8_t			num;

	uint8_t			data[4];
	uint8_t			eot;
}happybonz_ts;


typedef union uc_ts{
	uint16_t		u16;
	uint8_t			byte[2];
	struct bits{
		uint16_t	b0:1;
		uint16_t	b1:1;
		uint16_t	b2:1;
		uint16_t	b3:1;
		uint16_t	b4:1;
		uint16_t	b5:1;
		uint16_t	b6:1;
		uint16_t	b7:1;
		uint16_t	b8:1;
		uint16_t	b9:1;
		uint16_t	b10:1;
		uint16_t	b11:1;
		uint16_t	b12:1;
		uint16_t	b13:1;
		uint16_t	b14:1;
		uint16_t	b15:1;
	}bits;
}uc_ts;

class HAPPYBONZ{
    HardwareSerial *p;
	uint16_t	soltime;
	uint8_t 	sensor[3];
	STEP_ts		sMotor[8];
	uc_ts		uc;
	
public:
	union smotor{
		uint8_t all;
		struct bits{
			uint8_t	pickup1	:1;	
			uint8_t	pickup2	:1;	
			uint8_t	pickup3	:1;	
			uint8_t	updown	:1;	
			uint8_t	ready	:1;	
			uint8_t	upcheck	:1;	
			uint8_t	pickup4	:1;	
			uint8_t nc		:1;
		}bits;
	}smotor,cw, run_flag;
	uc_ts		inp;
	int motors;

	int cwccw;
	char buffer[bufferSize];

	char *tokens[30];

	String stokens[maxTokens];
	bool b=0;
	long h_cnt[10]={0};
	bool m_flag[10]={0};
	bool s_flag[10]={0};
	uint16_t shiftInput;

    void begin(HardwareSerial *_p);

    void parsing();

	void parsingSetup();

	void parsingSetup(String s);
	void cmdParsor();
	void ipSetup();
	void zeroSetup();
	void dbSetup();
	// void memoryClear();
	int splitString(String input, char delimiter, String tokens[], int maxTokens);
	int splitString(char *input, char *tokens[], char delimiter, int maxTokens);

	void sensorRead();
	void solon();
	void soloff();
	void soltoggle();
	void motor(uint8_t sen);
	// void setMotor();

	void setInput(ModbusTCPServer *mb, uint16_t x);
	void response();

	void relayProc(ModbusTCPServer *mb);

	void loadcellProc(ModbusTCPServer *mb, int i);

	void sol_Response();

	void motor_Response();
	int timedRead();

	void ipReceiveProc(ModbusTCPServer *mb);

	void ipTransProc(ModbusTCPServer *mb);

	void stepmotorTask(ModbusTCPServer *mb);

	float randomDouble(double minf, double maxf);
};
#endif
/**
 * @file happybonz.cpp
 * @author Shin Hyeon Tae (fishiq@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-07-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <happybonz.h>
#include <config.h>
#include "HX711.h"
HAPPYBONZ hp;
happybonz_ts happy;
HX711 scale[16];
uni_conv readloadcell[16];
uni_conv stepPos[8];
extern IPAddress ip;
// extern IPAddress serverIP;
extern byte mac[];
extern IPAddress subnet;
extern IPAddress gateway;
extern uint8_t rxbuf[sizeof(PACKET_ts)];
extern PACKET_ts network;
// extern STEP_ts stepMotor;
void HAPPYBONZ::begin(HardwareSerial *_p){
    p = _p;
	p->println("Start Serial Port");
}
uint32_t _startMillis;
uint32_t _timeout = 1000;
int HAPPYBONZ::timedRead()
{
	int c;
	_startMillis = millis();
	do {
		c = p->read();
		if (c >= 0) return c;
	} while(millis() - _startMillis < _timeout);
	return -1;
}

void HAPPYBONZ::stepmotorTask(ModbusTCPServer *mb){
	for(int i = 0; i < 10;i++){
	long m_step = mb->holdingRegisterRead(i);
	if(m_step > 32767) m_step -= 65536;
	int sensor_no = network.motor[i];
	if(mb->coilRead(i+8)){ 
		if(m_flag[i]){
		m_flag[i] = 0;
		digitalWrite(dfSLEEP, HIGH);
		}
		if((shiftInput>>sensor_no&1)==0){
		digitalWrite(stepMotor[i].DIR, 1);
		digitalWrite(stepMotor[i].PWM, b);
		}
		else{
		h_cnt[i] = 0;
		s_flag[i] = 1;
		mb->coilWrite(i+8,0);

		digitalWrite(dfSLEEP, LOW);
		digitalWrite(stepMotor[i].DIR, LOW);
		digitalWrite(stepMotor[i].PWM, LOW);
		m_flag[i] = 1;
		}
		continue;
	}
	if(h_cnt[i] == m_step) {               
		if(m_flag[i] == 0){
		digitalWrite(dfSLEEP, LOW);
		// digitalWrite(dfEN, HIGH);
		digitalWrite(stepMotor[i].DIR, LOW);
		digitalWrite(stepMotor[i].PWM, LOW);
		m_flag[i] = 1;
		}
		continue;
	}
	if(m_flag[i]){
		m_flag[i] = 0;
		digitalWrite(dfSLEEP, HIGH);
		// digitalWrite(dfEN, LOW);
	}
	else{
		if(h_cnt[i] < m_step){               
		h_cnt[i]++;
		digitalWrite(stepMotor[i].DIR, 0);
		digitalWrite(stepMotor[i].PWM, b);
		}
		else{                           
		h_cnt[i]--;
		digitalWrite(stepMotor[i].DIR, 1);
		digitalWrite(stepMotor[i].PWM, b);
		}
	}

	if((shiftInput>>sensor_no&1) == 0){            
		s_flag[i] = 0;
	}
	else if(s_flag[i] == 0 && h_cnt[i] > m_step){   
		h_cnt[i] = 0;     
		s_flag[i] = 1;     
		mb->holdingRegisterWrite(i, 0);
	}
	}
	b^=1;
}
void HAPPYBONZ::parsingSetup(){
    if(!p->available()) return;
	int byteRead = p->readBytesUntil('\n',buffer, bufferSize);
	buffer[byteRead] = '\0';
	int tokenCount = splitString(buffer,tokens,',',30);
	#ifdef __DEBUG__
	for(int i = 0; i <tokenCount;i++){
		p->print("Token ");
		p->print(i);
		p->print(":");
		p->println(tokens[i]);
	}
	#endif
}
void HAPPYBONZ::parsingSetup(String s){
	if(!p->available()) return;
	String input = Serial.readStringUntil('\r');
	splitString(input,',', stokens, maxTokens);
	// #ifdef __DEBUG__
	// for(int i = 0; i < tokenCount;i++){
	// 	p->print("Token ");
	// 	p->print(i);
	// 	p->print(":");
	// 	p->println(stokens[i]);
	// }
	// #endif
}
void HAPPYBONZ::ipSetup(){
	network.svr[0]		=	stokens[1].toInt();
	network.svr[1]		=	stokens[2].toInt();
	network.svr[2]		=	stokens[3].toInt();
	network.svr[3]		=	stokens[4].toInt();
	
	network.local[0]	=	stokens[5].toInt();
	network.local[1]	=	stokens[6].toInt();
	network.local[2]	=	stokens[7].toInt();
	network.local[3]	=	stokens[8].toInt();

	network.subnet[0]	=	stokens[9].toInt();
	network.subnet[1]	=	stokens[10].toInt();
	network.subnet[2]	=	stokens[11].toInt();
	network.subnet[3]	=	stokens[12].toInt();

	network.gateway[0]	=	stokens[13].toInt();
	network.gateway[1]	=	stokens[14].toInt();
	network.gateway[2]	=	stokens[15].toInt();
	network.gateway[3]	=	stokens[16].toInt();

	network.mac			=	stokens[17].toInt();

	network.motor[0]	=	stokens[18].toInt();
	network.motor[1]	=	stokens[19].toInt();
	network.motor[2]	=	stokens[20].toInt();
	network.motor[3]	=	stokens[21].toInt();
	network.motor[4]	=	stokens[22].toInt();
	network.motor[5]	=	stokens[23].toInt();
	network.motor[6]	=	stokens[24].toInt();
	network.motor[7]	=	stokens[25].toInt();
	network.motor[8]	=	stokens[26].toInt();
	network.motor[9]	=	stokens[27].toInt();	

	eeprom_write_block((void*)(&network), (void*)0, sizeof(network));	
	eeprom_busy_wait();
	
	p->print("OK\r"); 
	p->print("Restarted.\r");
	p->flush();
	delay(1000);
	asm("jmp 0");
	// #ifdef __DEBUG__
	// p->println("IP");
	// for(int i = 0; i < 16;i++){
	// 	p->print(rxbuf[i]+String(","));
	// }
	// p->print(network->port);
	// p->println();
	// #endif
}
void HAPPYBONZ::dbSetup(){
	PACKET_ts temp;
	eeprom_read_block((void*)(&temp), (void*)0, sizeof(temp));
	network = temp;
	if(temp.svr[0] != 0xff)return;

}
void HAPPYBONZ::zeroSetup(){
	network.motor[0] = stokens[1].toInt();
	network.motor[1] = stokens[2].toInt();
	network.motor[2] = stokens[3].toInt();
	network.motor[3] = stokens[4].toInt();
	network.motor[4] = stokens[5].toInt();
	network.motor[5] = stokens[6].toInt();
	network.motor[6] = stokens[7].toInt();
	network.motor[7] = stokens[8].toInt();
	network.motor[8] = stokens[9].toInt();
	network.motor[9] = stokens[10].toInt();
	#ifdef __DEBUG__
	p->println("ZERO");
	for(int i = 0; i < 10;i++){
		p->print(String("Motor [")+String(i)+String("]")+network.motor[i]+String(","));
	}
	p->println();
	#endif
}

void HAPPYBONZ::cmdParsor(){
	// eeprom_write_block((void*)(&network), (void*)0, sizeof(network));			
	if(stokens[0] == "IP") {ipSetup();}
	// else if(stokens[0] == "ZERO") {zeroSetup();}
	// else p->println("Error Comunication");

	for(int i = 0;i < maxTokens;i++) stokens[i]="";
}
int HAPPYBONZ::splitString(char *input, char *tokens[], char delimiter, int maxTokens){
	int tokenIndex = 0;
	char *ptr = strtok(input, &delimiter);
	while(ptr != NULL&&tokenIndex < maxTokens){
		tokens[tokenIndex] = ptr;
		tokenIndex++;
		ptr = strtok(NULL,&delimiter);
	}
	return tokenIndex;
}
int HAPPYBONZ::splitString(String input, char delimiter, String tokens[], int maxTokens){
	int tokenIndex = 0;
	int startIndex = 0;
	int endIndex = input.indexOf(delimiter);
	while(endIndex != -1 && tokenIndex < maxTokens){
		stokens[tokenIndex] = input.substring(startIndex, endIndex);
		startIndex = endIndex+1;
		endIndex = input.indexOf(delimiter, startIndex);
		tokenIndex++;
	}
	if(startIndex < (int)input.length() && tokenIndex < maxTokens){
		stokens[tokenIndex] = input.substring(startIndex);
		tokenIndex++;
	}
	return tokenIndex;
}
void HAPPYBONZ::parsing(){
	uint8_t rxbuf[9]={0};
	if(!p->available()) return;
	
	uint8_t length = 9;
	uint8_t index = 0;
	while (index < length) {
		int c = timedRead();
		if (c < 0 || (c == EOT && index > 7)) break;
		rxbuf[index] = (char)c;
		index++;
	}
	
	if(rxbuf[0] == 'S' && rxbuf[1] == 'E' && rxbuf[2] == 'T'){
		
		happy.cmd = rxbuf[3];
		happy.sen = rxbuf[4];
		happy.num = rxbuf[5];
		
		soltime = ((rxbuf[5]&0xf0)>>4)*10+(rxbuf[5]&0x0f)*100;
		switch(happy.cmd){
			case 0xA0: sensorRead(); break;
			case 0xB1: soltoggle(); break;
			case 0xB0: 
				if(happy.num == 0) soloff();
				else if(happy.num == 0xff)solon(); 
				break;
			case 0xD0: motor(happy.sen); break;
		}
	}else if(rxbuf[0] == 'H' && rxbuf[1] == 'O' && rxbuf[2] == 'M'){
		p->write('C');
	}else if(rxbuf[0] == 'C' && rxbuf[1] == 'L' && rxbuf[2] == 'E'){
		p->write('K');
    }else{
		uint8_t txbuf[]={'A','C','K',0xff,0xff,0xff,0xff,0xff,0xFF};
		p->write(txbuf,9);
	}
	memset(rxbuf,0, 8);
}
void HAPPYBONZ::ipReceiveProc(ModbusTCPServer *mb){
	ip.operator[](0)= mb->holdingRegisterRead(50);
	ip.operator[](1)= mb->holdingRegisterRead(51);
	ip.operator[](2)= mb->holdingRegisterRead(52);
	ip.operator[](3)= mb->holdingRegisterRead(53);

	// serverIP.operator[](0)= mb->holdingRegisterRead(60);
	// serverIP.operator[](1)= mb->holdingRegisterRead(61);
	// serverIP.operator[](2)= mb->holdingRegisterRead(62);
	// serverIP.operator[](3)= mb->holdingRegisterRead(63);

	subnet.operator[](0)= mb->holdingRegisterRead(64);
	subnet.operator[](1)= mb->holdingRegisterRead(65);
	subnet.operator[](2)= mb->holdingRegisterRead(66);
	subnet.operator[](3)= mb->holdingRegisterRead(67);

	gateway.operator[](0)= mb->holdingRegisterRead(68);
	gateway.operator[](1)= mb->holdingRegisterRead(69);
	gateway.operator[](2)= mb->holdingRegisterRead(70);
	gateway.operator[](3)= mb->holdingRegisterRead(71);

	mac[0]= mb->holdingRegisterRead(54);
	mac[1]= mb->holdingRegisterRead(55);
	mac[2]= mb->holdingRegisterRead(56);
	mac[3]= mb->holdingRegisterRead(57);
	mac[4]= mb->holdingRegisterRead(58);
	mac[5]= mb->holdingRegisterRead(59);
}
void HAPPYBONZ::ipTransProc(ModbusTCPServer *mb){
	// mb->holdingRegisterRead(0);
	// mb->writeInputRegisters(0,100);
	// /*
	mb->holdingRegisterWrite(50, ip.operator[](0));
	mb->holdingRegisterWrite(51, ip.operator[](1));
	mb->holdingRegisterWrite(52, ip.operator[](2));
	mb->holdingRegisterWrite(53, ip.operator[](3));
	
	// mb->holdingRegisterWrite(60, serverIP.operator[](0));
	// mb->holdingRegisterWrite(61, serverIP.operator[](1));
	// mb->holdingRegisterWrite(62, serverIP.operator[](2));
	// mb->holdingRegisterWrite(63, serverIP.operator[](3));

	mb->holdingRegisterWrite(64, subnet.operator[](0));
	mb->holdingRegisterWrite(65, subnet.operator[](1));
	mb->holdingRegisterWrite(66, subnet.operator[](2));
	mb->holdingRegisterWrite(67, subnet.operator[](3));

	mb->holdingRegisterWrite(68, gateway.operator[](0));
	mb->holdingRegisterWrite(69, gateway.operator[](1));
	mb->holdingRegisterWrite(70, gateway.operator[](2));
	mb->holdingRegisterWrite(71, gateway.operator[](3));

	mb->holdingRegisterWrite(54, (int)mac[0]);
	mb->holdingRegisterWrite(55, (int)mac[1]);
	mb->holdingRegisterWrite(56, (int)mac[2]);
	mb->holdingRegisterWrite(57, (int)mac[3]);
	mb->holdingRegisterWrite(58, (int)mac[4]);
	mb->holdingRegisterWrite(59, (int)mac[5]);
	// 
}
void HAPPYBONZ::sensorRead(){
	uint8_t txbuf[9]={'A','C','K',0xC0,0x00,0x00,0x00,0x00,EOT};
	txbuf[4] = 0x33&((inp.bits.b1)|((inp.bits.b3)<<1)|((inp.bits.b0)<<4)|((inp.bits.b2)<<5));
	txbuf[5] = 0xC0&((inp.bits.b5<<6)|((inp.bits.b4)<<7));
	txbuf[6] = 0xCF&((inp.bits.b8)|((inp.bits.b9)<<1)|((inp.bits.b10)<<2)|((inp.bits.b11)<<3)|((inp.bits.b6)<<6)|((inp.bits.b7)<<7));
	// txbuf[4] = inp.byte[0];
	// txbuf[5] = inp.byte[1];
	p->write(txbuf,9);
}
void HAPPYBONZ::solon(){
	// if(happy.sen == 4){ pinMode(conRELAY[3], OUTPUT); digitalWrite(conRELAY[3], 1);}
	
	digitalWrite(conRELAY[happy.sen-1], 1);
	uint8_t txbuf[]={'A','C','K',0xE0,happy.sen,happy.num,0,0,EOT};
	p->write(txbuf,9);
}
void HAPPYBONZ::soloff(){
	digitalWrite(conRELAY[happy.sen-1], 0);
	delay(10);
	uint8_t txbuf[]={'A','C','K',0xE0,happy.sen,happy.num,0,0,EOT};
	p->write(txbuf,9);
}
void HAPPYBONZ::soltoggle(){
	digitalWrite(conRELAY[happy.sen-1], 1);
	delay(soltime);
	digitalWrite(conRELAY[happy.sen-1], 0);
	uint8_t txbuf[]={'A','C','K',0xE0,happy.sen,happy.num,0,0,EOT};
	p->write(txbuf,9);
}
void HAPPYBONZ::motor(uint8_t sen){
	if(sen == 0) smotor.all = 0;
	motors = (sen>>4);
	cwccw = (sen&0xf);
	// p->printf("sen : %02x = motor : %02x , cwccw : %02x\r\n", sen, motors, cwccw);
	if(motors == 0xf){
		switch(cwccw){
			case 1: smotor.bits.pickup1		= 0; break;
			case 2: smotor.bits.pickup2		= 0; break;
			case 3: smotor.bits.pickup3		= 0; break;
			case 4: smotor.bits.updown		= 0; break;
			case 5: smotor.bits.ready		= 0; break;
			case 6: smotor.bits.upcheck		= 0; break;
			case 7: smotor.bits.pickup4		= 0; break;
		}
	}else if(motors > 0 && motors < 8){
		
		switch(motors){
			case 1: smotor.bits.pickup1		= 1; cw.bits.pickup1	= (cwccw==1)?1:0; break;
			case 2: smotor.bits.pickup2		= 1; cw.bits.pickup2	= (cwccw==1)?1:0; break;
			case 3: smotor.bits.pickup3		= 1; cw.bits.pickup3	= (cwccw==1)?1:0; break;
			case 4: smotor.bits.updown		= 1; cw.bits.updown		= (cwccw==1)?1:0; break;
			case 5: smotor.bits.ready		= 1; cw.bits.ready		= (cwccw==1)?1:0; break;
			case 6: smotor.bits.upcheck		= 1; cw.bits.upcheck	= (cwccw==1)?1:0; break;
			case 7: smotor.bits.pickup4		= 1; cw.bits.pickup4	= (cwccw==1)?1:0; break;
		}
	}
	
}
float HAPPYBONZ::randomDouble(double minf, double maxf)
{
	return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31); 
}
// #define __LOADCELL_TEST__
void HAPPYBONZ::loadcellProc(ModbusTCPServer *mb, int i){
	#ifdef __LOADCELL_TEST__
	for(int i = 0; i < ; i++){
		readloadcell[i].fval = randomDouble(0.1,20.00);
		mb->inputRegisterWrite((int)(i*2), readloadcell[i*2].u16[0]);
		mb->inputRegisterWrite((int)(i*2+1), readloadcell[i*2].u16[1]);
	}
	#else
	// for (int i=0; i<16; i++){
		if (scale[i].is_ready()) {
			readloadcell[i].fval = (scale[i].get_units(10)*4);
			mb->inputRegisterWrite(i*2, readloadcell[i].u16[0]);
			mb->inputRegisterWrite(i*2+1, readloadcell[i].u16[1]);
			#ifdef __DEBUG__
			Serial.println("Loadcell ["+ String(i)+"]: "+String(readloadcell[i].fval));
			#endif
		} else {
			// mb->inputRegisterWrite(i*2, 0);
			// mb->inputRegisterWrite(i*2+1, 0);
			#ifdef __DEBUG__
			// Serial.println(" HX711 not found.");
			#endif
		}
		// delay(65);
	// }		
	#endif
	// Serial.println();
}
void HAPPYBONZ::relayProc(ModbusTCPServer *mb){
	for(int i = 0; i < 7;i++){
		digitalWrite(conRELAY[i], mb->coilRead(i));
	}
}
void HAPPYBONZ::response(){
	// uint8_t txbuf[]={'A','C','K',0xd0,happy.sen,happy.num,0,0,EOT};
	// sol_Response();
	motor_Response();
}
void HAPPYBONZ::setInput(ModbusTCPServer *mb, uint16_t x){
	inp.u16 = x;
	for(int i = 0; i < 16; i++){
		mb->discreteInputWrite(i, (x>>i)&1);
	}
	// Serial.println(inp.u16,HEX);
}
void HAPPYBONZ::sol_Response(){
	
}
void HAPPYBONZ::motor_Response(){
			// p->printf("%02X\r\n",run_flag.all);
			// delay(100);
	for(int i = 0; i < 8;i++){
		if(((run_flag.all>>i)&1)==1){
			uint8_t txbuf[]={'A','C','K',0xE0,happy.sen,happy.num,0,0,EOT};
			p->write(txbuf,9);	
			run_flag.all &=(0xff^(1<<i));
		}
	}
}

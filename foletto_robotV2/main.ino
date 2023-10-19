#include <Arduino.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <config.h>
#include <TimerOne.h>
#include <ArduinoRS485.h>
#include <ArduinoModbus.h>
#include "HX711.h"
#include <happybonz.h>

extern HAPPYBONZ hp;
#define dbg		Serial
// put function declarations here:
void Task();
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x54
};
uint8_t rxbuf[sizeof(PACKET_ts)]={0};
PACKET_ts network;
// #define __ELTECH__
#define __HOMEnEXZEN__
#ifdef __HOMEnEXZEN__
// IPAddress serverIP(192,168,0,6);
IPAddress ip(192,168,0,121);
IPAddress subnet(255,255,255,0);
IPAddress gateway(192,168,0,1);
IPAddress mydns(8,8,8,8);
#endif
#ifdef __ELTECH__
// IPAddress serverIP(192,168,20,107);
IPAddress ip(192,168,20,121);
IPAddress subnet(255,255,255,0);
IPAddress gateway(192,168,20,1);
IPAddress mydns(8,8,8,8);
#endif
EthernetClient client;
extern uni_conv readloadcell[16];
// uni_conv stepPos[8];
extern HX711 scale[16];
long lsVal[9]={0};
const unsigned int bufSize = 256;
byte buf[bufSize];

bool _1s;
unsigned long TimeAct, TimePrev, HodingResult, InputResult, HeartBeat, i, StartingAddr;
long Cmd;
EthernetServer server(502);
ModbusTCPServer modbusTCPServer;
// ModbusTCPClient modbusTCPClient;

// ModbusRTUSlave modbus(Serial, buf, bufSize);
void etherLogin(HardwareSerial *_p){
	// pinMode(75, OUTPUT);
	// digitalWrite(75,1);
	// Serial.begin(115200);
	// while (!_p) {
	// ; 
	// }
	PACKET_ts temp;
	eeprom_read_block((void*)(&temp), (void*)0, sizeof(PACKET_ts));
	Ethernet.init(53);
	mac[5]= temp.mac;
	network = temp;
	ip.operator[](0) = temp.svr[0];
	ip.operator[](1) = temp.svr[1];
	ip.operator[](2) = temp.svr[2];
	ip.operator[](3) = temp.svr[3];

	gateway.operator[](0) = temp.gateway[0];
	gateway.operator[](1) = temp.gateway[1];
	gateway.operator[](2) = temp.gateway[2];
	gateway.operator[](3) = temp.gateway[3];

	subnet.operator[](0) = temp.subnet[0];
	subnet.operator[](1) = temp.subnet[1];
	subnet.operator[](2) = temp.subnet[2];
	subnet.operator[](3) = temp.subnet[3];
	#ifdef __DEBUG__
	_p->printf("\r\nEthernet : %d.%d.%d.%d\r\n", ip.operator[](0),ip.operator[](1),ip.operator[](2),ip.operator[](3));
	_p->printf("Mac : %d\r\n",mac[5]);
	_p->printf("subnetMask : %d.%d.%d.%d\r\n", subnet.operator[](0),subnet.operator[](1),subnet.operator[](2),subnet.operator[](3));
	_p->printf("gateway : %d.%d.%d.%d\r\n", gateway.operator[](0),gateway.operator[](1),gateway.operator[](2),gateway.operator[](3));
	_p->flush();
	#endif
	Ethernet.begin(mac, ip, gateway, subnet);
	// start listening for clients
	//server.begin();
	// Open serial communications and wait for port to open:
	// EthServer.begin();
	server.begin();
	modbusTCPServer.begin();
	// modbusTCPClient.begin(serverIP, 502);
	if (!modbusTCPServer.begin()) {
		// #ifdef __DEBUG__
		Serial.print("Failed to start Modbus TCP Server!\r");
		// #endif
		// while (1);
	}
	InputResult = modbusTCPServer.configureDiscreteInputs(0, 16);
	HodingResult = modbusTCPServer.configureHoldingRegisters(0, 100);
	InputResult = modbusTCPServer.configureInputRegisters(0,100);
	modbusTCPServer.configureCoils(0,16);								
	#ifdef __DEBUG__
	// modbusTCPServer.
	_p->print("Chat server address:");
	// Serial.println(Ethernet.localIP());	
	_p->print("Holding Reg init result =");
	_p->print(HodingResult);
	_p->print("\n");

	_p->print("Input Reg init result =");
	_p->print(InputResult);
	_p->print("\n");

	_p->print("Modbus server address=");
	_p->println(Ethernet.localIP());
	_p->print("\n");
	_p->print(ip.operator[](0)+String(","));
	_p->print(ip.operator[](1)+String(","));
	_p->print(ip.operator[](2)+String(","));
	_p->println(ip.operator[](3));
	#endif	
}
void relayInit(){
	for(int i = 0; i < (int)sizeof(conRELAY);i++){
		pinMode(conRELAY[i], OUTPUT);
		#ifdef __DEBUG__
		digitalWrite(conRELAY[i], 1);
		delay(50);
		digitalWrite(conRELAY[i], 0);
		delay(50);
		#endif
	}
	#ifdef __DEBUG__
	Serial.println("Relay Start");
	#endif
}
void stepmotorInit(){
	for(int i = 0; i < 10;i++){
		pinMode(stepMotor[i].DIR,OUTPUT);
		pinMode(stepMotor[i].PWM,OUTPUT);
	}
	pinMode(dfEN, OUTPUT);
	pinMode(dfSLEEP, OUTPUT);
	digitalWrite(dfSLEEP, HIGH);
	digitalWrite(dfEN, LOW);
	#ifdef __DEBUG__
	Serial.println("STEP Motor Start");
	#endif	
}
void loadcellInit(){
	for (int i=0; i<16; i++){
		scale[i].begin(LOADCELL[i].DOUT, LOADCELL[i].SCKD);
		// scale[i].read();
		// scale[i].read_average(5);
		// scale[i].get_value(5);
		// scale[i].get_units(5);
		scale[i].set_scale(2280.f);
		// scale[i].tare();
		// scale[i].read();
		// scale[i].read_average(5);
		// scale[i].get_value(5);
		// scale[i].get_units(5);
		#ifdef __DEBUG__
		Serial.println("Loadcell Channel ["+String(i)+"] Setup");
		#endif
		delay(100);

	}
	// digitalWrite(LOADCELL[i].SCKD, LOW);
	#ifdef __DEBUG__
	Serial.println("Loadcell Start");
	#endif	
}
void shiftregisterInit(){
	pinMode(shiftResister.MISO,INPUT_PULLUP);
	pinMode(shiftResister.MOSI,OUTPUT);
	pinMode(shiftResister.CLK,OUTPUT);
	pinMode(shiftResister.PL,OUTPUT);
	pinMode(shiftResister.CE,OUTPUT);
	#ifdef __DEBUG__
	Serial.println("Shift Start");
	#endif	
}

void shiftRead(){
	uint32_t shiftIn=0;
	digitalWrite(shiftResister.CE, 1);
	digitalWrite(shiftResister.MOSI,0);
	digitalWrite(shiftResister.PL, 0);
	delayMicroseconds(5);
	digitalWrite(shiftResister.PL, 1);
	digitalWrite(shiftResister.CE, 0);
	for(int i = 0; i < 16;i++){
		uint32_t bitVal =digitalRead(shiftResister.MISO);
		shiftIn |=(bitVal<<((16-1)-SenSeq[i]));
		digitalWrite(shiftResister.CLK,1);
		delayMicroseconds(5);
		digitalWrite(shiftResister.CLK,0);
	}
	// dbg.println(shiftIn,HEX);
	// client.println(shiftIn,HEX);
	hp.shiftInput = shiftIn;
	#ifdef __DEBUG__
	// Serial.println(hp.shiftInput);
	#endif
	hp.setInput(&modbusTCPServer,(uint16_t)shiftIn);
}
// uint32_t old_t= 0, new_t = 0;
void timerInit(){
	Timer1.initialize(500); 
	Timer1.attachInterrupt(Task);	
}
void setup() {
	Serial.begin(115200);


	// Serial1.begin(9600);
	// Serial2.begin(9600);
	etherLogin(&Serial);
	shiftregisterInit();
	stepmotorInit();
	loadcellInit();
	timerInit();
	relayInit();
	hp.begin(&Serial);
	hp.run_flag.all = 0;
	#ifdef __DEBUG__
	Serial.println("Setup Finished.");
	#endif
}
int connected = 0;
uint16_t x = 1;
void modbus_proc(){
	// modbusTCPClient.
	EthernetClient client = server.available();

	
	if (client) {
	// a new client connected
	#ifdef __DEBUG__
	Serial.println("new client");
	#endif


	modbusTCPServer.accept(client);

	while (client.connected()) {
		hp.parsingSetup("String");
		hp.cmdParsor();
		
		
		shiftRead();
		if(++i>=16) i = 0;
		hp.loadcellProc(&modbusTCPServer, i);
		
		modbusTCPServer.poll();
		
		hp.relayProc(&modbusTCPServer);

	}
	#ifdef __DEBUG__
	Serial.println("client disconnected");
	#endif
	}	
}
void cmdParsor(){
	hp.parsingSetup("String");
	hp.cmdParsor();
}
void loop() {
	cmdParsor();
	modbus_proc();
}
int b = 0;
void oldTask(){
	b ^= 1;
	// for(int i = 0; i< 8;i++){
	for(int i = 0; i< 2;i++){
		// int ccw = ((hp.cw.all>>i)&1);
		digitalWrite(stepMotor[i].DIR, ((hp.cw.all>>i)&1));
		// digitalWrite(stepMotor[i].PWM, b);
		if(((hp.smotor.all>>i)&1)){
			int c = ((hp.inp.u16>>(i*2))&3);
			// digitalWrite(conRELAY[8], (c>>1));
			// digitalWrite(conRELAY[9], (c&1));
			if(((hp.cw.all>>i)&1) == 0){
				if(c!=1)digitalWrite(stepMotor[i].PWM, b);
				else hp.smotor.all &=(0xff^(1<<i)), hp.run_flag.all |= (1<<i);
			}else{
				if(c!=2)digitalWrite(stepMotor[i].PWM, b);
				else hp.smotor.all &=(0xff^(1<<i)), hp.run_flag.all |= (1<<i);
			}
		}
	}
}
void Task(){
	hp.stepmotorTask(&modbusTCPServer);
	return;
	oldTask();
	// dbg.println();
}


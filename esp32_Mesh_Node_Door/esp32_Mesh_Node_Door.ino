#define PCA9539_H_
#include "Arduino.h"
#include "Wire.h"

//#define DEBUG 1
#define NXP_INPUT      0
#define NXP_CONFIG     6

class PCA9539 {
  public:
    PCA9539(uint8_t address);                            // constructor
    void pinMode(uint8_t pin);          // pinMode
    uint8_t digitalRead(uint8_t pin);                    // digitalRead
    void digitalWrite(uint8_t pin, uint8_t value );      // digitalWrite

  private:
    uint16_t I2CGetValue(uint8_t address, uint8_t reg);
    void I2CSetValue(uint8_t address, uint8_t reg, uint8_t value);

    union {
      struct {
        uint8_t _configurationRegister_low;          // low order byte
        uint8_t _configurationRegister_high;         // high order byte
      };
      uint16_t _configurationRegister;                 // 16 bits presentation
    };
    uint8_t _address;                                    // address of port this class is supporting
    int _error;                                          // error code from I2C
};

PCA9539::PCA9539(uint8_t address) {
  _address         = address;        // save the address id
  Wire.begin();                      // start I2C communication
}

void PCA9539::pinMode(uint8_t pin) {
  if (pin <= 15) {
    _configurationRegister = _configurationRegister | (1 << pin);
    I2CSetValue(_address, NXP_CONFIG    , _configurationRegister_low);
    I2CSetValue(_address, NXP_CONFIG + 1, _configurationRegister_high);
  }
}

uint8_t PCA9539::digitalRead(uint8_t pin) {
  uint16_t _inputData = 0;
  if (pin > 15 ) return 255;
  _inputData  = I2CGetValue(_address, NXP_INPUT);
  _inputData |= I2CGetValue(_address, NXP_INPUT + 1) << 8;
  if ((_inputData & (1 << pin)) > 0) {
    return HIGH;
  } else {
    return LOW;
  }
}

uint16_t PCA9539::I2CGetValue(uint8_t address, uint8_t reg) {
  uint16_t _inputData;
  Wire.beginTransmission(address);          // setup read registers
  Wire.write(reg);
  _error = Wire.endTransmission();
  if (Wire.requestFrom((int)address, 1) != 1)
  {
    return 256;                            // error code is above normal data range
  };
  _inputData = Wire.read();
  return _inputData;
}

void PCA9539::I2CSetValue(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);              // setup direction registers
  Wire.write(reg);                              // pointer to configuration register address 0
  Wire.write(value);                            // write config register low byte
  _error = Wire.endTransmission();
}
///mux_Lib

#include  "painlessMesh.h"
#define   SERIAL_MAX  64

#define   MESH_PREFIX     "smartHiveMesh"
#define   MESH_PASSWORD   "smarthive123"
#define   MESH_PORT       3333

Scheduler taskScheduler; // to control upload task
painlessMesh  mesh;

String nodeID = "";
enum DOOR_LOACTION {CLOSE = 0, OPEN_1T, OPEN_2T, OPEN}; //1:열림, 2:2/3, 3:1/3 ,4:닫힘
//// ----------- Flage --------------
boolean door_able[4]      = {true,true,true,true};
boolean door_moter[4]     = {false,false,false,false};
boolean door_direction[4] = {false,false,false,false}; //true = 열림, false = 닫힘
//// ----------- Variable -----------
uint8_t door_location[4]  = {4, 4, 4, 4};
////for millis() func//
unsigned long time_door   = 0;
uint16_t time_attack      = 0;
uint16_t door_runtime[4]  = {0,0,0,0};
//// ------------- PIN --------------
//// ------------- OUTPUT -----------
const uint8_t moter_F[4]  = {4, 17, 18, 23};
const uint8_t moter_B[4]  = {16, 15, 19, 13};
//// ------------- INPUT ------------
const uint8_t location[4][4]  = {{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}};

//// ----------- Command  -----------
void command_Service(String command, String value) {

  int8_t control = 0;
  if (value.toInt() > 255) {
    control = 255;
  } else {
    control = value.toInt();
  }
  Serial.print("command_Service:");
  Serial.print(command);
  Serial.print(":");
  Serial.print(value);
  Serial.print(":");
  Serial.println(control);
  if (command.equals("AT+OPEN")) {
    Serial.println("AT+OPEN");
    moterLotation(control, OPEN);
    mesh.sendBroadcast("DOOR=SET=OPEN;");
  } else if (command.equals("AT+OPEN_2T")) {
    Serial.println("AT+OPEN_2T");
    moterLotation(control, OPEN_2T);
    mesh.sendBroadcast("DOOR=SET=OPEN_2T;");
  } else if (command.equals("AT+OPEN_1T")) {
    Serial.println("AT+OPEN_1T");
    moterLotation(control, OPEN_1T);
    mesh.sendBroadcast("DOOR=SET=OPEN_1T;");
  } else if (command.equals("AT+CLOSE")) {
    Serial.println("AT+CLOSE");
    moterLotation(control, CLOSE);
    mesh.sendBroadcast("DOOR=SET=CLOSE;");
  } else if (command.equals("AT+ATTACK")) {
    Serial.println("AT+ATTACK");
    time_attack = control;
    mesh.sendBroadcast("DOOR=SET=ATTACK;");
  }
}//Command_service() END
//// ----------- TEST  -----------
void AT_commandHelp() {
  Serial.println("------------- AT command help ----------------");
  Serial.println(";AT+OPEN=int;         [int] Door Full Open");
  Serial.println(";AT+OPEN_2T=int;      [int] Door 2/3 Open");
  Serial.println(";AT+OPEN_1T=int;      [int] Door 1/3 Open");
  Serial.println(";AT+CLOSE=int;        [int] Door Close");
  Serial.println(";AT+ATTACK=int;       All Door Close during[int] ");
}
char Serial_buf[SERIAL_MAX];
int8_t Serial_num;
void Serial_service() {
  String str1 = strtok(Serial_buf, "=");
  String str2 = strtok(NULL, ";");
  command_Service(str1, str2);
}
void Serial_process() {
  char ch;
  ch = Serial.read();
  switch ( ch ) {
    case ';':
      Serial_buf[Serial_num] = ';';
      Serial_service();
      Serial_num = 0;
      break;
    default :
      Serial_buf[ Serial_num ++ ] = ch;
      Serial_num %= SERIAL_MAX;
      break;
  }
}
//// ----------- TEST  -----------

//taskSendMessage funtion
void sendMessage() ; // Prototype so PlatformIO doesn't complain
Task doorLog( TASK_SECOND * 60, TASK_FOREVER, &doorValue );
void doorValue() {
  String msg = "DOOR=SET=" + String(door_location[0]) + String(door_location[1]) + String(door_location[2]) + String(door_location[3]) + ';' ;
  mesh.sendBroadcast( msg );
}
//taskSendMessage funtion end


// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  char msg_buf[SERIAL_MAX];
  for (int i = 0; i < msg.length(); i++) {
    msg_buf[i] = msg[i];
  }
  String types   = strtok(msg_buf, "=");
  if (types == "D") {
    String device  = strtok(NULL, "=");
    if (device == nodeID) {
      Serial.println(msg);
      String command = strtok(NULL, "=");
      String value   = strtok(NULL, ";");
      command_Service(command, value);
    }
  }
}
///////

PCA9539 ioport(0x76); // Base address starts at 0x74 for A0 = L and A1 = L
//Address   A1    A0
//0x74      L     L
//0x75      L     H
//0x76      H     L
//0x77      H     H

void setup()
{
  Serial.begin(115200);
  for (int8_t index = 0; index < 4; index++) {
    pinMode(moter_F[index], OUTPUT);
    digitalWrite(moter_F[index], false);
    pinMode(moter_B[index], OUTPUT);
    digitalWrite(moter_B[index], false);
    ioport.pinMode(location[OPEN][index]);
    ioport.pinMode(location[OPEN_2T][index]);
    ioport.pinMode(location[OPEN_1T][index]);
    ioport.pinMode(location[CLOSE][index]);
  }
  //// ------------ Door init location ------------
  for (int8_t index = 0; index < 4; index++) {
    door_location[index] = doorLocation(index);
  }

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &taskScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);

  nodeID = mesh.getNodeId();

  taskScheduler.addTask( doorLog );
  doorLog.enable();

  Serial.print("DoorID : ");
  Serial.println(nodeID);
  if (door_location[0] == 4 || door_location[1] == 4 || door_location[2] == 4 || door_location[3] == 4 ) {
    Serial.print(", Can not found the Door location.");
  } else {
    Serial.print(", System all green.");
  }
  for (int8_t index = 0; index < 4; index++) {
    Serial.print(door_location[index]);
    if (index == 3) {
      Serial.println('.');
    } else {
      Serial.print(',');
    }
  }
  AT_commandHelp();
}

void loop()
{
  mesh.update();
  moterRun();
  if (Serial.available()) {
    Serial_process();
  }
}

int8_t doorLocation(int8_t DOOR) {
  int8_t nowDoorLocation = 4;
  Serial.print(DOOR);
  Serial.print(" - DOOR Location is ");
  if (!ioport.digitalRead(location[DOOR][OPEN_2T])) {
    Serial.println("OPEN_2T");
    nowDoorLocation = OPEN_2T;
  } else if (!ioport.digitalRead(location[DOOR][OPEN_1T])) {
    Serial.println("OPEN_1T");
    nowDoorLocation = OPEN_1T;
  } else if (!ioport.digitalRead(location[DOOR][OPEN])) {
    Serial.println("OPEN");
    nowDoorLocation = OPEN;
  } else if (!ioport.digitalRead(location[DOOR][CLOSE])) {
    Serial.println("CLOSE");
    nowDoorLocation = CLOSE;
  } else {
    Serial.println("Not Found");
  }
  return nowDoorLocation;
}//doorLocation() END

void moterLotation(int8_t MOTER, int8_t LOCATION) {
  int8_t nowDoorLocation = doorLocation(MOTER);
  Serial.print("MovingTest: ");
  Serial.print(MOTER);
  Serial.print(" moter move to ");
  Serial.println(LOCATION);
  door_runtime[MOTER] = 0;
  if (LOCATION == nowDoorLocation) {
    door_moter[MOTER] = false;
  } else if(door_able[MOTER]){
    door_moter[MOTER] = true;
    if (LOCATION > nowDoorLocation) {
      door_direction[MOTER] = false;
    } else if (LOCATION < nowDoorLocation) {
      door_direction[MOTER] = true;
    }
    door_location[MOTER]  = LOCATION;
  }
  if (door_moter[MOTER]) {
    Serial.print("Move to ");
    if (door_direction[MOTER]) {
      Serial.println("Front");
      digitalWrite(moter_F[MOTER], true);
    } else {
      Serial.println("Backyard");
      digitalWrite(moter_B[MOTER], true);
    }
  }
}//moterLotation() END

void moterRun() {
  if ((millis() - time_door) > 10) {
    time_door = millis();
    for (int index = 0; index < 4; index++) {
      if (door_moter[index]) {
        if (door_runtime[index] < 100 * 18) { //5초
          if (door_location[index] == doorLocation(index)) {
            digitalWrite(moter_F[index], false);
            digitalWrite(moter_B[index], false);
            door_moter[index] = false;
            Serial.println("Moved");
          }
          door_runtime[index]++;
        } else {
          digitalWrite(moter_F[index], false);
          digitalWrite(moter_B[index], false);
          door_moter[index] = false;
          door_able[index]  = false;
          Serial.println("Out of order!");
        }
      }
    }//for
  }//millis()
}//moterRun() END

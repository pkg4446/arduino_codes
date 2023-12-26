#include "Arduino.h"
#include "Wire.h"

#define DEBUG 1
#define NXP_INPUT      0
#define NXP_OUTPUT     2
#define NXP_INVERT     4
#define NXP_CONFIG     6


/******************************************* PCA9555 LIB *******************************************/
class PCA9555 {
  public:
    PCA9555(uint8_t address, int interruptPin = -1);     // optional interrupt pin in second argument
    void pinMode(uint8_t pin, uint8_t IOMode );          // pinMode
    uint8_t digitalRead(uint8_t pin);                    // digitalRead
    void digitalWrite(uint8_t pin, uint8_t value );      // digitalWrite
    uint8_t stateOfPin(uint8_t pin);                     // Actual ISR
    void setClock(uint32_t clockFrequency);              // Clock speed
    bool begin();                                        // Checks if PCA is responsive

  private:
    static PCA9555* instancePointer;
    static void alertISR(void); // Function pointing to actual ISR
    void pinStates();           // Function tied to interrupt
    uint16_t I2CGetValue(uint8_t address, uint8_t reg);
    void I2CSetValue(uint8_t address, uint8_t reg, uint8_t value);

    union {
      struct {
        uint8_t _stateOfPins_low;          // low order byte
        uint8_t _stateOfPins_high;         // high order byte
      };
      uint16_t _stateOfPins;                 // 16 bits presentation
    };
    union {
      struct {
        uint8_t _configurationRegister_low;          // low order byte
        uint8_t _configurationRegister_high;         // high order byte
      };
      uint16_t _configurationRegister;                 // 16 bits presentation
    };
    union {
      struct {
        uint8_t _valueRegister_low;                  // low order byte
        uint8_t _valueRegister_high;                 // high order byte
      };
      uint16_t _valueRegister;
    };
    uint8_t _address;                                    // address of port this class is supporting
    int _error;                                          // error code from I2C
};

PCA9555* PCA9555::instancePointer = 0;
PCA9555::PCA9555(uint8_t address, int interruptPin) {
  _address         = address;        // save the address id
  _valueRegister   = 0;
  Wire.begin();                      // start I2C communication
  if (interruptPin >= 0) {
    instancePointer = this;
    attachInterrupt(digitalPinToInterrupt(interruptPin), PCA9555::alertISR, false); // Set to low for button presses
  }
}

bool PCA9555::begin() {
  Wire.beginTransmission(_address);
  Wire.write(0x02); // Test Address
  _error = Wire.endTransmission();

  if (_error != 0) {
    return false;
  } else {
    return true;
  }
}

void PCA9555::pinMode(uint8_t pin, uint8_t IOMode) {
  if (pin <= 15) {
    if (IOMode == OUTPUT) {
      _configurationRegister = _configurationRegister & ~(1 << pin);
    } else {
      _configurationRegister = _configurationRegister | (1 << pin);
    }
    I2CSetValue(_address, NXP_CONFIG    , _configurationRegister_low);
    I2CSetValue(_address, NXP_CONFIG + 1, _configurationRegister_high);
  }
}

uint8_t PCA9555::digitalRead(uint8_t pin) {
  uint16_t _inputData = 0;
  if (pin > 15 ) return 255;
  _inputData  = I2CGetValue(_address, NXP_INPUT);
  _inputData |= I2CGetValue(_address, NXP_INPUT + 1) << 8;
  if ((_inputData & (1 << pin)) > 0) {
    return true;
  } else {
    return false;
  }
}

void PCA9555::digitalWrite(uint8_t pin, uint8_t value) {
  if (pin > 15 ) {
    _error = 255;            // invalid pin
    return;                  // exit
  }
  if (value > 0) {
    _valueRegister = _valueRegister | (1 << pin);    // and OR bit in register
  } else {
    _valueRegister = _valueRegister & ~(1 << pin);    // AND all bits
  }
  I2CSetValue(_address, NXP_OUTPUT    , _valueRegister_low);
  I2CSetValue(_address, NXP_OUTPUT + 1, _valueRegister_high);
}
void PCA9555::pinStates() {
  _stateOfPins = I2CGetValue(_address, NXP_INPUT);
  _stateOfPins |= I2CGetValue(_address, NXP_INPUT + 1) << 8;
}

// Returns to user the state of desired pin
uint8_t PCA9555::stateOfPin(uint8_t pin) {
  if ((_stateOfPins & (1 << pin)) > 0) {
    return true;
  } else {
    return false;
  }
}

void PCA9555::setClock(uint32_t clockFrequency) {
  Wire.setClock(clockFrequency);
}

void PCA9555::alertISR()
{
  if (instancePointer != 0) {
    instancePointer->pinStates(); // Points to the actual ISR
  }
}

uint16_t PCA9555::I2CGetValue(uint8_t address, uint8_t reg) {
  uint16_t _inputData;
  Wire.beginTransmission(address);          // setup read registers
  Wire.write(reg);
  _error = Wire.endTransmission();
  if (Wire.requestFrom((int)address, 1) != 1) {
    return 256;                            // error code is above normal data range
  };
  _inputData = Wire.read();
  return _inputData;
}

void PCA9555::I2CSetValue(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);              // setup direction registers
  Wire.write(reg);                              // pointer to configuration register address 0
  Wire.write(value);                            // write config register low byte
  _error = Wire.endTransmission();
}
//// PCA955 library ////
PCA9555 ioport(0x20);
/******************************************* PCA9555 END *******************************************/

HardwareSerial nxSerial(2);
#define RX2 18
#define TX2 19

#define PWM1 0
#define PWM2 1
#define PIN_PWM1 2
#define PIN_PWM2 15

//#define DEBUG_MONIT

/******************************************* STEP  MOTOR *******************************************/
#define STEP_NUM  3

typedef struct STEP_ts{
	int8_t   ENA;
	int8_t   DIR;
	int8_t   PUL;
}STEP_ts;
//uint8_t pin_out[6] = {4, 26, 27, 14, 12, 13};
const STEP_ts step_under[STEP_NUM]={
   {15,9,4},
   {14,8,26},
   {13,0,27},
};
const STEP_ts step_upper[STEP_NUM]={
   {12,1,14},
   {11,2,12},
   {10,3,13},
};

bool      NextStep_upper              = false;
bool      NextStep_under              = false;
uint32_t  Target_pos_upper[STEP_NUM]  = {0,};
uint32_t  Target_pos_under[STEP_NUM]  = {0,};

uint32_t  Position_upper[STEP_NUM]    = {0,};
uint32_t  Position_under[STEP_NUM]    = {0,};
bool      Direction_upper[STEP_NUM]   = {false,false,false};
bool      Direction_under[STEP_NUM]   = {false,false,false};
bool      Activation_upper[STEP_NUM]  = {false,false,false};
bool      Activation_under[STEP_NUM]  = {false,false,false};
uint16_t Interval_upper               = 1000;
uint16_t Interval_under               = 1000;
unsigned long Update_upper            = 0UL;
unsigned long Update_under            = 0UL;

void postion_cal_upper(){
  if(NextStep_upper){
    NextStep_upper = false;
  }
  
}

void postion_cal_under(){
  if(NextStep_under){
    NextStep_under = false;
  }
}

void moter_run(unsigned long millisec){

  if(millisec - Update_upper > Interval_upper){
    Update_upper = millisec;
    NextStep_upper    = true;
    for (uint8_t index = 0; index < STEP_NUM; index++){ 
      if(Position_upper[index] != Target_pos_upper[index]){
        if(!Activation_upper[index]){
          Activation_upper[index] = true;
          ioport.digitalWrite(step_upper[index].ENA, true);
        }
        if(Position_upper[index] > Target_pos_upper[index]){
          if(!Direction_upper[index]){
            Direction_upper[index] = true;
            ioport.digitalWrite(step_upper[index].DIR, true);
          }
        }else{
          if(Direction_upper[index]){
            Direction_upper[index] = false;
            ioport.digitalWrite(step_upper[index].DIR, false);
          }
        }
        digitalWrite(step_upper[index].PUL, true);
        if(Direction_upper[index]){
          Position_upper[index] += 1;
        }else{
          if(Position_upper[index]>0) Position_upper[index] -= 1;
        }
        digitalWrite(step_upper[index].PUL, false);
      }
    }
  }

  if(millisec - Update_under > Interval_under){
    Update_under = millisec;
    NextStep_under    = true;
    for (uint8_t index = 0; index < STEP_NUM; index++){
      if(Position_under[index] != Target_pos_under[index]){
        if(!Activation_under[index]){
          Activation_under[index] = true;
          ioport.digitalWrite(step_under[index].ENA, true);
        }
        if(Position_under[index] > Target_pos_under[index]){
          if(!Direction_under[index]){
            Direction_under[index] = true;
            ioport.digitalWrite(step_under[index].DIR, true);
          }
        }else{
          if(Direction_under[index]){
            Direction_under[index] = false;
            ioport.digitalWrite(step_under[index].DIR, false);
          }
        }
        digitalWrite(step_under[index].PUL, true);
        if(Direction_under[index]){
          Position_under[index] += 1;
        }else{
          if(Position_under[index]>0) Position_under[index] -= 1;
        }
        digitalWrite(step_under[index].PUL, false);
      }
    }
  }
}

void moter_sleep(unsigned long millisec){
  for (uint8_t index = 0; index < STEP_NUM; index++){ 
    if(Activation_upper[index] && (millisec - Update_upper > 10)){
      Activation_upper[index] = false;
      ioport.digitalWrite(step_upper[index].ENA, false);
    }
    if(Activation_under[index] && (millisec - Update_under > 10) ){
      Activation_under[index] = false;
      ioport.digitalWrite(step_under[index].ENA, false);
    }
  }
}
/******************************************* STEP  MOTOR *******************************************/
/******************************************* SERIAL PROCESS ****************************************/
#define SERIAL_MAX  8
char Serial_buf[SERIAL_MAX];
int16_t Serial_num;
void Serial_process(char ch) {
  if(ch==0x03){
    Serial_buf[Serial_num] = 0x00;
    Serial.println(Serial_buf);
    command_pros();
    Serial_num = 0;
  }else if(ch==0x02){
    Serial_num = 0;
  }else{
    Serial_buf[ Serial_num++ ] = ch;
    Serial_num %= SERIAL_MAX;
  }
}
//0x02 [0][1][2][3][n][n][n][n][0x00]
void command_pros(){
  if(Serial_buf[0] == 'W' && Serial_buf[1] == 'O' && Serial_buf[2] == 'R' && Serial_buf[3] == 'K'){

    if(Serial_buf[5] == 'O' && Serial_buf[6] == 'N'){}
    else if(Serial_buf[5] == 'F' && Serial_buf[6] == 'F'){}

  }else if(Serial_buf[0] == 'S' && Serial_buf[1] == 'Y' && Serial_buf[2] == 'N' && Serial_buf[3] == 'C'){

    if(Serial_buf[5] == 'O' && Serial_buf[6] == 'N'){}
    else if(Serial_buf[5] == 'F' && Serial_buf[6] == 'F'){}
    
  }else if(Serial_buf[0] == 'M' && Serial_buf[1] == 'N'){

    if(Serial_buf[3] == 'O' && Serial_buf[4] == 'F' && Serial_buf[4] == 'F'){
      //Manual_stop
    }else if(Serial_buf[3] == 'Z' && Serial_buf[4] == 'R'){
      if(Serial_buf[6] == 'T'){       //Zero_set = top
        for (uint8_t index = 0; index < STEP_NUM; index++){
          Target_pos_upper[index] = 0;
        }
      }else if(Serial_buf[6] == 'B'){ //Zero_set = bottom
        for (uint8_t index = 0; index < STEP_NUM; index++){
          Target_pos_under[index] = 0;
        }
      }
      Serial.println("Zero set");
    }else if(Serial_buf[3] == 'T' && Serial_buf[4] == 'X'){

    }else if(Serial_buf[3] == 'T' && Serial_buf[4] == 'Y'){

    }else if(Serial_buf[3] == 'T' && Serial_buf[4] == 'Z'){

    }else if(Serial_buf[3] == 'D' && Serial_buf[4] == 'X'){

    }else if(Serial_buf[3] == 'D' && Serial_buf[4] == 'Y'){

    }else if(Serial_buf[3] == 'D' && Serial_buf[4] == 'Z'){

    }//Manual_run

  }else if(Serial_buf[0] == 'C' && Serial_buf[1] == 'T'){

    if(Serial_buf[2] == 'S'){ //Top stepper moving speed change
      uint16_t buffer_num[2] = {Serial_buf[3],Serial_buf[4]};
      Interval_upper = buffer_num[1]*256 + buffer_num[0];
    }else if(Serial_buf[2] == 'X'){
      uint32_t buffer_num[4] = {Serial_buf[3],Serial_buf[4],Serial_buf[5],Serial_buf[6]};
      Target_pos_upper[0] = buffer_num[3]*256*256*256 + buffer_num[2]*256*256 + buffer_num[1]*256 + buffer_num[0];
    }else if(Serial_buf[2] == 'Y'){
      uint32_t buffer_num[4] = {Serial_buf[3],Serial_buf[4],Serial_buf[5],Serial_buf[6]};
      Target_pos_upper[1] = buffer_num[3]*256*256*256 + buffer_num[2]*256*256 + buffer_num[1]*256 + buffer_num[0];
    }else if(Serial_buf[2] == 'Z'){
      uint32_t buffer_num[4] = {Serial_buf[3],Serial_buf[4],Serial_buf[5],Serial_buf[6]};
      Target_pos_upper[2] = buffer_num[3]*256*256*256 + buffer_num[2]*256*256 + buffer_num[1]*256 + buffer_num[0];
    }//Top stepper target position change

  }else if(Serial_buf[0] == 'C' && Serial_buf[1] == 'B' && Serial_buf[2] == 'S'){

    if(Serial_buf[2] == 'S'){ //Bottom stepper moving speed change
      uint16_t buffer_num[2] = {Serial_buf[3],Serial_buf[4]};
      Interval_under = buffer_num[1]*256 + buffer_num[0];
    }else if(Serial_buf[2] == 'X'){
      uint32_t buffer_num[4] = {Serial_buf[3],Serial_buf[4],Serial_buf[5],Serial_buf[6]};
      Target_pos_under[0] = buffer_num[3]*256*256*256 + buffer_num[2]*256*256 + buffer_num[1]*256 + buffer_num[0];
    }else if(Serial_buf[2] == 'Y'){
      uint32_t buffer_num[4] = {Serial_buf[3],Serial_buf[4],Serial_buf[5],Serial_buf[6]};
      Target_pos_under[1] = buffer_num[3]*256*256*256 + buffer_num[2]*256*256 + buffer_num[1]*256 + buffer_num[0];
    }else if(Serial_buf[2] == 'Z'){
      uint32_t buffer_num[4] = {Serial_buf[3],Serial_buf[4],Serial_buf[5],Serial_buf[6]};
      Target_pos_under[2] = buffer_num[3]*256*256*256 + buffer_num[2]*256*256 + buffer_num[1]*256 + buffer_num[0];
    }//Bottom stepper target position change

  }else{
    Serial.print("Is not command: ");Serial.println(Serial_buf);
  }
}

void SerialConnect() {
  if (Serial.available()) {
    Serial_process(Serial.read());
  }
}
/******************************************* SERIAL PROCESS ****************************************/
/******************************************* NEXTION ***********************************************/
void DisplayConnect() {
  if (nxSerial.available()) {
    Serial_process(nxSerial.read());
  }
}

void send2Nextion(String cmd) {
  nxSerial.print(cmd);
  nxSerial.write(0xFF);
  nxSerial.write(0xFF);
  nxSerial.write(0xFF);
}

void Display(String IDs, uint16_t values) {
  String cmd;
  char buf[8] = { 0 };
  sprintf(buf, "%d", values);
  cmd = IDs + ".val=";
  cmd += buf;
  send2Nextion(cmd);
}
/******************************************* NEXTION ***********************************************/
uint8_t pin_in[3] =  {32, 33, 25};

/******************************************** S E T U P ********************************************/
void setup() {
  Serial.begin(115200);
  ioport.begin();
  ioport.setClock(400000);
  /*
  ledcSetup(PWM1, 5000, 8);
  ledcSetup(PWM2, 5000, 8);
  ledcAttachPin(PIN_PWM1, PWM1);
  ledcAttachPin(PIN_PWM2, PWM2);
  ledcWrite(PWM1, 50);
  ledcWrite(PWM2, 50);
  */

  for (uint8_t index = 0; index < STEP_NUM; index++) {
    ioport.pinMode(step_under[index].ENA, OUTPUT);
    ioport.digitalWrite(step_under[index].ENA, false);
    ioport.pinMode(step_under[index].DIR, OUTPUT);
    ioport.digitalWrite(step_under[index].DIR, false);
    pinMode(step_under[index].PUL, OUTPUT);
    ioport.pinMode(step_upper[index].ENA, OUTPUT);
    ioport.digitalWrite(step_upper[index].ENA, false);
    ioport.pinMode(step_upper[index].DIR, OUTPUT);
    ioport.digitalWrite(step_upper[index].DIR, false);
    pinMode(step_upper[index].PUL, OUTPUT);
  }

  for (uint8_t index = 0; index < 3; index++) {
    pinMode(pin_in[index], INPUT_PULLUP);
  }
  Serial.println("System all green");
}
/******************************************** S E T U P ********************************************/
#ifdef DEBUG_MONIT
  uint8_t index_led = 0;
  boolean led_flage = true;
#endif
/********************************************* L O O P *********************************************/
void loop() {
  unsigned long millisec = millis();
  SerialConnect();
  moter_run(millisec);
  moter_sleep(millisec);

  /*
    for (uint8_t index = 0; index < 3; index++) {
    Serial.print("limit sw ");
    Serial.print(index);
    Serial.print(": ");
    Serial.println(digitalRead(pin_in[index]));
    }
  */

  #ifdef DEBUG_MONIT
    if (led_flage) {
      ioport.digitalWrite(step_upper[index_led].ENA, true);
      ioport.digitalWrite(step_upper[index_led].DIR, true);
      ioport.digitalWrite(step_under[index_led].ENA, true);
      ioport.digitalWrite(step_under[index_led].DIR, true);
      delay(300);
      ioport.digitalWrite(step_upper[index_led].ENA, false);
      ioport.digitalWrite(step_upper[index_led].DIR, false);
      ioport.digitalWrite(step_under[index_led].ENA, false);
      ioport.digitalWrite(step_under[index_led].DIR, false);
      if (index_led < STEP_NUM) {
        index_led++;
      } else {
        index_led = 0;
        led_flage = false;
      }
    } else {
      digitalWrite(step_upper[index_led].PUL, true);
      digitalWrite(step_under[index_led].PUL, true);
      delay(300);
      digitalWrite(step_upper[index_led].PUL, false);
      digitalWrite(step_under[index_led].PUL, false);
      if (index_led < 6) {
        index_led++;
      } else {
        index_led = 0;
        led_flage = true;
      }
    }
  #endif
}
/********************************************* L O O P *********************************************/
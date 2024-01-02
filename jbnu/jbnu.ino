#include "Arduino.h"
#include "Wire.h"

/******************************************* PCA9555 LIB *******************************************/
#define NXP_INPUT 0
#define NXP_OUTPUT 2
#define NXP_CONFIG 6

class PCA9555 {
public:
  PCA9555(uint8_t address, int interruptPin = -1);  // optional interrupt pin in second argument
  void pinMode(uint8_t pin, uint8_t IOMode);        // pinMode
  uint8_t digitalRead(uint8_t pin);                 // digitalRead
  void digitalWrite(uint8_t pin, uint8_t value);    // digitalWrite
  uint8_t stateOfPin(uint8_t pin);                  // Actual ISR
  void setClock(uint32_t clockFrequency);           // Clock speed
  bool begin();                                     // Checks if PCA is responsive

private:
  static PCA9555 *instancePointer;
  static void alertISR(void);  // Function pointing to actual ISR
  void pinStates();            // Function tied to interrupt
  uint16_t I2CGetValue(uint8_t address, uint8_t reg);
  void I2CSetValue(uint8_t address, uint8_t reg, uint8_t value);

  union {
    struct {
      uint8_t _stateOfPins_low;   // low order byte
      uint8_t _stateOfPins_high;  // high order byte
    };
    uint16_t _stateOfPins;  // 16 bits presentation
  };
  union {
    struct {
      uint8_t _configurationRegister_low;   // low order byte
      uint8_t _configurationRegister_high;  // high order byte
    };
    uint16_t _configurationRegister;  // 16 bits presentation
  };
  union {
    struct {
      uint8_t _valueRegister_low;   // low order byte
      uint8_t _valueRegister_high;  // high order byte
    };
    uint16_t _valueRegister;
  };
  uint8_t _address;  // address of port this class is supporting
  int _error;        // error code from I2C
};

PCA9555 *PCA9555::instancePointer = 0;
PCA9555::PCA9555(uint8_t address, int interruptPin) {
  _address = address;  // save the address id
  _valueRegister = 0;
  Wire.begin();  // start I2C communication
  if (interruptPin >= 0) {
    instancePointer = this;
    attachInterrupt(digitalPinToInterrupt(interruptPin), PCA9555::alertISR, false);  // Set to low for button presses
  }
}

bool PCA9555::begin() {
  Wire.beginTransmission(_address);
  Wire.write(0x02);  // Test Address
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
    I2CSetValue(_address, NXP_CONFIG, _configurationRegister_low);
    I2CSetValue(_address, NXP_CONFIG + 1, _configurationRegister_high);
  }
}
uint8_t PCA9555::digitalRead(uint8_t pin) {
  uint16_t _inputData = 0;
  if (pin > 15) return 255;
  _inputData = I2CGetValue(_address, NXP_INPUT);
  _inputData |= I2CGetValue(_address, NXP_INPUT + 1) << 8;
  if ((_inputData & (1 << pin)) > 0) {
    return true;
  } else {
    return false;
  }
}
void PCA9555::digitalWrite(uint8_t pin, uint8_t value) {
  if (pin > 15) {
    _error = 255;  // invalid pin
    return;        // exit
  }
  if (value > 0) {
    _valueRegister = _valueRegister | (1 << pin);  // and OR bit in register
  } else {
    _valueRegister = _valueRegister & ~(1 << pin);  // AND all bits
  }
  I2CSetValue(_address, NXP_OUTPUT, _valueRegister_low);
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
void PCA9555::alertISR() {
  if (instancePointer != 0) {
    instancePointer->pinStates();  // Points to the actual ISR
  }
}
uint16_t PCA9555::I2CGetValue(uint8_t address, uint8_t reg) {
  uint16_t _inputData;
  Wire.beginTransmission(address);  // setup read registers
  Wire.write(reg);
  _error = Wire.endTransmission();
  if (Wire.requestFrom((int)address, 1) != 1) {
    return 256;  // error code is above normal data range
  };
  _inputData = Wire.read();
  return _inputData;
}
void PCA9555::I2CSetValue(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);  // setup direction registers
  Wire.write(reg);                  // pointer to configuration register address 0
  Wire.write(value);                // write config register low byte
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

#define SPEED_EXCHANGE_TIME 1200000

//#define DEBUG_MONIT


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

void Display(String IDs, uint32_t values) {
  String cmd;
  char buf[8] = { 0 };
  sprintf(buf, "%d", values);
  cmd = IDs + ".val=";
  cmd += buf;
  send2Nextion(cmd);
}
/******************************************* NEXTION ***********************************************/
/******************************************* STEP  MOTOR *******************************************/
#define STEP_NUM 3

typedef struct STEP_ts {
  int8_t DIR;  //Direction
  int8_t ENA;  //Enable
  int8_t PUL;  //Pulse
} STEP_ts;
//uint8_t pin_out[6] = {4, 26, 27, 14, 12, 13};
const STEP_ts step_upper[STEP_NUM] = {
  { 15, 9, 4 },
  { 14, 8, 26 },
  { 13, 0, 27 },
};
const STEP_ts step_under[STEP_NUM] = {
  { 12, 1, 14 },
  { 11, 2, 12 },
  { 10, 3, 13 },
};

typedef struct STEP_CTR_ts {
  int32_t DEST;   //Destination
  int32_t START;  //Start point
  int32_t POS;    //Position
  bool DIR;       //Direction
  bool ACT;       //Actibation
} STEP_CTR_ts;
STEP_CTR_ts upper_ctr[STEP_NUM] = {
  { 0, 0, 0, false, false },
  { 0, 0, 0, false, false },
  { 0, 0, 0, false, false },
};
STEP_CTR_ts under_ctr[STEP_NUM] = {
  { 0, 0, 0, false, false },
  { 0, 0, 0, false, false },
  { 0, 0, 0, false, false },
};

typedef struct STEP_CAL_ts {
  bool    PUL;   //Pulse flage
  bool    RUN;   //Axis run flage
  int32_t RATIO; //Distance ratio
  int32_t COUNT; //Stap ratio count
} STEP_CAL_ts;
STEP_CAL_ts upper_cal[STEP_NUM] = {
  { 0, false, 0, 0 },
  { 0, false, 0, 0 },
  { 0, false, 0, 0 },
};
STEP_CAL_ts under_cal[STEP_NUM] = {
  { 0, false, 0, 0 },
  { 0, false, 0, 0 },
  { 0, false, 0, 0 },
};
uint8_t nextion_page = 0;

bool stepmoter_work = false;
bool stepmoter_sync = false;
bool postions_sync  = false;
bool NextStep_upper = false;
bool NextStep_under = false;

bool manual_mode = false;
bool manual_mode_work = false;
char manual_command[3] = { 0x00, 0x00, 0x00 };
unsigned long Update_manual = 0UL;

uint32_t Interval_upper = SPEED_EXCHANGE_TIME;
uint32_t Interval_upper_cal = SPEED_EXCHANGE_TIME;
uint32_t Interval_under = SPEED_EXCHANGE_TIME;
uint32_t Interval_under_cal = SPEED_EXCHANGE_TIME;

unsigned long Update_upper = 0UL;
unsigned long Update_under = 0UL;

void postion_cal_upper() {
  if (!NextStep_upper) {
    
    NextStep_upper   = true;
    uint8_t Interval_upper_count = 0;
    bool ctr_temp[3] = {false,};
    for (uint8_t index = 0; index < STEP_NUM; index++){
      if(upper_cal[index].RUN && (upper_ctr[index].DEST != upper_ctr[index].POS) && (++upper_cal[index].COUNT >= upper_cal[index].RATIO)){
        upper_cal[index].COUNT = 0;
        upper_cal[index].PUL   = true;
        Interval_upper_count++;
      }
    }

    Interval_upper_cal = 100; //Interval_upper;

    float Interval_upper_float = Interval_upper;

    if(Interval_upper_count == 1){
      Interval_upper_float *= 1.414;
      Interval_upper_cal    = Interval_upper_float;
    }else if(Interval_upper_count == 2){
      Interval_upper_float *= 1.732;
      Interval_upper_cal    = Interval_upper_float;
    }
  }
}

void postion_cal_under() {
  if (!NextStep_under) {
    NextStep_under = true;    
    uint8_t Interval_under_count = 0;
    for (uint8_t index = 0; index < STEP_NUM; index++){
      if(under_cal[index].RUN && (under_ctr[index].DEST != under_ctr[index].POS) && (++under_cal[index].COUNT >= under_cal[index].RATIO)){
        under_cal[index].COUNT = 0;
        under_cal[index].PUL   = true;
        Interval_under_count++;
      }
    }
    
    Interval_under_cal = 100; //Interval_under;

    float Interval_under_float = Interval_under;

    if(Interval_under_count == 1){
      Interval_under_float *= 1.414;
      Interval_under_cal = Interval_under_float;
    }else if(Interval_under_count == 2){
      Interval_under_float *= 1.732;
      Interval_under_cal = Interval_under_float;
    }
  }
}

void moter_run(unsigned long *millisec) {
  if (stepmoter_work) {
    postion_cal_upper();
    postion_cal_under();
    if (NextStep_upper && (*millisec - Update_upper > Interval_upper_cal)) {
      Update_upper   = *millisec;
      NextStep_upper = false;
      for (uint8_t index = 0; index < STEP_NUM; index++) {
        if (upper_cal[index].PUL) {
          if (!upper_ctr[index].ACT) {
            upper_ctr[index].ACT = true;
            ioport.digitalWrite(step_upper[index].ENA, false);
          }
          ioport.digitalWrite(step_upper[index].DIR, upper_ctr[index].DIR);
          digitalWrite(step_upper[index].PUL, true);
        }
      }
      for (uint8_t index = 0; index < STEP_NUM; index++) {  //pulse on time delay
        if (upper_cal[index].PUL) {
          upper_cal[index].PUL = false;
          if (upper_ctr[index].DIR) {
            upper_ctr[index].POS += 1;
          } else {
            if (upper_ctr[index].POS > 0) upper_ctr[index].POS -= 1;
          }
          digitalWrite(step_upper[index].PUL, false);
        }
      }
    }

    if (NextStep_under && (*millisec - Update_under > Interval_under_cal)) {
      Update_under   = *millisec;
      NextStep_under = false;
      for (uint8_t index = 0; index < STEP_NUM; index++) {
        if (under_cal[index].PUL) {
          if (!under_ctr[index].ACT) {
            under_ctr[index].ACT = true;
            ioport.digitalWrite(step_under[index].ENA, false);
          }
          ioport.digitalWrite(step_under[index].DIR, under_ctr[index].DIR);
          digitalWrite(step_under[index].PUL, true);
        }
      }
      for (uint8_t index = 0; index < STEP_NUM; index++) {  //pulse on time delay
        if (under_cal[index].PUL) {
          under_cal[index].PUL = false;
          if (under_ctr[index].DIR) {
            under_ctr[index].POS += 1;
          } else {
            if (under_ctr[index].POS > 0) under_ctr[index].POS -= 1;
          }
          digitalWrite(step_under[index].PUL, false);
        }
      }
    }

    //HMI Position refresh here.
    if (nextion_page == 0) {
      Display("nTX", upper_ctr[0].POS);
      Display("nTY", upper_ctr[1].POS);
      Display("nTZ", upper_ctr[2].POS);
      Display("nBX", under_ctr[0].POS);
      Display("nBY", under_ctr[1].POS);
      Display("nBZ", under_ctr[2].POS);
    }
  }
}  //moter_run()

void moter_sleep(unsigned long *millisec) {
  for (uint8_t index = 0; index < STEP_NUM; index++) {
    if (under_ctr[index].ACT && (*millisec - Update_upper > 10)) {
      under_ctr[index].ACT = false;
      ioport.digitalWrite(step_upper[index].ENA, true);
    }
    if (under_ctr[index].ACT && (*millisec - Update_under > 10)) {
      under_ctr[index].ACT = false;
      ioport.digitalWrite(step_under[index].ENA, true);
    }
  }
}  //moter_sleep()

void moter_manual(unsigned long *millisec) {
  if (manual_mode) {
    if (*millisec - Update_manual > 1) {
      bool up_down = false;
      bool mn_dir = false;
      uint8_t moter_number = 0;
      if (manual_command[0] == 'T') { up_down = true; }
      if (manual_command[2] == 'U') { mn_dir = true; }
      if (manual_command[1] == 'Y') {
        moter_number = 1;
      } else if (manual_command[1] == 'Z') {
        moter_number = 2;
      }
      if (up_down) {
        ioport.digitalWrite(step_upper[moter_number].ENA, false);
        ioport.digitalWrite(step_upper[moter_number].DIR, mn_dir);
        digitalWrite(step_upper[moter_number].PUL, true);
        if (upper_ctr[moter_number].DIR) {
          upper_ctr[moter_number].POS += 1;
        } else {
          if (upper_ctr[moter_number].POS > 0) upper_ctr[moter_number].POS -= 1;
        }
      } else {
        ioport.digitalWrite(step_under[moter_number].ENA, false);
        ioport.digitalWrite(step_under[moter_number].DIR, mn_dir);
        digitalWrite(step_under[moter_number].PUL, true);
        if (under_ctr[moter_number].DIR) {
          under_ctr[moter_number].POS += 1;
        } else {
          if (under_ctr[moter_number].POS > 0) under_ctr[moter_number].POS -= 1;
        }
      }
      Update_manual = *millisec;
      manual_mode_work = true;
      if (up_down) {
        digitalWrite(step_upper[moter_number].PUL, false);
      } else {
        digitalWrite(step_under[moter_number].PUL, false);
      }
    }
  } else if (manual_mode_work) {
    manual_mode_work = false;
    for (uint8_t index = 0; index < STEP_NUM; index++) {
      ioport.digitalWrite(step_under[index].ENA, true);
      ioport.digitalWrite(step_upper[index].ENA, true);
    }
  }  //manual mode off
}  //moter_manual()
/******************************************* STEP  MOTOR *******************************************/
/******************************************* SERIAL PROCESS ****************************************/
#define SERIAL_MAX 8
char Serial_buf[SERIAL_MAX];
int16_t Serial_num;
void Serial_process(char ch) {
  if (ch == 0x03) {
    Serial_buf[Serial_num] = 0x00;
    Serial.println(Serial_buf);
    command_pros();
    Serial_num = 0;
  } else if (ch == 0x02) {
    Serial_num = 0;
  } else {
    Serial_buf[Serial_num++] = ch;
    Serial_num %= SERIAL_MAX;
  }
}
void page_zero() {
  Display("btn_run", stepmoter_work);
  Display("btn_sync", stepmoter_sync);
  Display("nTS", SPEED_EXCHANGE_TIME/Interval_upper);
  Display("nTX_T", upper_ctr[0].DEST);
  Display("nTY_T", upper_ctr[1].DEST);
  Display("nTZ_T", upper_ctr[2].DEST);
  Display("nTX", upper_ctr[0].POS);
  Display("nTY", upper_ctr[1].POS);
  Display("nTZ", upper_ctr[2].POS);

  Display("nBS", SPEED_EXCHANGE_TIME/Interval_under);
  Display("nBX_T", under_ctr[0].DEST);
  Display("nBY_T", under_ctr[1].DEST);
  Display("nBZ_T", under_ctr[2].DEST);
  Display("nBX", under_ctr[0].POS);
  Display("nBY", under_ctr[1].POS);
  Display("nBZ", under_ctr[2].POS);
}
//0x02 [0][1][2][3][4][5][6][0x00] 0x03
void command_pros() {
  if (Serial_buf[0] == 'P' && Serial_buf[1] == 'A' && Serial_buf[2] == 'G' && Serial_buf[3] == 'E') {

    if (Serial_buf[5] == '0') {
      nextion_page = 0;
      page_zero();
    } else if (Serial_buf[5] == '1') {
      nextion_page = 1;
    } else if (Serial_buf[5] == '2') {
      nextion_page = 2;
    }

  } else if (Serial_buf[0] == 'W' && Serial_buf[1] == 'O' && Serial_buf[2] == 'R' && Serial_buf[3] == 'K') {

    if (Serial_buf[5] == 'O' && Serial_buf[6] == 'N') {      
      uint32_t distance_upper[STEP_NUM] = {0,};
      uint32_t distance_under[STEP_NUM] = {0,};

      uint32_t max_upper = 1;
      uint32_t max_under = 1;

      for (uint8_t index = 0; index < STEP_NUM; index++){
        upper_ctr[index].START = upper_ctr[index].POS;
        under_ctr[index].START = under_ctr[index].POS;

        if(upper_ctr[index].DEST != upper_ctr[index].START){
          upper_cal[index].RUN = true;
          if(upper_ctr[index].DEST - upper_ctr[index].START > 0){
            distance_upper[index] = upper_ctr[index].DEST - upper_ctr[index].START;
            upper_ctr[index].DIR  = true;
          }else{
            distance_upper[index] = upper_ctr[index].START - upper_ctr[index].DEST;
            upper_ctr[index].DIR  = false;
          }
          max_upper = distance_upper[index];
        }else{
          upper_cal[index].RUN = false;
        }
        if(stepmoter_sync){ ///////////////////////////////////here
          under_cal[index].RUN = upper_cal[index].RUN;
          if(upper_ctr[index].POS != under_ctr[index].POS){
            under_ctr[index].RUN = true; //under module move to upper module position.
            sync_postions = true;
          }
        }
        else{
          if(under_ctr[index].DEST != under_ctr[index].START){
            under_cal[index].RUN = true;
            if(under_ctr[index].DEST - under_ctr[index].START > 0){
              distance_under[index] = under_ctr[index].DEST - under_ctr[index].START;
              under_ctr[index].DIR  = true;
            }else{
              distance_under[index] = under_ctr[index].START - under_ctr[index].DEST;
              under_ctr[index].DIR  = false;
            }
            max_under = distance_under[index];
          }else{
            under_cal[index].RUN = false;
          }
        }
      }

      for (uint8_t index = 1; index < STEP_NUM; index++){
        if(upper_cal[index-1].RUN && upper_cal[index].RUN){
          if(distance_upper[index-1] > distance_upper[index] ){
            if(distance_upper[index-1] > max_upper) max_upper = distance_upper[index-1];
          }else if(distance_upper[index-1] < distance_upper[index]){
            if(distance_upper[index] > max_upper) max_upper = distance_upper[index];
          }

          if(distance_under[index-1] > distance_under[index] ){
            if(distance_under[index-1] > max_under) max_under = distance_under[index-1];
          }else if(distance_under[index-1] < distance_under[index]){
            if(distance_under[index] > max_under) max_under = distance_under[index];
          }
        }
      }

      Serial.print("max_upper: ");Serial.println(max_upper);
      Serial.print("max_under: ");Serial.println(max_under);
      
      for (uint8_t index = 0; index < STEP_NUM; index++){
        if(upper_cal[index].RUN){
          upper_cal[index].RATIO = max_upper/distance_upper[index];
        }
        if(under_cal[index].RUN){
          under_cal[index].RATIO = max_under/distance_under[index];
        }
      }
      
      stepmoter_work = true;
      NextStep_upper = false;
      NextStep_under = false;
      
    } else if (Serial_buf[5] == 'F' && Serial_buf[6] == 'F') {
      stepmoter_work = false;
    }

  } else if (Serial_buf[0] == 'S' && Serial_buf[1] == 'Y' && Serial_buf[2] == 'N' && Serial_buf[3] == 'C') {

    if (Serial_buf[5] == 'O' && Serial_buf[6] == 'N') {
      stepmoter_work = false;
      stepmoter_sync = true;
      under_ctr[0].DEST = upper_ctr[0].DEST;
      under_ctr[1].DEST = upper_ctr[1].DEST;
      under_ctr[2].DEST = upper_ctr[2].DEST;
      Display("nBX_T", under_ctr[0].DEST);
      Display("nBY_T", under_ctr[1].DEST);
      Display("nBZ_T", under_ctr[2].DEST);
      //좌표 계산 다시
    } else if (Serial_buf[5] == 'F' && Serial_buf[6] == 'F') {
      stepmoter_work = false;
      stepmoter_sync = false;
      //좌표 계산 다시
    }

  } else if (Serial_buf[0] == 'M' && Serial_buf[1] == 'N') {

    if (Serial_buf[3] == 'O' && Serial_buf[4] == 'F' && Serial_buf[4] == 'F') {
      manual_mode = false;  //Manual_stop
    } else if (Serial_buf[3] == 'Z' && Serial_buf[4] == 'R') {
      if (Serial_buf[6] == 'T') {  //Zero_set = top
        for (uint8_t index = 0; index < STEP_NUM; index++) {
          upper_ctr[index].POS = 0;
        }
      } else if (Serial_buf[6] == 'B') {  //Zero_set = bottom
        for (uint8_t index = 0; index < STEP_NUM; index++) {
          under_ctr[index].POS = 0;
        }
      }
      Serial.println("Zero set");
    } else {
      manual_mode = true;
      manual_command[0] = Serial_buf[3];
      manual_command[1] = Serial_buf[4];
      manual_command[2] = Serial_buf[6];
    }

  } else if (Serial_buf[0] == 'C' && Serial_buf[1] == 'T') {
    stepmoter_work = false;
    //좌표 계산 다시
    if (Serial_buf[2] == 'S') {  //Top stepper moving speed change
      uint16_t buffer_num[2] = { Serial_buf[3], Serial_buf[4] };
      uint32_t speed_temp    = buffer_num[1] * 256 + buffer_num[0]; //20um per step // step/20*60*1000=1um/min
      Interval_upper = SPEED_EXCHANGE_TIME/speed_temp;
      Serial.print("Interval_upper: ");Serial.println(Interval_upper);
    } else if (Serial_buf[2] == 'X') {
      uint32_t buffer_num[4] = { Serial_buf[3], Serial_buf[4], Serial_buf[5], Serial_buf[6] };
      upper_ctr[0].DEST = buffer_num[3] * 256 * 256 * 256 + buffer_num[2] * 256 * 256 + buffer_num[1] * 256 + buffer_num[0];
    } else if (Serial_buf[2] == 'Y') {
      uint32_t buffer_num[4] = { Serial_buf[3], Serial_buf[4], Serial_buf[5], Serial_buf[6] };
      upper_ctr[1].DEST = buffer_num[3] * 256 * 256 * 256 + buffer_num[2] * 256 * 256 + buffer_num[1] * 256 + buffer_num[0];
    } else if (Serial_buf[2] == 'Z') {
      uint32_t buffer_num[4] = { Serial_buf[3], Serial_buf[4], Serial_buf[5], Serial_buf[6] };
      upper_ctr[2].DEST = buffer_num[3] * 256 * 256 * 256 + buffer_num[2] * 256 * 256 + buffer_num[1] * 256 + buffer_num[0];
    }  //Top stepper target position change

  } else if (Serial_buf[0] == 'C' && Serial_buf[1] == 'B') {
    stepmoter_work = false;
    //좌표 계산 다시
    if (Serial_buf[2] == 'S') {  //Bottom stepper moving speed change
      uint16_t buffer_num[2] = { Serial_buf[3], Serial_buf[4] };
      uint32_t speed_temp    = buffer_num[1] * 256 + buffer_num[0]; //20um per step // step/20*60*1000=1um/min
      Interval_under = SPEED_EXCHANGE_TIME/speed_temp;
      Serial.print("Interval_under: ");Serial.println(Interval_under);
    } else if (Serial_buf[2] == 'X') {
      uint32_t buffer_num[4] = { Serial_buf[3], Serial_buf[4], Serial_buf[5], Serial_buf[6] };
      under_ctr[0].DEST = buffer_num[3] * 256 * 256 * 256 + buffer_num[2] * 256 * 256 + buffer_num[1] * 256 + buffer_num[0];
    } else if (Serial_buf[2] == 'Y') {
      uint32_t buffer_num[4] = { Serial_buf[3], Serial_buf[4], Serial_buf[5], Serial_buf[6] };
      under_ctr[1].DEST = buffer_num[3] * 256 * 256 * 256 + buffer_num[2] * 256 * 256 + buffer_num[1] * 256 + buffer_num[0];
    } else if (Serial_buf[2] == 'Z') {
      uint32_t buffer_num[4] = { Serial_buf[3], Serial_buf[4], Serial_buf[5], Serial_buf[6] };
      under_ctr[2].DEST = buffer_num[3] * 256 * 256 * 256 + buffer_num[2] * 256 * 256 + buffer_num[1] * 256 + buffer_num[0];
    }  //Bottom stepper target position change

  } else {
    Serial.print("Is not command: ");
    Serial.println(Serial_buf);
  }
}

void SerialConnect() {
  if (Serial.available()) {
    Serial_process(Serial.read());
  }
}
/******************************************* SERIAL PROCESS ****************************************/

uint8_t pin_in[3] = { 32, 33, 25 };

/******************************************** S E T U P ********************************************/
void setup() {
  Serial.begin(115200);
  nxSerial.begin(115200, SERIAL_8N1, RX2, TX2);
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
    ioport.digitalWrite(step_under[index].ENA, true);
    ioport.pinMode(step_under[index].DIR, OUTPUT);
    ioport.digitalWrite(step_under[index].DIR, false);
    pinMode(step_under[index].PUL, OUTPUT);
    ioport.pinMode(step_upper[index].ENA, OUTPUT);
    ioport.digitalWrite(step_upper[index].ENA, true);
    ioport.pinMode(step_upper[index].DIR, OUTPUT);
    ioport.digitalWrite(step_upper[index].DIR, false);
    pinMode(step_upper[index].PUL, OUTPUT);
  }

  for (uint8_t index = 0; index < 3; index++) {
    pinMode(pin_in[index], INPUT_PULLUP);
  }

  send2Nextion("page 0");
  page_zero();
  Serial.println("System all green");
}
/******************************************** S E T U P ********************************************/

/********************************************* L O O P *********************************************/
void loop() {
  unsigned long millisec = millis();
  SerialConnect();
  DisplayConnect();
  moter_run(&millisec);
  moter_sleep(&millisec);
  moter_manual(&millisec);
  /*
    for (uint8_t index = 0; index < 3; index++) {
    Serial.print("limit sw ");
    Serial.print(index);
    Serial.print(": ");
    Serial.println(digitalRead(pin_in[index]));
    }
  */

#ifdef DEBUG_MONIT
  pin_led_check();
#endif
}
/********************************************* L O O P *********************************************/

#ifdef DEBUG_MONIT
uint8_t index_led = 0;
boolean led_flage = true;
void pin_led_check() {
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
    if (++index_led >= STEP_NUM) {
      index_led = 0;
      led_flage = false;
    }
  } else {
    digitalWrite(step_upper[index_led].PUL, true);
    digitalWrite(step_under[index_led].PUL, true);
    delay(300);
    digitalWrite(step_upper[index_led].PUL, false);
    digitalWrite(step_under[index_led].PUL, false);
    if (++index_led >= STEP_NUM) {
      index_led = 0;
      led_flage = true;
    }
  }
}
#endif
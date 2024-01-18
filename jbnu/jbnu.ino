/*  dubble automatic XYZ stage  */
//- feed resolution: 10um
//- max speed: 330um/s
//- pulse generator frequency: recommended ~300KHz, max 40MHz

/********** hardware **********/
//housing:  Aluminum profile 40x40
//frame :   Al6061

//actuator:
//- LM guide:   MGN12H, 
//- LeadScrew:  8pi lead2
//- stepmoter:  nema17

//controler:
//- moter driver: tb6600
//- power source: 24VDC

//- motherboard:
// * mcu:                 ESP32-D0WD-V3
// * mcu to pin isolator: pc817
// * pin extender ic:     PCA9555
// * pin out transistor : uln2803
// * mosfet :             IRFP4227
// * mosfet  dirver:      UCC27524
// * power supply:        IRM-20-5

//- display: Nextion HMI LCD, NX8048P070-011C-Y
/********** hardware **********/

#include "Arduino.h"
#include "Wire.h"
/******************************************* Ver 0.2.0   *******************************************/
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

#define TIME_UNIT           1000
#define SPEED_EXCHANGE_TIME 1200000
#define SPEED_MAX_um_min    60000

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
STEP_CTR_ts sync_ctr[STEP_NUM] = {
  { 0, 0, 0, false, false },
  { 0, 0, 0, false, false },
  { 0, 0, 0, false, false },
};

typedef struct STEP_CAL_ts {
  bool    PUL;      //Pulse flage
  bool    RUN;      //Axis run flage
  float   CONST[2]; //funstion constant
  uint8_t XYZ[2];   //Axis for constant
} STEP_CAL_ts;
STEP_CAL_ts upper_cal[STEP_NUM] = {
  { 0, false, {0,0}, {0,0} },
  { 0, false, {0,0}, {0,0} },
  { 0, false, {0,0}, {0,0} },
};
STEP_CAL_ts under_cal[STEP_NUM] = {
  { 0, false, {0,0}, {0,0} },
  { 0, false, {0,0}, {0,0} },
  { 0, false, {0,0}, {0,0} },
};
STEP_CAL_ts sync_cal[STEP_NUM] = {
  { 0, false, {0,0}, {0,0} },
  { 0, false, {0,0}, {0,0} },
  { 0, false, {0,0}, {0,0} },
};
uint8_t nextion_page = 0;

uint32_t time_remain_upper = 0;
uint32_t time_remain_upper_index = 0;
uint32_t time_remain_upper_check = 0;
uint32_t time_remain_under = 0;
uint32_t time_remain_under_index = 0;
uint32_t time_remain_under_check = 0;
uint8_t  max_upper_index   = 0;
uint8_t  max_under_index   = 0;
uint8_t  max_sync_index    = 0;

bool stepmoter_work = false;
bool stepmoter_sync = false;
bool postions_sync  = false;
bool NextStep_upper = false;
bool NextStep_under = false;
bool NextStep_sync  = false;

bool manual_mode = false;
bool manual_mode_work = false;
char manual_command[3] = { 0x00, 0x00, 0x00 };
unsigned long Update_manual = 0UL;
unsigned long run_time      = 0UL;

uint32_t Interval_upper     = SPEED_EXCHANGE_TIME;
uint32_t Interval_upper_cal = SPEED_EXCHANGE_TIME*TIME_UNIT;
uint32_t Interval_under     = SPEED_EXCHANGE_TIME;
uint32_t Interval_under_cal = SPEED_EXCHANGE_TIME*TIME_UNIT;
uint32_t Interval_sync      = SPEED_EXCHANGE_TIME;
uint32_t Interval_sync_cal  = SPEED_EXCHANGE_TIME*TIME_UNIT;

unsigned long Update_upper = 0UL;
unsigned long Update_under = 0UL;
unsigned long Update_sync  = 0UL;

uint8_t interval_queue[3][3] = {{0,},};
uint8_t queue_index[3]       = {0,};

void interval_linear_cal(uint8_t *queue_interval, uint8_t *index_queue, uint32_t *Interval_val, uint32_t *Interval_cal, uint8_t *count_Interval, uint8_t *Axis_index){
  float Interval_float = *Interval_val;
  if(*count_Interval == 1){
    *(queue_interval+(*index_queue)) = *Axis_index;
    if(*queue_interval != *(queue_interval+1) && *(queue_interval+1) != *(queue_interval+2) && *(queue_interval+2) != *queue_interval){
      Interval_float  = TIME_UNIT*Interval_float/1.732;
      *Interval_cal   = Interval_float;
    }else if(*index_queue > 0 && (*(queue_interval+(*index_queue-1)) == *(queue_interval+(*index_queue)))){
      *Interval_cal   = TIME_UNIT*(*Interval_val);
    }else if(*index_queue == 0 && *(queue_interval+2) == *queue_interval){
      *Interval_cal   = TIME_UNIT*(*Interval_val);
    }else{
      Interval_float  = TIME_UNIT*Interval_float/1.414;
      *Interval_cal   = Interval_float;
    }
    *index_queue += 1;
    if(*index_queue >= 3) *index_queue = 0;
  }else if(*count_Interval == 2){
    Interval_float  = TIME_UNIT*Interval_float*1.414;
    *Interval_cal   = Interval_float;
  }else if(*count_Interval == 3){
    Interval_float  = TIME_UNIT*Interval_float*1.732;
    *Interval_cal   = Interval_float;
  }
  #ifdef DEBUG_MONIT
    Serial.print("arr: ");Serial.print(*(queue_interval));Serial.print(",");Serial.print(*(queue_interval+1));Serial.print(",");Serial.print(*(queue_interval+2));
    Serial.print(" , index_queue: ");Serial.print(*index_queue);Serial.print(" , interval: ");Serial.println(*Interval_cal);
  #endif
 }
/******************************************* Linear movement ****************************************/
void linear_upper() {
  if (!NextStep_upper) {
    NextStep_upper   = true;
    uint8_t Interval_count = 0;
    uint8_t Axis_num = 0;
    for (uint8_t index = 0; index < STEP_NUM; index++){
      uint8_t Axis_run = 0;
      if(upper_cal[index].RUN && (upper_ctr[index].DEST != upper_ctr[index].POS)){
        bool pulse_on_flage = false;
        if(upper_cal[index].CONST[0] != 0) Axis_run++;
        if(upper_cal[index].CONST[1] != 0) Axis_run++;
        if(Axis_run == 0 ){ pulse_on_flage = true;}
        else{
          float coordinate = (upper_cal[index].CONST[0]*float(upper_ctr[upper_cal[index].XYZ[0]].POS+1)+upper_cal[index].CONST[1]*float(upper_ctr[upper_cal[index].XYZ[1]].POS+1))/Axis_run;
          if(uint32_t(coordinate) != upper_ctr[index].POS) pulse_on_flage = true;
        }

        if(time_remain_upper_index++ > 3){
          time_remain_upper_index = 0;
          if(time_remain_upper_check != time_remain_upper){
            time_remain_upper_check = time_remain_upper;
          }else{
            pulse_on_flage = true;
          }
        }

        if(pulse_on_flage){
          upper_cal[index].PUL  = true;
          Interval_count++;
          Axis_num = index;
        }
        if(index == max_upper_index){
          if(time_remain_upper > 0) time_remain_upper--;
          Display("nTTR", time_remain_upper*Interval_upper/TIME_UNIT);
        }
      }
      if(upper_ctr[index].DEST == upper_ctr[index].POS) upper_cal[index].RUN = false;
    }
    if(Interval_count != 0) interval_linear_cal(&interval_queue[0][0], &queue_index[0], &Interval_upper, &Interval_upper_cal, &Interval_count, &Axis_num);
  }
}

void linear_under() {
  if (!NextStep_under) {
    NextStep_under = true;
    uint8_t Interval_count = 0;
    uint8_t Axis_num = 0;
    for (uint8_t index = 0; index < STEP_NUM; index++){
      uint8_t Axis_run = 0;
      if(under_cal[index].RUN && (under_ctr[index].DEST != under_ctr[index].POS)){
        bool pulse_on_flage = false;
        if(under_cal[index].CONST[0] != 0) Axis_run++;
        if(under_cal[index].CONST[1] != 0) Axis_run++;
        if(Axis_run == 0 ){ pulse_on_flage = true;}
        else{
          float coordinate = (under_cal[index].CONST[0]*float(under_ctr[under_cal[index].XYZ[0]].POS+1)+under_cal[index].CONST[1]*float(under_ctr[under_cal[index].XYZ[1]].POS+1))/Axis_run;
          if(uint32_t(coordinate) != under_ctr[index].POS) pulse_on_flage = true;
        }

        if(time_remain_under_index++ > 3){
          time_remain_under_index = 0;
          if(time_remain_under_check != time_remain_under){
            time_remain_under_check = time_remain_under;
          }else{
            pulse_on_flage = true;
          }
        }

        if(pulse_on_flage){
          under_cal[index].PUL  = true;
          Interval_count++;
          Axis_num = index;
        }
        if(index == max_under_index){
          if(time_remain_under > 0) time_remain_under--;
          Display("nBTR", (time_remain_under)*Interval_under/TIME_UNIT);
        }
      }
      if(under_ctr[index].DEST == under_ctr[index].POS) under_cal[index].RUN = false;
    }
    if(Interval_count != 0) interval_linear_cal(&interval_queue[1][0], &queue_index[1], &Interval_under, &Interval_under_cal, &Interval_count, &Axis_num);
  }
}

void linear_sync() {
  if (!NextStep_sync) {
    NextStep_sync = true;
    uint8_t Interval_count = 0;
    uint8_t Axis_num = 0;
    for (uint8_t index = 0; index < STEP_NUM; index++){
      uint8_t Axis_run = 0;
      if(sync_cal[index].RUN && (sync_ctr[index].DEST != sync_ctr[index].POS)){
        bool pulse_on_flage = false;
        if(sync_cal[index].CONST[0] != 0) Axis_run++;
        if(sync_cal[index].CONST[1] != 0) Axis_run++;
        if(Axis_run == 0 ){ pulse_on_flage = true;}
        else{
          float coordinate = (sync_cal[index].CONST[0]*float(sync_ctr[sync_cal[index].XYZ[0]].POS+1)+sync_cal[index].CONST[1]*float(sync_ctr[sync_cal[index].XYZ[1]].POS+1))/Axis_run;
          if(uint32_t(coordinate) != sync_ctr[index].POS) pulse_on_flage = true;
        }

        if(time_remain_under_index++ > 3){
          time_remain_under_index = 0;
          if(time_remain_under_check != time_remain_under){
            time_remain_under_check = time_remain_under;
          }else{
            pulse_on_flage = true;
          }
        }

        if(pulse_on_flage){
          sync_cal[index].PUL  = true;
          Interval_count++;
          Axis_num = index;
        }
        if(index == max_sync_index){
          if(time_remain_under > 0) time_remain_under--;
          Display("nTTR", time_remain_upper*Interval_upper/TIME_UNIT);
          Display("nBTR", (time_remain_under)*Interval_under/TIME_UNIT);
        }
      }
      if(sync_ctr[index].DEST == sync_ctr[index].POS) sync_cal[index].RUN = false;
    }
    if(Interval_count != 0) interval_linear_cal(&interval_queue[2][0], &queue_index[2], &Interval_upper, &Interval_sync_cal, &Interval_count, &Axis_num);
  }
}

void linear_run() {
  uint32_t distance_upper[STEP_NUM] = {0,};
  uint32_t distance_under[STEP_NUM] = {0,};
  uint32_t distance_sync[STEP_NUM]  = {0,};

  uint32_t max_upper = 1;
  uint32_t max_under = 1;
  uint32_t max_sync  = 1;

  flushing();

  if(stepmoter_sync){
    Interval_sync      = Interval_upper;
    Interval_sync_cal  = Interval_upper_cal;
    Interval_under     = Interval_upper;
    Interval_under_cal = Interval_upper_cal;
    Display("nBS", SPEED_EXCHANGE_TIME/Interval_upper);
  }

  for (uint8_t index = 0; index < STEP_NUM; index++){
    upper_ctr[index].START = upper_ctr[index].POS;
    under_ctr[index].START = under_ctr[index].POS;

    if(stepmoter_sync){
      under_ctr[index].DEST = upper_ctr[index].DEST;

      if(under_ctr[index].POS != upper_ctr[index].POS){
        postions_sync = true;
        under_ctr[index].START = upper_ctr[index].POS;
        
        sync_ctr[index].DEST   = upper_ctr[index].POS;
        sync_ctr[index].POS    = under_ctr[index].POS;
        sync_ctr[index].START  = under_ctr[index].POS;

        sync_cal[index].RUN    = true; //under module move to upper module position.
        
        if(sync_ctr[index].DEST >= sync_ctr[index].START){
          distance_sync[index] = sync_ctr[index].DEST - sync_ctr[index].START;
          sync_ctr[index].DIR  = true;
        }else{
          distance_sync[index] = sync_ctr[index].START - sync_ctr[index].DEST;
          sync_ctr[index].DIR  = false;
        }
        max_sync = distance_sync[index];
      }else{
        sync_cal[index].RUN = false;
      }
    }

    if(upper_ctr[index].DEST != upper_ctr[index].START){
      upper_cal[index].RUN = true;
      if(upper_ctr[index].DEST >= upper_ctr[index].START){
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

    if(under_ctr[index].DEST != under_ctr[index].START){
      under_cal[index].RUN = true;
      if(under_ctr[index].DEST >= under_ctr[index].START){
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

  Display("nBX_T", under_ctr[0].DEST);
  Display("nBY_T", under_ctr[1].DEST);
  Display("nBZ_T", under_ctr[2].DEST);

  for (uint8_t index = 0; index < STEP_NUM; index++) {
    uint8_t first_const  = 0;
    uint8_t second_const = 1;
    if(index == 0){
      first_const  = 1;
      second_const = 2;
    }else if(index == 1){
      second_const = 2;
    }

    upper_cal[index].XYZ[0] = first_const;
    upper_cal[index].XYZ[1] = second_const;
    upper_cal[index].CONST[0] = 0;
    upper_cal[index].CONST[1] = 0;
    if(distance_upper[first_const] != 0)  upper_cal[index].CONST[0] = float(distance_upper[index])/float(distance_upper[first_const]);
    if(distance_upper[second_const] != 0) upper_cal[index].CONST[1] = float(distance_upper[index])/float(distance_upper[second_const]);

    under_cal[index].XYZ[0] = first_const;
    under_cal[index].XYZ[1] = second_const;
    under_cal[index].CONST[0] = 0;
    under_cal[index].CONST[1] = 0;
    if(distance_under[first_const] != 0)  under_cal[index].CONST[0] = float(distance_under[index])/float(distance_under[first_const]);
    if(distance_under[second_const] != 0) under_cal[index].CONST[1] = float(distance_under[index])/float(distance_under[second_const]);

    sync_cal[index].XYZ[0] = first_const;
    sync_cal[index].XYZ[1] = second_const;
    sync_cal[index].CONST[0] = 0;
    sync_cal[index].CONST[1] = 0;
    if(distance_sync[first_const] != 0)  sync_cal[index].CONST[0] = float(distance_sync[index])/float(distance_sync[first_const]);
    if(distance_sync[second_const] != 0) sync_cal[index].CONST[1] = float(distance_sync[index])/float(distance_sync[second_const]);
  }

  /*************
    2*x = distance_x/distance_y*y + distance_x/distance_z*z;
    2*y = distance_y/distance_x*x + distance_y/distance_z*z;
    2*z = distance_z/distance_x*x + distance_z/distance_y*y;
  ***************/

  //remain time calculte
  for (uint8_t index = 1; index < STEP_NUM; index++){
    if(upper_cal[index-1].RUN && upper_cal[index].RUN){
      if(distance_upper[index-1] > distance_upper[index] ){
        if(distance_upper[index-1] > max_upper){
          max_upper = distance_upper[index-1];
          max_upper_index = index-1;
        }
      }else if(distance_upper[index-1] < distance_upper[index]){
        if(distance_upper[index] > max_upper){
          max_upper = distance_upper[index];
          max_upper_index = index;
        }
      }
    }
    if(under_cal[index-1].RUN && under_cal[index].RUN){
      if(distance_under[index-1] > distance_under[index] ){
        if(distance_under[index-1] > max_under){
          max_under = distance_under[index-1];
          max_under_index = index-1;
        }
      }else if(distance_under[index-1] < distance_under[index]){
        if(distance_under[index] > max_under){
          max_under = distance_under[index];
          max_under_index = index;
        }
      }
    }
    if(sync_cal[index-1].RUN && sync_cal[index].RUN){
      if(distance_sync[index-1] > distance_sync[index] ){
        if(distance_sync[index-1] > max_sync){
          max_sync = distance_sync[index-1];
          max_sync_index = index-1;
        }
      }else if(distance_sync[index-1] < distance_sync[index]){
        if(distance_sync[index] > max_sync){
          max_sync = distance_sync[index];
          max_sync_index = index;
        }
      }
    }
  }

  stepmoter_work = true;
  NextStep_upper = false;
  NextStep_under = false;
  NextStep_sync  = false;

  time_remain_upper = max_upper;
  time_remain_under = max_under + max_sync;
  if(max_upper!=1) Display("nTTR", (time_remain_upper--)*Interval_upper/TIME_UNIT);
  if(max_under!=1 || max_sync!=1) Display("nBTR", (time_remain_under--)*Interval_under/TIME_UNIT);

}
/******************************************* Linear movement ****************************************/
void moter_run(unsigned long *microsec) {
  if (stepmoter_work) {

    if(postions_sync){
      linear_sync();
      if (NextStep_sync && (*microsec - Update_sync > Interval_sync_cal)) {
        Update_sync   = *microsec;
        Update_under  = *microsec;
        NextStep_sync = false;
        for (uint8_t index = 0; index < STEP_NUM; index++) {
          if (sync_cal[index].PUL) {
            if (!sync_ctr[index].ACT) {
              sync_ctr[index].ACT = true;
              ioport.digitalWrite(step_under[index].ENA, false);
            }
            if(sync_ctr[index].POS == sync_ctr[index].START) ioport.digitalWrite(step_under[index].DIR, sync_ctr[index].DIR);
            digitalWrite(step_under[index].PUL, true);
            
            sync_cal[index].PUL = false;
            if (sync_ctr[index].DIR) {
              sync_ctr[index].POS += 1;
            } else {
              if (sync_ctr[index].POS > 0) sync_ctr[index].POS -= 1;
            }
            under_ctr[index].POS = sync_ctr[index].POS;
            //HMI Position refresh here.
            if (nextion_page == 0) {
              if (index == 0) Display("nBX", under_ctr[0].POS);
              else if (index == 1) Display("nBY", under_ctr[1].POS);
              else if (index == 2) Display("nBZ", under_ctr[2].POS);
            }
            digitalWrite(step_under[index].PUL, false);
          }
        }
      }

    }else{
      linear_upper();
      linear_under();
      if (NextStep_upper && (*microsec - Update_upper > Interval_upper_cal)) {
        Update_upper   = *microsec;
        NextStep_upper = false;
        for (uint8_t index = 0; index < STEP_NUM; index++) {
          if (upper_cal[index].PUL) {
            if (!upper_ctr[index].ACT) {
              upper_ctr[index].ACT = true;
              ioport.digitalWrite(step_upper[index].ENA, false);
            }
            if(upper_ctr[index].POS == upper_ctr[index].START) ioport.digitalWrite(step_upper[index].DIR, upper_ctr[index].DIR);
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
            //HMI Position refresh here.
            if (nextion_page == 0) {
              if (index == 0) Display("nTX", upper_ctr[0].POS);
              else if (index == 1) Display("nTY", upper_ctr[1].POS);
              else if (index == 2) Display("nTZ", upper_ctr[2].POS);
            }
            digitalWrite(step_upper[index].PUL, false);
          }
        }
      }

      if (NextStep_under && (*microsec - Update_under > Interval_under_cal)) {
        Update_under   = *microsec;
        NextStep_under = false;
        for (uint8_t index = 0; index < STEP_NUM; index++) {
          if (under_cal[index].PUL) {
            if (!under_ctr[index].ACT) {
              under_ctr[index].ACT = true;
              ioport.digitalWrite(step_under[index].ENA, false);
            }
            if(under_ctr[index].POS == under_ctr[index].START) ioport.digitalWrite(step_under[index].DIR, under_ctr[index].DIR);
            digitalWrite(step_under[index].PUL, true);

             under_cal[index].PUL = false;
             if (under_ctr[index].DIR) {
              under_ctr[index].POS += 1;
            } else {
              if (under_ctr[index].POS > 0) under_ctr[index].POS -= 1;
            }
            //HMI Position refresh here.
            if (nextion_page == 0) {
              if (index == 0) Display("nBX", under_ctr[0].POS);
              else if (index == 1) Display("nBY", under_ctr[1].POS);
              else if (index == 2) Display("nBZ", under_ctr[2].POS);
            }
            digitalWrite(step_under[index].PUL, false);
          }
        }
      }

    }

    if(postions_sync  && !sync_cal[0].RUN && !sync_cal[1].RUN && !sync_cal[2].RUN){
      postions_sync = false;
      flushing();
      Update_upper = *microsec;
      Update_under = *microsec;
      Interval_upper_cal = Interval_sync_cal;
      Interval_under_cal = Interval_sync_cal;
    }
    if(!postions_sync && !upper_cal[0].RUN && !upper_cal[1].RUN && !upper_cal[2].RUN && !under_cal[0].RUN && !under_cal[1].RUN && !under_cal[2].RUN) stepmoter_work = false;
    if (nextion_page == 0 && !stepmoter_work) {
      Display("btn_run", stepmoter_work);
      Serial.print("RunTime: ");Serial.println(millis()-run_time);
    }
  }
}  //moter_run()

void flushing(){
  for (uint8_t index = 0; index < STEP_NUM; index++){
    upper_cal[index].PUL   = false;
    under_cal[index].PUL   = false;
    sync_cal[index].PUL    = false;
  }
}

void moter_sleep(unsigned long *microsec) {
  for (uint8_t index = 0; index < STEP_NUM; index++) {
    if (upper_ctr[index].ACT && (*microsec - Update_upper > TIME_UNIT*100)) {
      upper_ctr[index].ACT = false;
      ioport.digitalWrite(step_upper[index].ENA, true);
    }
    if ((under_ctr[index].ACT||sync_ctr[index].ACT) && (*microsec - Update_under > TIME_UNIT*100)) {
      under_ctr[index].ACT = false;
      ioport.digitalWrite(step_under[index].ENA, true);
    }
  }
}  //moter_sleep()

void moter_manual() {
  if (manual_mode) {
    unsigned long manual_sec = millis();
    if (manual_sec - Update_manual > 1) {
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
        if (mn_dir) {
          upper_ctr[moter_number].POS += 1;
        } else {
          if (upper_ctr[moter_number].POS > 0) upper_ctr[moter_number].POS -= 1;
        }
      } else {
        ioport.digitalWrite(step_under[moter_number].ENA, false);
        ioport.digitalWrite(step_under[moter_number].DIR, mn_dir);
        digitalWrite(step_under[moter_number].PUL, true);
        if (mn_dir) {
          under_ctr[moter_number].POS += 1;
        } else {
          if (under_ctr[moter_number].POS > 0) under_ctr[moter_number].POS -= 1;
        }
      }
      Update_manual = manual_sec;
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
  if (Serial_num == 7 && ch == 0x03) {
    Serial_buf[Serial_num] = 0x00;
    Serial.println(Serial_buf);
    command_pros();
    Serial_num = 0;
  } else if (ch == 0x02 && Serial_num < 3) {
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
    if (Serial_buf[5] != '1') manual_mode_work = false;
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
      linear_run();
      run_time = millis();
    } else if (Serial_buf[5] == 'F' && Serial_buf[6] == 'F') {
      stepmoter_work = false;
    }

  } else if (Serial_buf[0] == 'S' && Serial_buf[1] == 'Y' && Serial_buf[2] == 'N' && Serial_buf[3] == 'C') {

    if (Serial_buf[5] == 'O' && Serial_buf[6] == 'N') {
      stepmoter_work = false;
      stepmoter_sync = true;
      postions_sync  = false;
      
      Display("nBX_T", upper_ctr[0].DEST);
      Display("nBY_T", upper_ctr[1].DEST);
      Display("nBZ_T", upper_ctr[2].DEST);
      Display("nBS", SPEED_EXCHANGE_TIME/Interval_upper);
      Display("btn_run", stepmoter_work);
      //좌표 계산 다시
    } else if (Serial_buf[5] == 'F' && Serial_buf[6] == 'F') {
      stepmoter_work = false;
      stepmoter_sync = false;
      Display("btn_run", stepmoter_work);
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
      if(speed_temp>SPEED_MAX_um_min) speed_temp = SPEED_MAX_um_min;
      else if(speed_temp<1) speed_temp = 1;
      Interval_upper     = SPEED_EXCHANGE_TIME/speed_temp;
      Interval_upper_cal = Interval_upper*TIME_UNIT;
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
      if(speed_temp>SPEED_MAX_um_min) speed_temp = SPEED_MAX_um_min;
      Interval_under     = SPEED_EXCHANGE_TIME/speed_temp;
      Interval_under_cal = Interval_under*TIME_UNIT;
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
  unsigned long microsec = micros();
  SerialConnect();
  DisplayConnect();
  moter_run(&microsec);
  moter_sleep(&microsec);
  moter_manual();
  /*
    for (uint8_t index = 0; index < 3; index++) {
    Serial.print("limit sw ");
    Serial.print(index);
    Serial.print(": ");
    Serial.println(digitalRead(pin_in[index]));
    }
  */
}
/********************************************* L O O P *********************************************/
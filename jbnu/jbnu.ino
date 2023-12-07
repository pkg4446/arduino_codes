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
    attachInterrupt(digitalPinToInterrupt(interruptPin), PCA9555::alertISR, LOW); // Set to low for button presses
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
    return HIGH;
  } else {
    return LOW;
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
    return HIGH;
  } else {
    return LOW;
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

/******************************************* STEP  MOTOR *******************************************/
#define STEP_NUM  3
typedef struct STEP_ts{
	int8_t   ENA;
	int8_t   DIR;
	int8_t   PUL;
}STEP_ts;
//uint8_t pin_out[6] = {4, 26, 27, 14, 12, 13};
const STEP_ts step_under[STEP_NUM]={
   {0,1,4},
   {2,3,26},
   {4,5,27},
};
const STEP_ts step_upper[STEP_NUM]={
   {6,7,14},
   {8,9,12},
   {10,11,13},
};

uint32_t  Position[STEP_NUM]  = {0,};
bool      Direction[STEP_NUM] = {0,};
/******************************************* STEP  MOTOR *******************************************/

uint8_t pin_in[3] =  {32, 33, 25};

uint8_t index_led = 0;
boolean led_flage = true;

/******************************************** S E T U P ********************************************/
void setup() {
  Serial.begin(115200);
  ioport.begin();
  ioport.setClock(400000);
  for (uint8_t index = 0; index < STEP_NUM; index++) {
    ioport.pinMode(step_under[index].ENA, OUTPUT);
    ioport.pinMode(step_under[index].DIR, OUTPUT);
    pinMode(step_under[index].PUL, OUTPUT);
    ioport.pinMode(step_upper[index].ENA, OUTPUT);
    ioport.pinMode(step_upper[index].DIR, OUTPUT);
    pinMode(step_upper[index].PUL, OUTPUT);
  }
  for (uint8_t index = 0; index < 3; index++) {
    pinMode(pin_in[index], INPUT_PULLUP);
  }
}
/******************************************** S E T U P ********************************************/
/********************************************* L O O P *********************************************/
void loop() {
  /*
    for (uint8_t index = 0; index < 3; index++) {
    Serial.print("limit sw ");
    Serial.print(index);
    Serial.print(": ");
    Serial.println(digitalRead(pin_in[index]));
    }
  */
  if (led_flage) {
    ioport.digitalWrite(step_under[index_led].ENA, HIGH);
    ioport.digitalWrite(step_under[index_led].DIR, HIGH);
    ioport.digitalWrite(step_upper[index_led].ENA, HIGH);
    ioport.digitalWrite(step_upper[index_led].DIR, HIGH);
    delay(200);
    ioport.digitalWrite(step_under[index_led].ENA, LOW);
    ioport.digitalWrite(step_under[index_led].DIR, LOW);
    ioport.digitalWrite(step_upper[index_led].ENA, LOW);
    ioport.digitalWrite(step_upper[index_led].DIR, LOW);
    if (index_led < STEP_NUM) {
      index_led++;
    } else {
      index_led = 0;
      led_flage = false;
    }
  } else {
    digitalWrite(step_under[index_led].PUL, true);
    delay(200);
    digitalWrite(step_under[index_led].PUL, LOW);
    if (index_led < 6) {
      index_led++;
    } else {
      index_led = 0;
      led_flage = true;
    }
  }
  delay(1000);
}
/********************************************* L O O P *********************************************/
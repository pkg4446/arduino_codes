#define PCA9539_H_
#include "Arduino.h"
#include "Wire.h"

#define DEBUG 1
#define NXP_INPUT      0
#define NXP_OUTPUT     2
#define NXP_INVERT     4
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
    if ((_inputData & (1 << pin)) > 0){
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

void PCA9539::I2CSetValue(uint8_t address, uint8_t reg, uint8_t value){
    Wire.beginTransmission(address);              // setup direction registers
    Wire.write(reg);                              // pointer to configuration register address 0
    Wire.write(value);                            // write config register low byte
    _error = Wire.endTransmission();
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
  for (int8_t index = 0 ; index <= 15 ; index++) {
    ioport.pinMode(index);
  }
}

void loop()
{ 
  for (int8_t index = 0 ; index <= 15 ; index++) {
    if (ioport.digitalRead(index)) {
      Serial.println(index);
    }
  }
}

#include "PCA9555.h"

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
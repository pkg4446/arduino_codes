#pragma once

#include "Arduino.h"

class MOTOR{
	uint8_t driver;
	
  public:
    MOTOR();
    ~MOTOR();
    void begin(uint8_t num);
};
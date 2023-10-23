#ifndef _control
#define _control

#include "pin_setup"

class MOTOR{
	STEP_ts	step_moter[8];
	
public:
	uint16_t shiftInput;
    void begin();

};
#endif
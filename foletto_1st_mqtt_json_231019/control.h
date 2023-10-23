#pragma once

#include "pin_setup.h"

class MOTOR{
	STEP_ts	step_moter;
	uint32_t Position;
	bool     Zero;
	public:
		MOTOR(STEP_ts moter_pins);
		~MOTOR();
		void init();
};
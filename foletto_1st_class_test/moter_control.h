#pragma once

#include "pin_setup.h"

class MOTOR{
	STEP_ts	step_moter;
	uint32_t hight_max;
	uint32_t Position;
	bool     Zero_set;

	public:
		MOTOR();
		~MOTOR();
		void init(STEP_ts moter_pins);
		void set_maximum(uint32_t maximum);
		void status();
		uint32_t get_max();
		uint32_t get_pos();
		bool get_zero_set();
		void run_drive(bool direction, uint8_t limit_sw, uint32_t step, uint8_t acceleration, uint8_t deceleration, uint8_t speed_max, uint16_t speed_min);
};
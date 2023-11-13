#pragma once

#include "pin_setup.h"

class MOTOR{
	uint8_t  DIR;
	uint8_t  PWM;
	uint32_t Position;
	bool     Zero_set;
	uint16_t accel;
	uint16_t decel;
	uint16_t dla_s;
	uint16_t dla_l;

	public:
		MOTOR();
		~MOTOR();
		void init(STEP_ts moter_pins);
		void set_config(uint16_t v_accel, uint16_t v_decel, uint16_t v_dla_s, uint16_t v_dla_l);
		void status();
		void pos_update(bool direction);
		uint32_t get_pos();
		bool 	 get_zero_set();
		uint16_t accel_step();
		uint16_t decel_step();
		uint16_t delay_short();
		uint16_t delay_long();
		void run_drive(bool direction, uint8_t limit_sw, uint32_t step, uint32_t hight_max);
};
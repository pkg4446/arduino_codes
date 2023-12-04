#pragma once

#define NUMBER_OF_SHIFT_CHIPS 2
#define DATA_WIDTH  NUMBER_OF_SHIFT_CHIPS * 8
#define BYTES_VAL_T uint16_t

void shift_regs_init();
BYTES_VAL_T read_shift_regs();
boolean swich_values(uint8_t pin, BYTES_VAL_T pinValues);
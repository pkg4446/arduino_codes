#pragma once
#include "arduino.h"

uint16_t gaussian_range(uint16_t average, uint16_t standard_deviation);
uint16_t get_gaussian(uint16_t average, uint16_t standard_deviation, uint16_t value);

void swap(uint16_t *num_a, uint16_t *num_b);
void perforation(String text);
void spacebar(String text);
void unit_split(uint16_t values, uint16_t divides);
void unit_mm();
void destruct();
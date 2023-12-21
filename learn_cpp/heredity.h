#pragma once
#include "arduino.h"

uint16_t gaussian_range(uint16_t average, uint16_t standard_deviation);
uint16_t get_gaussian(uint16_t average, uint16_t standard_deviation, uint16_t value);
uint16_t change_gender(uint16_t ex_average, uint16_t average, uint16_t standard_deviation, uint16_t value, uint16_t ratio);

String   mutation_string(String mother, String father);
bool     mutation_bool(bool mother, bool father);
uint8_t  mutation_u8(uint8_t mother, uint8_t father);
uint16_t mutation_u16(uint16_t mother, uint16_t father);
bool     heredity_bool(bool mother, bool father, bool dominant);
uint8_t  heredity_u8(uint8_t mother, uint8_t father);
uint16_t heredity_u16(uint16_t mother, uint16_t father);
uint16_t heredity_ratio(uint16_t mother, uint16_t father, bool gender, uint16_t ratio_moter, uint16_t ratio_fater);
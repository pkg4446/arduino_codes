#pragma once
#include <arduino.h>
#define LINE_NUM    90

void make_csv(String *csv_text, String add_data);
void make_csv_text(String *csv_text, String add_data);

uint16_t gaussian_range(uint16_t average, uint16_t standard_deviation);
uint16_t get_gaussian(uint16_t average, uint16_t standard_deviation, uint16_t value);
String   get_progmem(const char* progmem_ptr);
String   spacebar(bool print_flage);

void swap(uint16_t *num_a, uint16_t *num_b);
void paging();
void perforation(String text);
void space(bool korea, String text);
void space_option(bool korea, uint16_t number, String text);
void unit_split(uint16_t values, uint16_t divides);
void unit_mm(void);
void destruct(void);
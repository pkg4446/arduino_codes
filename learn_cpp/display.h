#pragma once
#include "arduino.h"

#include "word.h"
#include "./progmem/interfaces.h"
#include "./progmem/scene.h"

/*************** Display ***************/
void display_cmd();
/*************** Display ***************/
void display_model_err();
/*************** Display ***************/
void display_boot();
/*************** Display ***************/
void display_help_cmd();
/*************** Display ***************/
void display_hour(uint8_t *clock_hours);
/*************** Display ***************/
void display_prologue(String name, bool gender);
/*************** Display ***************/
void display_continue();
/*************** Display ***************/
void cancle_cmd(bool line_break);
/*************** Display ***************/
void display_edu();
/*************** Display ***************/
void display_info();
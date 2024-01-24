#pragma once
#include "arduino.h"
#include "model_hard.h"
#include "model_soft.h"
#include "progmem/scene.h"

void display_hour(uint8_t *clock_hours);
/*************** Display ***************/
void display_newday(uint32_t *calendar, INFO *class_info, STAT *class_stat, MENS *class_mens, CURRENT *class_current);
/*************** Display ***************/
void display_boot();
/*************** Display ***************/
void display_main();
/*************** Display ***************/
void display_explore();
/*************** Display ***************/
void display_edu();
/*************** Display ***************/
void display_info();
/*************** Display ***************/
void display_shop();
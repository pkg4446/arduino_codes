#pragma once
#include "arduino.h"
#include "define.h"
#include "model_hard.h"
#include "model_soft.h"
#include "./progmem/scene.h"

void display_boot();
/*************** Display ***************/
void display_help();
/*************** Display ***************/
void display_hour(uint8_t *clock_hours);
/*************** Display ***************/
void display_newday(uint32_t *calendar, INFO *class_info, STAT *class_stat, MENS *class_mens, CURRENT *class_current);
/*************** Display ***************/
void display_prologue();
/*************** Display ***************/
void cancle_cmd(bool line_break);
/*************** Display ***************/
void display_shelter();
/*************** Display ***************/
void display_explore(bool sweet_home);
void display_explore_move();
/*************** Display ***************/
void display_edu();
/*************** Display ***************/
void display_info();
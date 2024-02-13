#pragma once
#include <arduino.h>
#include "./progmem/interfaces.h"
#include "./progmem/scene.h"
#include "./progmem/words.h"
#include "define.h"
#include "utility.h"

/*************** Display ***************/
void display_cmd();
/*************** Display ***************/
void display_model_err();
/*************** Display ***************/
void display_boot();
/*************** Display ***************/
void display_hash_check();
/*************** Display ***************/
void display_help_cmd();
/*************** Display ***************/
void display_hour(uint8_t *clock_hours);
/*************** Display ***************/
void display_make_assist();
/*************** Display ***************/
void display_make_user();
/*************** Display ***************/
void display_prologue(String name, bool gender);
/*************** Display ***************/
void display_continue();
/*************** Display ***************/
void display_game_help();
/*************** Display ***************/
void cancle_cmd(bool line_break);
/*************** Display ***************/
void display_cmd_main();
/*************** Display ***************/
void display_info();
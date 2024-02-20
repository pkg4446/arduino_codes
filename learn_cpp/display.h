#pragma once
#include <arduino.h>
#include "./progmem/interfaces.h"
#include "./progmem/scene.h"
#include "./progmem/words.h"
#include "define.h"
#include "utility.h"

/*************** Display ***************/
void cancle_cmd(bool line_break);
/*************** Display ***************/
void display_cmd(void);
/*************** Display ***************/
void display_model_err(void);
/*************** Display ***************/
void display_boot(void);
/*************** Display ***************/
void display_hash_check(void);
/*************** Display ***************/
void display_help_cmd(void);
/*************** Display ***************/
void display_time(uint16_t *time_year, uint8_t *time_month, uint8_t *time_day, uint8_t *time_hour);
/*************** Display ***************/
void display_hour(uint8_t *clock_hours);
/*************** Display ***************/
void display_make_assist(void);
/*************** Display ***************/
void display_make_user(void);
/*************** Display ***************/
void display_rest(void);
/*************** Display ***************/
void display_prologue(String name, bool gender);
/*************** Display ***************/
void display_continue(void);
/*************** Display ***************/
void display_game_help(void);
/*************** Display ***************/
void display_cmd_main(void);
/*************** Display ***************/
void display_dungeon(void);
/*************** Display ***************/
void display_coordinate(uint16_t *scene_number);
/*************** Display ***************/
void display_amenity(uint16_t *scene_number);
/*************** Display ***************/
void display_training(uint16_t *scene_number, String name);
/*************** Display ***************/
void display_cmd_invasion(void);
/*************** Display ***************/
void display_info(void);
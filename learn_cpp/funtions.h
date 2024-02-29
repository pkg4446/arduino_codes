#pragma once
#include "model_hard.h"
#include "model_soft.h"
#include "model_status.h"
#include "./progmem/scene_fn.h"
#include "./progmem/words.h"
#include "define.h"
#include "utility.h"
#include "coresys.h"
#include "display.h"
#include "path_string.h"

String  get_model_name(String path);
bool    get_model_gender(String path);
void get_recon(void);
/**************************/
void villager(void);
/**************************/
void prologue_txt(void);
void routine_day_mens(void);
/**************************/
/**************************/
//void routine_day();
void back_to_main(uint16_t *scene_number,uint16_t *time_year, uint8_t *time_month, uint8_t *time_day, uint8_t *time_hour);
void play_main(uint16_t *scene_number,uint16_t scene_command);
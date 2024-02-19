#pragma once
#include "model_hard.h"
#include "model_soft.h"
#include "model_status.h"

#include "define.h"
#include "utility.h"
#include "coresys.h"
#include "display.h"
#include "path_string.h"

void prologue_txt(void);
void routine_day_mens(void);
//void routine_day();
void back_to_main(uint16_t *scene_number,uint16_t *time_year, uint8_t *time_month, uint8_t *time_day, uint8_t *time_hour);
void play_main(uint16_t *scene_number,uint16_t scene_command);
#pragma once
#include "model_hard.h"
#include "model_soft.h"
#include "model_status.h"

#include "utility.h"
#include "coresys.h"
#include "display.h"
#include "path_string.h"

void prologue_txt();
void routines_day(MENS *class_mens, CURRENT *class_current);
void play_main(uint8_t *scene_number);
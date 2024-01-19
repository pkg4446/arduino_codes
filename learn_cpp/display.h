#pragma once
#include "arduino.h"
#include "model_hard.h"
#include "model_soft.h"

void display_newday(uint32_t calendar, INFO *class_info, STAT *class_stat, MENS *class_mens, CURRENT *class_current);
/*************** Display ***************/
const PROGMEM char scene_boot[] = "SD 카드로 부터 정보를 불러옵니다.\nSD카드가 있나요?\n";
const PROGMEM char scene_boot_opt1[] = "2예";
const PROGMEM char scene_boot_opt2[] = "1아니오";
void display_boot();
/*************** Display ***************/
const PROGMEM char scene_main_opt1[] = "100탐험";
const PROGMEM char scene_main_opt2[] = "110교류";
const PROGMEM char scene_main_opt3[] = "120훈련";
const PROGMEM char scene_main_opt4[] = "200상점";
void display_main();
/*************** Display ***************/
void display_explore();
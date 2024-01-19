#pragma once
#include "arduino.h"
#include "model_hard.h"
#include "model_soft.h"

const PROGMEM char scene_sun_rise[] = "캄캄한 밤이다.새벽이 밝아온다.해가 떠오른다.아침이 밝았다.";
const PROGMEM char scene_sun_fall[] = "해가 중천에 떳다.해가 저문다.노을이 진다.어둠이 내렸다.";
void display_hour(uint8_t *clock_hours);
/*************** Display ***************/
void display_newday(uint32_t *calendar, INFO *class_info, STAT *class_stat, MENS *class_mens, CURRENT *class_current);
/*************** Display ***************/
const PROGMEM char scene_boot[] = "SD 카드로 부터 정보를 불러옵니다.\nSD카드가 있나요?\n";
const PROGMEM char scene_boot_opt1[] = "2예";
const PROGMEM char scene_boot_opt2[] = "1아니오";
void display_boot();
/*************** Display ***************/
const PROGMEM char scene_main_opt1[] = "100탐험";
const PROGMEM char scene_main_opt2[] = "110교육";
const PROGMEM char scene_main_opt3[] = "120정보";
const PROGMEM char scene_main_opt4[] = "200상점";
void display_main();
/*************** Display ***************/
const PROGMEM char scene_explore[]  = "모험을 떠납니다.";
void display_explore();
/*************** Display ***************/
const PROGMEM char scene_edu[]      = "교육합니다.";
void display_edu();
/*************** Display ***************/
const PROGMEM char scene_info[]     = "정보를 확인합니다.";
void display_info();
/*************** Display ***************/
const PROGMEM char scene_shop[]     = "상점을 방문합니다.";
void display_shop();
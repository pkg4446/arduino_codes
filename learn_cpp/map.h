#pragma once
#include "arduino.h"
#include "enum.h"
#include "define.h"
#include "./progmem/maps.h"

uint8_t map_xy_axis(uint8_t axis_origine);
uint8_t map_duplication(uint8_t axis_xy, uint8_t duplication_type);
void map_xy_type(uint8_t map_arr[MAP_SIZE_X][MAP_SIZE_Y], uint8_t *axis_x, uint8_t *axis_y, uint8_t type);
void mini_map(uint8_t map_arr[MAP_SIZE_X][MAP_SIZE_Y],uint8_t *player_x,uint8_t *player_y,bool *shelter_f,uint8_t *shelter_x,uint8_t *shelter_y);
void display_map(uint8_t map_area);
void map_move(uint8_t *axis_x, uint8_t *axis_y, uint8_t direction_type, bool player_play);
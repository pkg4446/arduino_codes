#include "map.h"

uint8_t map_moving(uint8_t mpa_gps_xy){
    uint8_t response = 1;
    if(mpa_gps_xy == e_mountain)  response = 3;
    else if(mpa_gps_xy == e_lake || mpa_gps_xy == e_forest) response = 2;
    return response;
}

uint8_t map_xy_axis(uint8_t axis_origine){
    uint8_t response = axis_origine;
    bool axis_xy_add = random(2);
    if(axis_xy_add && axis_origine<9) response = axis_origine + 1;
    else if(!axis_xy_add && axis_origine>0) response = axis_origine - 1;
    else if(axis_origine==9) response = axis_origine - 1;
    else if(axis_origine==0) response = axis_origine + 1;
    return response;
}

uint8_t map_duplication(uint8_t axis_xy, uint8_t duplication_type){
    if(duplication_type == e_mountain){
        if(axis_xy == e_mountain)   return true;
    }if(duplication_type == e_lake){
        if(axis_xy == e_mountain)   return true;
        else if(axis_xy == e_lake)  return true;
    }if(duplication_type == e_forest){
        if(axis_xy == e_mountain)   return true;
        else if(axis_xy == e_lake)  return true;
        else if(axis_xy == e_forest)return true;
    }
    return false;
}

void map_xy_type(uint8_t map_arr[MAP_SIZE_X][MAP_SIZE_Y], uint8_t *axis_x, uint8_t *axis_y, uint8_t type){
    uint8_t axis_x_ans = *axis_x;
    uint8_t axis_y_ans = *axis_y;
    uint8_t escape = 0;
    while (map_duplication(map_arr[axis_x_ans][axis_y_ans],type)){
        axis_x_ans = map_xy_axis(*axis_x);
        axis_y_ans = map_xy_axis(*axis_y);
        if(++escape>8) break;
    }
    *axis_x = axis_x_ans;
    *axis_y = axis_y_ans;
}

void moving_xy(uint8_t moving_direction, uint8_t *gps_x,  uint8_t *gps_y){
  if(moving_direction == 1 || moving_direction == 2 || moving_direction == 3){
    if(*gps_y>0) *gps_y-=1;
    //이동불가 (바다)
  }else if(moving_direction == 9 || moving_direction == 8 || moving_direction == 7){
    if(*gps_y<MAP_SIZE_Y-1) *gps_y+=1;
    //이동불가 (바다)
  }

  if(moving_direction == 1 || moving_direction == 4 || moving_direction == 7){
    if(*gps_x>0) *gps_x-=1;
  }else if(moving_direction == 3 || moving_direction == 6 || moving_direction == 9){
    if(*gps_x<MAP_SIZE_X-1) *gps_x+=1;
  }
}
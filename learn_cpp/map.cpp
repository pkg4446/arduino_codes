#include "map.h"

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

void moving_xy(uint8_t *gps_x,  uint8_t *gps_y, uint8_t moving_direction){
  bool direction_sea = false;
  if(moving_direction == DIRECTION_EAST){
    if(*gps_x == MAP_SIZE_X-1) direction_sea = true;
    else *gps_y+=1;
  }else if(moving_direction == DIRECTION_WAST){
    if(*gps_x == 0) direction_sea = true;
    else *gps_y-=1;
  }else if(moving_direction == DIRECTION_SOUTH){
    if(*gps_y == 0) direction_sea = true;
    else *gps_y-=1;
  }else if(moving_direction == DIRECTION_NORTH){
    if(*gps_y == MAP_SIZE_Y-1) direction_sea = true;
    else *gps_y+=1;
  }
  if(direction_sea){
    //이동불가(바다)
  }
}

void mini_map(uint8_t map_arr[MAP_SIZE_X][MAP_SIZE_Y],bool *shelter_f,uint8_t *shelter_x,uint8_t *shelter_y){
  String response = "";
  for(uint16_t index=0; index<strlen_P(look_maps); index++){
    response += char(pgm_read_byte_near(look_maps+index));
  }
  Serial.println(response);
  for(uint16_t index_y=0; index_y<MAP_SIZE_Y; index_y++){
    for(uint16_t index_x=0; index_x<MAP_SIZE_X; index_x++){
      if(*shelter_f && index_x==*shelter_x && index_x==*shelter_y){
        Serial.print("집");
      }else{
        Serial.print(map_arr[index_x][index_y]);
      }
      
      if(index_x == MAP_SIZE_X-1) Serial.println();
    }
  }
}

void display_map(uint8_t map_area){
  String response = "";
  if(map_area == e_field){
    for(uint16_t index=0; index<strlen_P(area_field); index++){
        response += char(pgm_read_byte_near(area_field+index));
    }
  }else if(map_area == e_mountain){
    for(uint16_t index=0; index<strlen_P(area_mountain); index++){
        response += char(pgm_read_byte_near(area_mountain+index));
    }
  }else if(map_area == e_lake){
    for(uint16_t index=0; index<strlen_P(area_lake); index++){
        response += char(pgm_read_byte_near(area_lake+index));
    }
  }else if(map_area == e_forest){
    for(uint16_t index=0; index<strlen_P(area_forest); index++){
        response += char(pgm_read_byte_near(area_forest+index));
    }
  }else if(map_area == e_beach){
    for(uint16_t index=0; index<strlen_P(area_beach); index++){
        response += char(pgm_read_byte_near(area_beach+index));
    }
  }
  Serial.println(response);
}

void display_map_look(uint8_t map_arr[MAP_SIZE_X][MAP_SIZE_Y], uint8_t *axis_x, uint8_t *axis_y, uint8_t direction_type){
  String response = "";
  bool direction_sea = false;
  uint8_t look_xy[2] = {*axis_x, *axis_y};
  if(direction_type == DIRECTION_EAST){
    if(*axis_x == MAP_SIZE_X-1) direction_sea = true;
    else look_xy[0] = *axis_x+1;
  }else if(direction_type == DIRECTION_WAST){
    if(*axis_x == 0) direction_sea = true;
    else look_xy[0] = *axis_x-1;
  }else if(direction_type == DIRECTION_SOUTH){
    if(*axis_y == 0) direction_sea = true;
    else look_xy[1] = *axis_y-1;
  }else if(direction_type == DIRECTION_NORTH){
    if(*axis_y == MAP_SIZE_Y-1) direction_sea = true;
    else look_xy[1] = *axis_y+1;
  }
  Serial.print(direction_type);
  Serial.print(",");
  Serial.print(look_xy[0]);
  Serial.print(",");
  Serial.println(look_xy[1]);
  if(direction_sea){
    Serial.println("거긴 바다여");
  }else{
    uint8_t map_area = map_arr[look_xy[0]][look_xy[1]];
    if(map_area == e_field){
      for(uint16_t index=0; index<strlen_P(area_field); index++){
          response += char(pgm_read_byte_near(area_field+index));
      }
    }else if(map_area == e_mountain){
      for(uint16_t index=0; index<strlen_P(area_mountain); index++){
          response += char(pgm_read_byte_near(area_mountain+index));
      }
    }else if(map_area == e_lake){
      for(uint16_t index=0; index<strlen_P(area_lake); index++){
          response += char(pgm_read_byte_near(area_lake+index));
      }
    }else if(map_area == e_forest){
      for(uint16_t index=0; index<strlen_P(area_forest); index++){
          response += char(pgm_read_byte_near(area_forest+index));
      }
    }else if(map_area == e_beach){
      for(uint16_t index=0; index<strlen_P(area_beach); index++){
          response += char(pgm_read_byte_near(area_beach+index));
      }
    }
    Serial.println(response);
  }
}
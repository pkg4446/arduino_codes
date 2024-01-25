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

void mini_map(uint8_t map_arr[MAP_SIZE_X][MAP_SIZE_Y],uint8_t *player_x,uint8_t *player_y,bool *shelter_f,uint8_t *shelter_x,uint8_t *shelter_y){
  for(uint16_t index_y=0; index_y<MAP_SIZE_Y; index_y++){
    for(uint16_t index_x=0; index_x<MAP_SIZE_X; index_x++){
      if(index_x == 0){
        for(uint16_t index=0; index<LINE_NUM - (MAP_SIZE_X*2); index++){
          Serial.print(" ");
        }
      }
      uint8_t map_axis_y = MAP_SIZE_Y-index_y-1;
      if(index_x==*player_x && map_axis_y==*player_y){
        Serial.print("P");
      }else if(*shelter_f && index_x==*shelter_x && map_axis_y==*shelter_y){
        Serial.print("H");
      }else{
        if(map_arr[*player_x][*player_y] == e_mountain){
          if(map_arr[index_x][map_axis_y] == e_mountain)   Serial.print("^");
          else if(map_arr[index_x][map_axis_y] == e_lake)  Serial.print("~");
          else if(map_arr[index_x][map_axis_y] == e_forest)Serial.print("#");
          else if(map_arr[index_x][map_axis_y] == e_beach) Serial.print(".");
          else                                          Serial.print("=");
        }else{
          if(index_x>=*player_x-1 && index_x<=*player_x+1 && map_axis_y>=*player_y-1 && map_axis_y<=*player_y+1){
            if(map_arr[index_x][map_axis_y] == e_mountain)   Serial.print("^");
            else if(map_arr[index_x][map_axis_y] == e_lake)  Serial.print("~");
            else if(map_arr[index_x][map_axis_y] == e_forest)Serial.print("#");
            else if(map_arr[index_x][map_axis_y] == e_beach) Serial.print(".");
            else                                          Serial.print("=");
          }else{
            Serial.print("?");
          }
        }
      }
      if(index_x == MAP_SIZE_X-1) Serial.println();
      else Serial.print(" ");
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
  }else if(map_area == e_house){
    for(uint16_t index=0; index<strlen_P(area_house); index++){
        response += char(pgm_read_byte_near(area_house+index));
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
    for(uint16_t index=0; index<strlen_P(look_sea); index++){
      response += char(pgm_read_byte_near(look_sea+index));
    }
  }else{
    uint8_t map_area = map_arr[look_xy[0]][look_xy[1]];
    if(map_area == e_field){
      for(uint16_t index=0; index<strlen_P(look_field); index++){
          response += char(pgm_read_byte_near(look_field+index));
      }
    }else if(map_area == e_mountain){
      for(uint16_t index=0; index<strlen_P(look_mountain); index++){
          response += char(pgm_read_byte_near(look_mountain+index));
      }
    }else if(map_area == e_lake){
      for(uint16_t index=0; index<strlen_P(look_lake); index++){
          response += char(pgm_read_byte_near(look_lake+index));
      }
    }else if(map_area == e_forest){
      for(uint16_t index=0; index<strlen_P(look_forest); index++){
          response += char(pgm_read_byte_near(look_forest+index));
      }
    }else if(map_area == e_beach){
      for(uint16_t index=0; index<strlen_P(look_beach); index++){
          response += char(pgm_read_byte_near(look_beach+index));
      }
    }else if(map_area == e_house){
      for(uint16_t index=0; index<strlen_P(look_house); index++){
          response += char(pgm_read_byte_near(look_house+index));
      }
    }
  }
  Serial.println(response);
}

void map_move(uint8_t *axis_x, uint8_t *axis_y, uint8_t direction_type){
  bool direction_sea = false;
  if(direction_type == DIRECTION_EAST){
    if(*axis_x == MAP_SIZE_X-1) direction_sea = true;
    else *axis_x+=1;
  }else if(direction_type == DIRECTION_WAST){
    if(*axis_x == 0) direction_sea = true;
    else *axis_x-=1;
  }else if(direction_type == DIRECTION_SOUTH){
    if(*axis_y == 0) direction_sea = true;
    else *axis_y-=1;
  }else if(direction_type == DIRECTION_NORTH){
    if(*axis_y == MAP_SIZE_Y-1) direction_sea = true;
    else *axis_y+=1;
  }
  if(direction_sea){
    String response = "";
    for(uint16_t index=0; index<strlen_P(look_sea); index++){
      response += char(pgm_read_byte_near(look_sea+index));
    }
    Serial.println(response);
  }
}
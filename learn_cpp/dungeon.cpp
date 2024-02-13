#include "dungeon.h"

void mapClass::init(void) {
    for (uint8_t index_x = 0; index_x < MAP_X; index_x++){
        for (uint8_t index_y = 0; index_y < MAP_Y; index_y++){
            maps[index_x][index_y] = 0;
        }
    }
}
void mapClass::view(void) {
    for (uint8_t index_y = 0; index_y < MAP_Y; index_y++){
        for (uint8_t index_x = 0; index_x < MAP_X; index_x++){
            Serial.print(maps[index_x][index_y]);
            if(index_x != MAP_X-1)Serial.print(",");
        }
        Serial.println();
    }
}

mapClass playmap;
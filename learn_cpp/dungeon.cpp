#include "dungeon.h"

void mapClass::init(void) {
    for (uint8_t index_y = 0; index_y < MAP_Y; index_y++){
        for (uint8_t index_x = 0; index_x < MAP_X; index_x++){
            maps[index_y][index_x] = 0;
        }
    }
}
void mapClass::view(void) {
    for (uint8_t index_y = 0; index_y < MAP_Y; index_y++){
        for (uint8_t index_x = 0; index_x < MAP_X; index_x++){
            Serial.print(maps[index_y][index_x]);
            if(index_x != MAP_X-1)Serial.print(",");
        }
        Serial.println();
    }
}

bool mapClass::pathfinder(void) {
    bool visited[ROWS][COLS] = {0};
    bool foundPath = 0;

    return true;
}
void mapClass::rebuild(uint8_t axis_x, uint8_t axis_y, uint8_t types) {
    if(types == wall && axis_y == 2 &&(axis_x == 0 || axis_x == MAP_X-1)) return;
    maps[axis_x][axis_y] = types;
}
uint8_t mapClass::get(uint8_t axis_x, uint8_t axis_y) {
    return maps[axis_x][axis_y];
}

mapClass playmap;
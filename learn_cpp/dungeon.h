#include <Arduino.h>
#include "enum.h"

#define MAP_X 7
#define MAP_Y 3

class mapClass
{
  public:
    void    init(void);
    void    view(void);
    bool    pathfinder(void);
    void    rebuild(uint8_t axis_x, uint8_t axis_y, uint8_t types);
    uint8_t get(uint8_t axis_x, uint8_t axis_y);
  private:
    uint8_t maps[MAP_Y][MAP_X];
};

class dungeonClass
{
  public:
  private:
};

extern mapClass playmap;
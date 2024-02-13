#include <Arduino.h>
#include "enum.h"

#define MAP_X 8
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
    uint8_t enter_y;
    uint8_t exit_y;
    uint8_t maze[MAP_Y][MAP_X];
    bool visited[MAP_Y][MAP_X];
    struct Point {
      int row;
      int col;
    };
};

class dungeonClass
{
  public:
  private:
};

extern mapClass playmap;
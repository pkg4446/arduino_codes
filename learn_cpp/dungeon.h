#include <Arduino.h>
#include "enum.h"
#include "utility.h"
const PROGMEM char path_err[] = "외부와 열결 통로는 남겨두어야 합니다.";

#define MAP_X 8
#define MAP_Y 3

class mapClass
{
  public:
    uint8_t maze[MAP_Y][MAP_X];
    void    init(void);
    void    view(void);
    void    rebuild(uint8_t axis_x, uint8_t axis_y, uint8_t types);
    uint8_t get(uint8_t axis_x, uint8_t axis_y);
    uint8_t get_enter(void);
    uint8_t get_exit(void);
  private:
    uint8_t enter_y;
    uint8_t exit_y;
    bool    visited[MAP_Y][MAP_X];
    bool    pathfinder(void);
    struct Point {
      int row;
      int col;
    };
};

class moveClass
{
  public:
    uint8_t gps_x;
    uint8_t gps_y;
    uint8_t init(uint8_t enter);
    void    moving(uint8_t (*maze)[MAP_X]);
    bool    event(void);
  private:
    uint8_t act_point;
    bool    back_move;
    bool    target_x;
    bool    target_y;
    bool    visited[MAP_Y][MAP_X];
};

class dungeonClass
{
  public:
  private:
};

extern mapClass playmap;
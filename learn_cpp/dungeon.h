#include <Arduino.h>
#include "enum.h"
#include "utility.h"
#include "filesys.h"
#include "path_string.h"
#include "./progmem/dungeon_menu.h"

const PROGMEM char path_err[] = "외부와 열결 통로는 남겨두어야 합니다.";

#define MAP_X 8
#define MAP_Y 3

class mapClass
{
  private:
    uint8_t enter_y;
    uint8_t exit_y;
    bool    visited[MAP_Y][MAP_X];
    bool    pathfinder(void);
    void    parse_map(String  text);
    void    save_csv(void);
    struct Point {
      int row;
      int col;
    };

  public:
    uint8_t maze[MAP_Y][MAP_X];
    void    init(void);
    void    load_csv(void);
    void    view(uint8_t axis_x, uint8_t axis_y);
    void    rebuild(uint8_t axis_x, uint8_t axis_y, uint8_t types);
    uint8_t get(uint8_t axis_x, uint8_t axis_y);
    uint8_t get_enter(void);
    uint8_t get_exit(void);
};

class moveClass
{
  private:
    uint8_t act_point;
    bool    back_move;
    bool    target_x;
    bool    target_y;
    bool    visited[MAP_Y][MAP_X];

  public:
    uint8_t gps_x;
    uint8_t gps_y;
    uint8_t init(uint8_t enter);
    void    moving(uint8_t (*maze)[MAP_X]);
    bool    event(void);
};

class dungeonClass
{
  private:
  public:
};

extern mapClass playmap;
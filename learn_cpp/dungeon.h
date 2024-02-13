#include <Arduino.h>

#define MAP_X 7
#define MAP_Y 3

class mapClass
{
  public:
    void    init(void);
    void    view(void);
  private:
    uint8_t maps[MAP_X][MAP_Y];
};

class dungeonClass
{
  public:
  private:

};

extern mapClass playmap;
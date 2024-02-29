#include <Arduino.h>
#include "filesys.h"
#include "define.h"
#include "path_string.h"
#include "display.h"
#include "funtions.h"

void command_dos(char *command_buf, String *path, bool *exit);
/*************************************************************/
void cmd_chs_victim(uint16_t *scene_number, uint8_t model_max_num);
bool path_equal_chk(String path, String target);
void cmd_dng_manage(uint16_t *scene_number, String *path);
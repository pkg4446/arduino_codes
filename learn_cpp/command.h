#include <Arduino.h>
#include "filesys.h"
#include "path_string.h"
#include "display.h"

void command_dos(char *command_buf, String *path, bool *exit);
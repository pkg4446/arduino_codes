#pragma once
#include <arduino.h>
#include "utility.h"
#include "./progmem/path.h"

String path_config(void);
String path_slash(void);
String path_cryobank(void);
String path_avatar(void);
String path_assist(void);
String path_womb(void);
String path_troop(void);
String path_captive(void);
String path_raider(void);
String path_town(void);

String file_time(void);
String file_map(void);
String file_mother(void);
String file_father(void);
String file_hard(void);
String file_soft(void);
String file_mens(void);
String file_current(void);
String file_exp(void);
String file_breed(void);
String file_hash(void);
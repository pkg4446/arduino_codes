#include "path_string.h"
/*******************************************************/
String path_root(){
    return get_progmem(path_root_str);
};
/*******************************************************/
String path_config(){
    return path_root() + get_progmem(path_config_str);
};
/*******************************************************/
String path_cryobank(){
    return path_root() + get_progmem(path_cryobank_str);
};
/*******************************************************/
String path_avatar(){
    return path_root() + get_progmem(path_avatar_str);
};
/*******************************************************/
String path_assist(){
    return path_root() + get_progmem(path_assist_str);
};
/*******************************************************/
String path_womb(){
    return get_progmem(path_womb_str);
};

/*******************************************************/
String file_map(){
    return get_progmem(file_map_str);
};
/*******************************************************/
String file_mother(){
    return get_progmem(file_mother_str);
};
/*******************************************************/
String file_father(){
    return get_progmem(file_father_str);
};
/*******************************************************/
String file_hard(){
    return get_progmem(file_hard_str);
};
/*******************************************************/
String file_soft(){
    return get_progmem(file_soft_str);
};
/*******************************************************/
String file_mens(){
    return get_progmem(file_mens_str);
};
/*******************************************************/
String file_current(){
    return get_progmem(file_current_str);
};
/*******************************************************/
String file_exp(){
    return get_progmem(file_exp_str);
};
/*******************************************************/
String file_hash(){
    return get_progmem(file_hash_str);
};
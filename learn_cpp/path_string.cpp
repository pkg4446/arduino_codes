#include "path_string.h"
/*******************************************************/
String path_root(void){
    return get_progmem(path_root_str);
};
/*******************************************************/
String path_slash(void){
    return "/";
};
/*******************************************************/
String path_config(void){
    return path_root() + get_progmem(path_config_str);
};
/*******************************************************/
String path_cryobank(void){
    return path_root() + get_progmem(path_cryobank_str);
};
/*******************************************************/
String path_avatar(void){
    return path_root() + get_progmem(path_avatar_str);
};
/*******************************************************/
String path_assist(void){
    return path_root() + get_progmem(path_assist_str);
};
/*******************************************************/
String path_womb(void){
    return get_progmem(path_womb_str);
};
/*******************************************************/
String path_troop(void){
    return path_root() + get_progmem(path_troop_str);
};
/*******************************************************/
String path_captive(void){
    return path_root() + get_progmem(path_captive_str);
};
/*******************************************************/
String path_raider(void){
    return path_root() + get_progmem(path_raider_str);
};
/*******************************************************/
String file_time(void){
    return get_progmem(file_time_str);
};
/*******************************************************/
String file_map(void){
    return get_progmem(file_map_str);
};
/*******************************************************/
String file_mother(void){
    return get_progmem(file_mother_str);
};
/*******************************************************/
String file_father(void){
    return get_progmem(file_father_str);
};
/*******************************************************/
String file_hard(void){
    return get_progmem(file_hard_str);
};
/*******************************************************/
String file_soft(void){
    return get_progmem(file_soft_str);
};
/*******************************************************/
String file_mens(void){
    return get_progmem(file_mens_str);
};
/*******************************************************/
String file_current(void){
    return get_progmem(file_current_str);
};
/*******************************************************/
String file_exp(void){
    return get_progmem(file_exp_str);
};
/*******************************************************/
String file_breed(void){
    return get_progmem(file_breed_str);
};
/*******************************************************/
String file_hash(void){
    return get_progmem(file_hash_str);
};
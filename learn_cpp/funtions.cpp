#include "funtions.h"
/*************** funtion ***************/
void prologue_txt(void){
    /***** Model *****/
    /***** Hardware *****/
    INFO      *info_class   = new INFO();
    HEAD      *head_class   = new HEAD();
    BODY      *body_class   = new BODY();
    EROGENOUS *parts_class  = new EROGENOUS();
    /***** Model *****/
    read_model_hard(path_avatar(),info_class,head_class,body_class,parts_class);
    bool gender = info_class->get_gender();
    read_model_hard(path_assist(),info_class,head_class,body_class,parts_class);
    display_prologue(info_class->get_family() + info_class->get_name(), gender);
}
/*************** funtion ***************/
void routine_day_mens(void){
    mens_check(path_assist(),true);
    mens_check(path_avatar(),true);
    //나머지 체크
}
/*************** funtion ***************/
void back_to_main(uint8_t *scene_number){
  *scene_number = 100;
  display_cmd_main();
}
/*************** funtion ***************/
void play_main(uint8_t *scene_number){
  if(*scene_number == COMMAND_DUNGEON)     display_dungeon();
  else if(*scene_number == COMMAND_INFOMATION)  display_info();
  else if(*scene_number == COMMAND_STORE)       ;
  else if(*scene_number == COMMAND_INVASION)    display_cmd_invasion();
  else if(*scene_number == COMMAND_TRAINING)    ;
  else if(*scene_number == COMMAND_REST)        ;
}
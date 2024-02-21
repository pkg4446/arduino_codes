#include "funtions.h"
/*************** play funtion ***************/
String get_model_name(String path){
  /***** Hardware *****/
  INFO      *info_class   = new INFO();
  read_model_hard_info(path_assist(),info_class);
  String response = info_class->get_family()+info_class->get_name();
  delete info_class;
}
/*************** funtion ***************/
void prologue_txt(void){
    /***** Model *****/
    /***** Hardware *****/
    INFO      *info_class   = new INFO();
    /***** Model *****/
    read_model_hard_info(path_avatar(),info_class);
    bool gender = info_class->get_gender();
    read_model_hard_info(path_assist(),info_class);
    display_prologue(info_class->get_family() + info_class->get_name(), gender);
    delete info_class;
}
/*************** funtion ***************/
void routine_day_mens(void){
    mens_check(path_assist(),true);
    mens_check(path_avatar(),true);
    //나머지 체크
}
/*************** funtion ***************/
void back_to_main(uint16_t *scene_number,uint16_t *time_year, uint8_t *time_month, uint8_t *time_day, uint8_t *time_hour){
  *scene_number = COMMAND_MAIN;
  display_time(time_year,time_month,time_day,time_hour);
  display_cmd_main();
}
/*************** funtion ***************/
void play_main(uint16_t *scene_number,uint16_t scene_command){
  if(scene_command == COMMAND_DUNGEON){
    *scene_number = scene_command;
    display_dungeon();
  }else if(scene_command == COMMAND_INFOMATION){
    *scene_number = scene_command;
    display_info();
  }else if(scene_command == COMMAND_STORE){
    *scene_number = scene_command;
  }else if(scene_command == COMMAND_INVASION){
    *scene_number = scene_command;
    display_cmd_invasion();
  }else if(scene_command == COMMAND_TRAINING){
    *scene_number = scene_command;
  }
}
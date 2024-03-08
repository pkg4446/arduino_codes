#include "funtions.h"
/*************** play funtion ***************/
String get_model_name(String path){
  String response = "";
  if(exisits_check(path+file_hard())){
    INFO      *info_class   = new INFO();
    read_model_hard_info(path,info_class);
    response = info_class->get_family()+info_class->get_name();
    delete info_class;
  }
  return response;
}
bool get_model_gender(String path){
  bool response = false;
  if(exisits_check(path+file_hard())){
    INFO      *info_class   = new INFO();
    read_model_hard_info(path,info_class);
    response = info_class->get_gender();
    delete info_class;
  }
  return response;
}
void get_model_name_gender(String path, String *name, bool *gender){
  if(exisits_check(path+file_hard())){
    INFO      *info_class   = new INFO();
    read_model_hard_info(path,info_class);
    *name   = info_class->get_family()+info_class->get_name();
    *gender = info_class->get_gender();
    delete info_class;
  }
}
/*************** play funtion ***************/
void get_recon(void){
  if(!exisits_check(path_town())) return;
  /***** Hardware *****/
  INFO  *info_class = new INFO();
  read_model_hard_info(path_town(),info_class);
  Serial.print(get_progmem(scene_recon_1));
  spacebar();
  if(info_class->get_gender())  Serial.print(get_progmem(word_male));
  else  Serial.print(get_progmem(word_female));
  Serial.print(get_progmem(gramma_ul_2));
  spacebar();
  Serial.println(get_progmem(scene_recon_2));

  Serial.print(get_progmem(word_he));
  if(!info_class->get_gender()) Serial.print(get_progmem(word_she));
  Serial.print(get_progmem(gramma_ui));
  spacebar();
  Serial.print(get_progmem(word_name));
  Serial.print(get_progmem(gramma_un));
  spacebar();

  Serial.print(info_class->get_family()+info_class->get_name());
  spacebar();
  Serial.println(get_progmem(scene_recon_3));
  delete info_class;
}
/*************** funtion ***************/
/*************** funtion ***************/
void villager(void){
  if(!exisits_check(path_town())){
    dir_make(path_town());
    new_model(path_town(),random(2));
  }else if(dir_list(path_town(),false,false) < FILE_AMOUNT){
    new_model(path_town(),random(2));
  }
}
/*************** funtion ***************/
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
    display_invasion();
  }else if(scene_command == COMMAND_TROOP){
    *scene_number = scene_command;
  }
}
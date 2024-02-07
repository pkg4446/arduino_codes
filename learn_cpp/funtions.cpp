#include "funtions.h"
/*************** funtion ***************/
void prologue_txt(){
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
void routines_day(MENS *class_mens, CURRENT *class_current){
    class_mens->daily();
    class_current->daily();
    #ifdef DEBUG
        perforation("daily");
        if(!gender){
            class_mens->status();
            Serial.println(class_mens->get());
        }
        class_current->status();
    #endif
}
/*************** funtion ***************/
void play_main(uint8_t *scene_number){
  if(*scene_number == COMMAND_MAIN)            display_cmd_main();
  else if(*scene_number == COMMAND_INFOMATION) display_info();
  else if(*scene_number == COMMAND_INFOMATION) ;
  else if(*scene_number == COMMAND_INFOMATION) ;
  else if(*scene_number == COMMAND_INFOMATION) ;
}
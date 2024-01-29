#include "filesys.h"

#include "model_hard.h"
#include "model_soft.h"
#include "enum.h"
#include "define.h"
#include "utility.h"
#include "funtions.h"

#include "database.h"
#include "display.h"
#include "path_string.h"

#include "coresys.h"

#include "interface.h"

/***** Player *****/
INFO *info_class        = new INFO();
HEAD *head_class        = new HEAD();
BODY *body_class        = new BODY();
EROGENOUS *parts_class  = new EROGENOUS();
STAT *stat_class        = new STAT();
HOLE *hole_class        = new HOLE();
SENSE *sense_class      = new SENSE();
NATURE *nature_class    = new NATURE();
EROS *eros_class        = new EROS();
/***** Player *****/
/***** Variable *****/
uint32_t  calendar     = 1;
uint8_t   hour_count   = 6;
uint8_t   season       = 0; // month = rand(12);

unsigned long time_clock  = 0UL;
bool          time_stop   = false;
uint32_t      one_hour_sec= ONE_HOUR;

uint8_t       scene_number= 0;

/***** Variable *****/
/*
void test(){
  INFO      *info_gene  = new INFO();
  delete info_gene;
}
*/
/***** funtions ************/
/***** funtion time ************/
bool time_stream(unsigned long millisec){
  bool response = false;
  if(!time_stop && millisec - time_clock > one_hour_sec){
    time_clock = millisec;
    response = true;
  }
  return response;
}
/***** funtion scene ****/
void display_scene(){
  if(scene_number == COMMAND_EDUCATION)       display_edu();
  else if(scene_number == COMMAND_INFOMATION) display_info();
}
/***** funtion command ****/
char command_buf[COMMAND_LENGTH] = {0x00,};
uint8_t command_num = 0;
void get_command(char ch) {
  if(ch=='\n'){
    command_buf[ command_num ] = 0x00;
    command_num = 0;
    command_progress(command_buf);
  }else if(ch!='\r'){
    command_buf[ command_num++ ] = ch;
    command_num %= COMMAND_LENGTH;
  }
}

void command_progress(String recieve){
  Serial.println(recieve);
  if(scene_number == 0){
    uint8_t scene_command = recieve.toInt();
    if(scene_command == 1 || scene_command == 2) scene_number = scene_command;
  }else if(scene_number == 1){
    uint8_t scene_command = recieve.toInt();
    bool select_check = false;
    if(scene_command == COMMAND_EDUCATION || scene_command == COMMAND_INFOMATION){
      scene_number = scene_command;
    }else{

    }
  }
  display_scene();
}
/***** funtion gene *******/
/*
void gene_meiosis(uint8_t child, uint8_t target){
  head_class[target]->   meiosis(head_class_parents[child*2],  head_class_parents[child*2+1]);
  body_class[target]->   meiosis(body_class_parents[child*2],  body_class_parents[child*2+1]);
  parts_class[target]->  meiosis(parts_class_parents[child*2], parts_class_parents[child*2+1]);
  stat_class[target]->   meiosis(stat_class_parents[child*2],  stat_class_parents[child*2+1]);
  hole_class[target]->   meiosis(hole_class_parents[child*2],  hole_class_parents[child*2+1]);
  sense_class[target]->  meiosis(sense_class_parents[child*2], sense_class_parents[child*2+1]);
  nature_class[target]-> meiosis(nature_class_parents[child*2],nature_class_parents[child*2+1]);
  eros_class[target]->   meiosis(eros_class_parents[child*2],  eros_class_parents[child*2+1]);
}

void pregnant(String family_name){
  info_class[e_baby]->generate(random(2), true);
  info_class[e_baby]->set_family(family_name);
  view_status("\nnew baby",info_class[e_baby],head_class[e_baby],body_class[e_baby],parts_class[e_baby],stat_class[e_baby],hole_class[e_baby],sense_class[e_baby],nature_class[e_baby],eros_class[e_baby]);
}
*/
/***** funtion routine ****/
bool routine_hours(){
  bool response = false;
  if(time_stream(millis())){
    if(++hour_count > 23){
      hour_count = 0;
      response = true;
    }else{
      //time spand
      display_hour(&hour_count);
    }
    //here
  }
  return response;
}

void routine_days(){
  if(routine_hours()){
    calendar ++;
    for(uint8_t index=0; index<CLASS_ARRAY; index++){
      //routines_day(info_class[index]->get_gender(),mens_class[index],current_class[index]);
    }
    //display_newday(&calendar,info_class[e_player],stat_class[e_player],mens_class[e_player],current_class[e_player]);
    display_scene();
  }
}
/***** funtions ************/
/***** CORE ****************/
/***** setup ***************/
void setup() {
  Serial.begin(115200);
  #if defined(ESP32)
    randomSeed(analogRead(39));
    Serial.println("ESP32 ver");
  #else
    randomSeed(analogRead(A0));
    Serial.println("ARDUINO ver");
  #endif
  sd_init();
  
  if(!exisits_check(path_avatar())){
    dir_make(path_avatar());
    new_model(random(2),path_avatar());
    Serial.println("new!");
  }
  //////////
  dir_make(path_avatar());
  new_model(random(2),path_avatar());
  Serial.println("new!");
  //////////
  read_model(path_avatar(),info_class,head_class,body_class,parts_class,stat_class,hole_class,sense_class,nature_class,eros_class);

  if(scene_number == 1){
    /***** HARDWARE *****/
    display_prologue();
    scene_number = 100;
  }else{
    scene_number = 100; //get sd card
  }
  
  time_clock = millis();
  //display_newday(&calendar,info_class,stat_class,mens_class,current_class);
  display_scene();
}
/***** loop ****************/
void loop() {
  if (Serial.available()) get_command(Serial.read());
  routine_days();
}
/***** CORE ****************/
#include "filesys.h"

#include "model_hard.h"
#include "model_soft.h"
#include "model_status.h"
#include "enum.h"
#include "define.h"
#include "utility.h"
#include "funtions.h"

#include "database.h"
#include "display.h"
#include "path_string.h"

#include "coresys.h"

String path_current = "/";
bool   dos_mode     = false;
/***** Model *****/
/***** Hardware *****/
INFO      *info_class   = new INFO();
HEAD      *head_class   = new HEAD();
BODY      *body_class   = new BODY();
EROGENOUS *parts_class  = new EROGENOUS();
/***** Software *****/
STAT      *stat_class   = new STAT();
HOLE      *hole_class   = new HOLE();
SENSE     *sense_class  = new SENSE();
NATURE    *nature_class = new NATURE();
EROS      *eros_class   = new EROS();
/***** Status *****/
MENS      *mens_class   = new MENS();
CURRENT   *feel_class   = new CURRENT();
BREED     *breed_class  = new BREED();

/***** Model *****/
/***** Variable *****/
uint32_t  calendar    = 1;
uint8_t   hour_count  = 6;
uint8_t   season      = 0; // month = rand(12);

uint8_t   aggro_point = 0;

unsigned long time_clock  = 0UL;
bool          time_stop   = false;
uint32_t    one_hour_sec  = ONE_HOUR;

uint8_t   scene_number    = 0;
/***** Variable *****/

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
  display_cmd();
  Serial.println(recieve);

  if(dos_mode){
    String temp_text = "";
    for(uint8_t index_check=3; index_check<COMMAND_LENGTH; index_check++){
      if(command_buf[index_check] == 0x00) break;
      temp_text += command_buf[index_check];
    }
    if(command_buf[0]=='h' && command_buf[1]=='e' && command_buf[2]=='l' && command_buf[3]=='p'){
      display_help_cmd();
    }else if(command_buf[0]=='c' && command_buf[1]=='d' && command_buf[2]==0x20){
      uint8_t command_index_check = 3;
      if(exisits_check(path_current+temp_text+"/")){
        path_current += temp_text;
        path_current += "/";
      }
      Serial.println(path_current);
    }else if(command_buf[0]=='c' && command_buf[1]=='d' && command_buf[2]=='/'){
      path_current = "/";
      Serial.println(path_current);
    }else if(command_buf[0]=='l' && command_buf[1]=='s'){
      dir_list(path_current,true,true);
    }else if(command_buf[0]=='m' && command_buf[1]=='d'){
      dir_make(path_current+temp_text);
    }else if(command_buf[0]=='r' && command_buf[1]=='d'){
      dir_remove(path_current+temp_text);
    }else if(command_buf[0]=='r' && command_buf[1]=='f'){
      file_remove(path_current+temp_text);
    }else if(command_buf[0]=='o' && command_buf[1]=='p'){
      Serial.println(file_read(path_current+temp_text));
    }else if(command_buf[0]=='r' && command_buf[1]=='f'){
      file_remove(path_current+temp_text);
    }else if(command_buf[0]=='e' && command_buf[1]=='x' && command_buf[2]=='i' && command_buf[3]=='t'){
      dos_mode = false;
      display_boot();
    }else{
      Serial.println("wrong command!");
    }
  }else if(scene_number == 0){
    uint8_t scene_command = recieve.toInt();
    if(scene_command == 1 || scene_command == 2) scene_number = scene_command;
    else if(scene_command == 99){
      dos_mode = true;
      display_help_cmd();
    }
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
void pregnant(String family_name){
  info_class[e_baby]->generate(random(2), true);
  info_class[e_baby]->set_family(family_name);
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
      //routines_day(mens_class[index],current_class[index]);
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
  display_boot();
  while (scene_number==0)
  {
    if (Serial.available()) get_command(Serial.read());
  }
  if(!exisits_check(path_assist())) dir_make(path_assist());
  if(scene_number==2 || dir_list(path_assist(),false,false) < 8){
    new_model_body(path_assist(),false);
    new_model_status(path_assist(),false);
    display_make_assist();
  }
  if(!exisits_check(path_avatar())) dir_make(path_avatar());
  if(scene_number==2 || dir_list(path_avatar(),false,false) < 8){
    bool gender = random(2);
    new_model_body(path_avatar(),gender);
    new_model_status(path_avatar(),gender);
    display_make_user();
    scene_number = 2;
  }
  //////////
  read_model_hard(path_assist(),info_class,head_class,body_class,parts_class);
  /*
  read_model_soft(path_assist(),stat_class,hole_class,sense_class,nature_class,eros_class);
  read_model_mens(path_assist(),mens_class);
  read_model_feel(path_assist(),feel_class);
  read_model_breed(path_assist(),breed_class);
  */
  
  if(scene_number == 2){
    display_prologue(info_class->get_family() + info_class->get_name(), info_class->get_gender());
    scene_number = 0;
    display_game_help();
    while (scene_number==0)
    {
      if (Serial.available()) get_command(Serial.read());
    }
    if(scene_number == 2){
      Serial.println("설명");
    }
    scene_number = 100;
  }else{
    display_continue();
    scene_number = 100;
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
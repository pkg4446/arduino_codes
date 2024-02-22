#include "filesys.h"

#include "model_hard.h"
#include "model_soft.h"
#include "model_status.h"
#include "enum.h"
#include "define.h"
#include "utility.h"
#include "funtions.h"

#include "dungeon.h"

#include "database.h"
#include "display.h"
#include "path_string.h"

#include "coresys.h"

bool   dos_mode = false;
String path_current;
/***** Model *****/
/***** Hardware *****//*
INFO      *info_class   = new INFO();
HEAD      *head_class   = new HEAD();
BODY      *body_class   = new BODY();
EROGENOUS *parts_class  = new EROGENOUS();
/***** Software *****//*
STAT      *stat_class   = new STAT();
HOLE      *hole_class   = new HOLE();
SENSE     *sense_class  = new SENSE();
NATURE    *nature_class = new NATURE();
EROS      *eros_class   = new EROS();
/***** Status *****//*
MENS      *mens_class   = new MENS();
CURRENT   *feel_class   = new CURRENT();
BREED     *breed_class  = new BREED();
/***** Model *****/
/*
read_model_hard(path_assist(),info_class,head_class,body_class,parts_class);
read_model_soft(path_assist(),stat_class,hole_class,sense_class,nature_class,eros_class);
read_model_mens(path_assist(),mens_class);
read_model_feel(path_assist(),feel_class);
read_model_breed(path_assist(),breed_class);
*/
/***** Variable *****/
uint8_t   aggro_point = 0;
uint32_t  play_time   = 0;
uint8_t   maps[7][3]  = {{0,},};
uint8_t   map_pos_x   = 0;
uint8_t   map_pos_y   = 0;

uint16_t year_count   = 0; // year  = rand(1001);
uint8_t  month_count  = 1; // month = rand(1,13);
uint8_t  day_count    = 1; // day   = rand(1,31);
uint8_t  hour_count   = 0;

unsigned long time_clock  = 0UL;
bool          time_stop   = false;
uint32_t    one_hour_sec  = ONE_HOUR;

uint16_t   scene_number   = 0;
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
/***** inner funtion command progress****/
void command_fn_dos(void){
  String temp_text = "";
  for(uint8_t index_check=3; index_check<COMMAND_LENGTH; index_check++){
    if(command_buf[index_check] == 0x00) break;
    temp_text += command_buf[index_check];
  }
  if(command_buf[0]=='h' && command_buf[1]=='e' && command_buf[2]=='l' && command_buf[3]=='p'){
    display_help_cmd();
  }else if(command_buf[0]=='c' && command_buf[1]=='d' && command_buf[2]==0x20){
    uint8_t command_index_check = 3;
    if(exisits_check(path_current+temp_text+path_slash())){
      path_current += temp_text;
      path_current += path_slash();
    }
    Serial.println(path_current);
  }else if(command_buf[0]=='c' && command_buf[1]=='d' && command_buf[2]=='/'){
    path_current = path_slash();
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
  }
}
/***** inner funtion command progress****/
void command_progress(String recieve){
  display_cmd();
  Serial.println(recieve);
  if(dos_mode){
    command_fn_dos();
  }else{
    uint16_t scene_command = recieve.toInt();
    if(scene_number == 0){
      if(scene_command == COMMAND_YES || scene_command == COMMAND_NO) scene_number = scene_command;
      else if(scene_command == COMMAND_CANCLE){
        dos_mode = true;
        display_help_cmd();
      }
    }else{
      if(scene_number == COMMAND_MAIN){
        if(scene_command == COMMAND_DUNGEON){
          playmap.view(map_pos_x,map_pos_y);
          //path_current = path_slash();
        }else if(scene_command == COMMAND_REST){
          time_clock -= one_hour_sec;
          display_rest();
          back_to_main(&scene_number,&year_count,&month_count,&day_count,&hour_count);
        };
        play_main(&scene_number,scene_command);
      }else if(scene_number == COMMAND_DUNGEON){
        if(scene_command == COMMAND_CANCLE) back_to_main(&scene_number,&year_count,&month_count,&day_count,&hour_count);
        else{
          if(scene_command == COMMAND_COORDINATE)     display_coordinate(&scene_number);
          else if(scene_command == COMMAND_OBSTRUCT)  playmap.rebuild(map_pos_x,map_pos_y,wall);
          else if(scene_command == COMMAND_WAYLAY)    playmap.rebuild(map_pos_x,map_pos_y,road);
          else if(scene_command == COMMAND_AMENITY)   display_amenity(&scene_number);
          else if(scene_command == COMMAND_MANAGEMENT){
            if(!exisits_check(path_current+file_hard())){
              uint8_t model_max_num = dir_list(path_captive(),true,false);
              if(model_max_num == 0){
                display_no_victim();
              }else if(model_max_num == 1){
                path_current = path_captive() + path_slash() + dir_index(path_captive(),true,model_max_num);
                display_management(&scene_number,get_model_name(path_current));
              }else{
                display_victim(&scene_number);
                for(uint8_t index=1; index<=model_max_num; index++){
                  space_option(true,index,get_model_name(path_captive()+path_slash()+dir_index(path_captive(),true,index)));
                }
              }
            }else display_management(&scene_number,get_model_name(path_current));
          }
          if(scene_command!=COMMAND_AMENITY && scene_command!=COMMAND_MANAGEMENT)playmap.view(map_pos_x,map_pos_y);
          play_main(&scene_number,scene_number);
        }
      }else if(scene_number == COMMAND_COORDINATE){
        if(scene_command == COMMAND_CANCLE) play_main(&scene_number,COMMAND_DUNGEON);
        else playmap.pos_move(&map_pos_x,&map_pos_y,scene_command);
      }else if(scene_number == COMMAND_MANAGEMENT){
        if(scene_command == COMMAND_CANCLE) play_main(&scene_number,COMMAND_DUNGEON);
        else{
          if(scene_command == COMMAND_VICTIM){
            display_victim(&scene_number);
            for(uint8_t index=1; index<=dir_list(path_captive(),true,false); index++){
              space_option(true,index,get_model_name(path_captive()+path_slash()+dir_index(path_captive(),true,index)));
              if(index%3 == 0)Serial.println();
            }
          }else if(scene_command == COMMAND_EDUCATION){ ;
          }else if(scene_command == COMMAND_TRANSFER){  ;
          }else if(scene_command == COMMAND_MENU1 || scene_command == COMMAND_MENU2){
            Serial.print(get_model_name(path_current));
            if(scene_command == COMMAND_MENU1){
              display_release();
              uint8_t reduce_aggro = random(10);
              if(aggro_point>reduce_aggro) aggro_point-=reduce_aggro;
              else  aggro_point = 1;
            }else{
              display_execute();
            }
            dir_remove(path_current);
            path_current = path_slash();
            play_main(&scene_number,COMMAND_DUNGEON);
          }
        }
      }else if(scene_number == COMMAND_VICTIM){
        if(scene_command == COMMAND_CANCLE) display_management(&scene_number,get_model_name(path_current));
        else{
          uint8_t model_max_num = dir_list(path_captive(),true,false);
          if(model_max_num != 0){
            if(scene_command>0 && scene_command<=model_max_num) path_current = path_captive() + path_slash() + dir_index(path_captive(),true,scene_command);
            else path_current = path_captive() + path_slash() + dir_index(path_captive(),true,model_max_num);
            display_management(&scene_number,get_model_name(path_current));
          }else play_main(&scene_number,COMMAND_DUNGEON);
        }
      }else if(scene_number == COMMAND_EDUCATION){
        if(scene_command == COMMAND_CANCLE) display_management(&scene_number,get_model_name(path_current));
        else{
        }
      }else if(scene_number == COMMAND_TRANSFER){
        if(scene_command == COMMAND_CANCLE) display_management(&scene_number,get_model_name(path_current));
        else{
        }
      }else if(scene_number == COMMAND_AMENITY){
        if(scene_command == COMMAND_CANCLE) play_main(&scene_number,COMMAND_DUNGEON);
        else{
          if(scene_command == COMMAND_MENU1)      playmap.rebuild(map_pos_x,map_pos_y,trap);
          else if(scene_command == COMMAND_MENU2) playmap.rebuild(map_pos_x,map_pos_y,prison);
          else if(scene_command == COMMAND_MENU3) playmap.rebuild(map_pos_x,map_pos_y,spa);
          else if(scene_command == COMMAND_MENU4) playmap.rebuild(map_pos_x,map_pos_y,inn);
          else if(scene_command == COMMAND_MENU5) playmap.rebuild(map_pos_x,map_pos_y,farm);
          else if(scene_command == COMMAND_MENU6) playmap.rebuild(map_pos_x,map_pos_y,cage);
          else if(scene_command == COMMAND_MENU7) playmap.put_enter(map_pos_y);
          else if(scene_command == COMMAND_MENU8) playmap.put_exit(map_pos_y);
          playmap.view(map_pos_x,map_pos_y);
        }
      }else if(scene_number == COMMAND_INFOMATION){
        if(scene_command == COMMAND_CANCLE) back_to_main(&scene_number,&year_count,&month_count,&day_count,&hour_count);
      }else if(scene_number == COMMAND_STORE){
        if(scene_command == COMMAND_CANCLE) back_to_main(&scene_number,&year_count,&month_count,&day_count,&hour_count);
      }else if(scene_number == COMMAND_INVASION){
        if(scene_command == COMMAND_CANCLE) back_to_main(&scene_number,&year_count,&month_count,&day_count,&hour_count);
        else{
          villager();
          if(scene_command == COMMAND_MENU1){;
            get_recon();
          }else if(scene_command==COMMAND_MENU2 || scene_command==COMMAND_MENU3){
            uint8_t grown_aggro  = random(10);
            String villager_name = get_model_name(path_town());
            Serial.print(villager_name);
            if(aggro_point+grown_aggro<255) aggro_point+=grown_aggro;
            else aggro_point = 255;
            if(scene_command == COMMAND_MENU2){
              dir_move(path_town(),path_captive()+path_slash()+read_hash_text(path_town()));
              display_villager_kidnap();
            }else{
              dir_remove(path_town());
              display_villager_attack();
            }
          }
          villager();
        }
      }else if(scene_number == COMMAND_TRAINING){
        if(scene_command == COMMAND_CANCLE) back_to_main(&scene_number,&year_count,&month_count,&day_count,&hour_count);
      }
    }
  }
}
/***** funtion routine ****/
bool routine_hour(void){
  bool response = false;
  if(time_stream(millis())){
    save_time_csv(&year_count,&month_count,&day_count,&hour_count);
    raider_new();
    play_time++;
    if(++hour_count > 23){
      hour_count = 0;
      response = true;
    }
    //hour routine is here
    display_hour(&hour_count);
  }
  return response;
}
void routine_day(void){
  if(routine_hour()){
    if(++day_count > 30){
      day_count = 1;
      if(++month_count > 12){
        month_count = 1;
        year_count++;
      }
    }
    //day routine is here
    routine_day_mens();
    //display_newday(&month_count,info_class[e_player],stat_class[e_player],mens_class[e_player],current_class[e_player]);
  }
}

void raider_new(void){
  if(aggro_point==0) aggro_point = 1;
  if(exisits_check(path_raider())){
    raider_move();
  }else if(random(255/aggro_point) == 0){
    if(!exisits_check(path_raider())) dir_make(path_raider());
    new_model(path_raider(),random(2));
    raider_move();
  }
}
void raider_move(void){
  moveClass *raider  = new moveClass();
  uint8_t move_point = raider->init(playmap.get_enter());
  for(uint8_t index=0; index<move_point; index++){
    uint8_t event = raider->moving(playmap.maze, playmap.get_exit());
    if(event == COMMAND_MAIN){
      break;
    }
    Serial.println(event);
  }
  delete raider;
}
void villager(void){
  if(!exisits_check(path_town())){
    dir_make(path_town());
    new_model(path_town(),random(2));
  }else if(dir_list(path_town(),false,false) < FILE_AMOUNT){
    new_model(path_town(),random(2));
  }
}
/***** funtions ************/
/***** CORE ****************/
/***** setup ***************/
void setup(void) {
  Serial.begin(115200);
  path_current = path_slash();
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
  if(!exisits_check(path_assist())){
    dir_make(path_assist());
  }else if(scene_number!=COMMAND_YES){
    display_hash_check();
    check_model_hash(path_assist(),0);
    check_model_hash(path_assist(),1);
    check_model_hash(path_assist(),2);
  }
  if(!exisits_check(path_avatar())){
    dir_make(path_avatar());
  }else if(scene_number!=COMMAND_YES){
    display_hash_check();
    check_model_hash(path_avatar(),0);
    check_model_hash(path_avatar(),1);
    check_model_hash(path_avatar(),2);
  }
  if(dir_list(path_assist(),false,false) < FILE_AMOUNT){
    new_model(path_assist(),false);
    display_make_assist();
  }
  if(dir_list(path_avatar(),false,false) < FILE_AMOUNT){
    bool gender = random(2);
    new_model(path_avatar(),gender);
    display_make_user();
    scene_number = COMMAND_YES;
  } 
  if(scene_number == COMMAND_YES){
    dir_remove(path_config());
    prologue_txt();
    scene_number = 0;
    display_game_help();
    while (scene_number==0)
    {
      if (Serial.available()) get_command(Serial.read());
    }
    if(scene_number == COMMAND_YES){
      Serial.println("설명");
    }
    scene_number = COMMAND_MAIN;
  }else{
    display_continue();
    scene_number = COMMAND_MAIN;
  }
  if(!exisits_check(path_config()))   dir_make(path_config());
  if(!exisits_check(path_troop()))    dir_make(path_troop());
  if(!exisits_check(path_captive()))  dir_make(path_captive());
  playmap.init();
  load_time_csv(&year_count,&month_count,&day_count,&hour_count);
  time_clock = millis();

  //pregnant_baby(path_assist(),path_avatar(),random(2));
  //dir_move(path_assist()+"/womb","/baby");

  //display_newday(&month_count,info_class,stat_class,mens_class,current_class);
  back_to_main(&scene_number,&year_count,&month_count,&day_count,&hour_count);
}
/***** loop ****************/
void loop(void) {
  if (Serial.available()) get_command(Serial.read());
  routine_day();
}
/***** CORE ****************/
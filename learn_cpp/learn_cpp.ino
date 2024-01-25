#include "model_hard.h"
#include "model_soft.h"
#include "enum.h"
#include "define.h"
#include "utility.h"
#include "funtions.h"

#include "database.h"
#include "display.h"
#include "map.h"

#include "interface.h"

/***** HARDWARE *****/
INFO      *info_class[CLASS_ARRAY];
HEAD      *head_class[CLASS_ARRAY];
BODY      *body_class[CLASS_ARRAY];
EROGENOUS *parts_class[CLASS_ARRAY];
STAT      *stat_class[CLASS_ARRAY];
HOLE      *hole_class[CLASS_ARRAY];
SENSE     *sense_class[CLASS_ARRAY];
NATURE    *nature_class[CLASS_ARRAY];
EROS      *eros_class[CLASS_ARRAY];
INFO      *info_class_parents[CLASS_ARRAY*2];
HEAD      *head_class_parents[CLASS_ARRAY*2];
BODY      *body_class_parents[CLASS_ARRAY*2];
EROGENOUS *parts_class_parents[CLASS_ARRAY*2];
STAT      *stat_class_parents[CLASS_ARRAY*2];
HOLE      *hole_class_parents[CLASS_ARRAY*2];
SENSE     *sense_class_parents[CLASS_ARRAY*2];
NATURE    *nature_class_parents[CLASS_ARRAY*2];
EROS      *eros_class_parents[CLASS_ARRAY*2];
/***** SOFTWARE *****/
EXP       *exp_class[CLASS_ARRAY];
CURRENT   *current_class[CLASS_ARRAY];
MENS      *mens_class[CLASS_ARRAY];
BREED     *breed_class[CLASS_ARRAY];
/***** HARDWARE *****/
/***** Variable *****/
uint32_t  calendar     = 1;
uint8_t   hour_count   = 6;
uint8_t   season       = 0; // month = rand(12);

uint8_t   maps[MAP_SIZE_X][MAP_SIZE_Y]  = {0,};
uint8_t   gps_player[2]   = {MAP_SIZE_X/2,MAP_SIZE_Y/2};
uint8_t   gps_shelter[2]  = {MAP_SIZE_X,MAP_SIZE_Y};
bool      shelter         = false;
uint8_t   gps_npc[2]      = {MAP_SIZE_X/2,MAP_SIZE_Y/2};
bool      npc_alive       = false;
bool      incounter_npc   = false;
bool      incounter_player= false;

// 탐험 = 사냥, 채집, 화전
// shelter, farm,

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
/***** funtion map ************/
void display_map(){
  for(uint8_t index_y=0; index_y<MAP_SIZE_Y; index_y++){
    for(uint8_t index_x=0; index_x<MAP_SIZE_X; index_x++){
      Serial.print(maps[index_x][index_y]);
      if(index_x == 9) Serial.println();
    }
  }
}
void map_generate(){
  uint8_t axis_x    = random(MAP_SIZE_X);
  uint8_t axis_y    = random(MAP_SIZE_Y);
  uint8_t terrain   = random(1,MAP_SIZE_X*MAP_SIZE_Y/10);
  maps[axis_x][axis_y] = e_mountain;
  for(uint8_t index=0; index<=terrain; index++){
    map_xy_type(maps,&axis_x,&axis_y,e_mountain);
    maps[axis_x][axis_y] = e_mountain;
  }
  terrain   = random(1,MAP_SIZE_X*MAP_SIZE_Y/20);
  for(uint8_t index=0; index<=terrain; index++){
    map_xy_type(maps,&axis_x,&axis_y,e_lake);
    maps[axis_x][axis_y] = e_lake;
  }
  terrain = random(1,MAP_SIZE_X*MAP_SIZE_Y/3);
  while (map_duplication(maps[axis_x][axis_y],e_forest)){
    axis_x  = random(10);
    axis_y  = random(10);
  }
  maps[axis_x][axis_y] = e_forest;
  for(uint8_t index=0; index<=terrain; index++){
    map_xy_type(maps,&axis_x,&axis_y,e_forest);
    maps[axis_x][axis_y] = e_forest;
  }
  for(uint8_t index=0; index<10; index++){
    if(!map_duplication(maps[index][0],e_forest)) maps[index][0] = e_beach;
    if(!map_duplication(maps[index][9],e_forest)) maps[index][9] = e_beach;
    if(!map_duplication(maps[0][index],e_forest)) maps[0][index] = e_beach;
    if(!map_duplication(maps[9][index],e_forest)) maps[9][index] = e_beach;
  }
  gps_player[0] = random(MAP_SIZE_X);
  gps_player[1] = random(MAP_SIZE_Y);
}
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
  if(scene_number == COMMAND_HOME)              display_shelter();
  else if(scene_number == COMMAND_EXPLORE)      display_explore(shelter);
  else if(scene_number == COMMAND_EXPLORE_MOVE) display_explore_move();
  else if(scene_number == COMMAND_EDUCATION)    display_edu();
  else if(scene_number == COMMAND_INFOMATION)   display_info();
}
/***** funtion incounter ****/
void npc_moving(){
  uint8_t npc_move_direction = DIRECTION_EAST;
  uint8_t npc_direction_dice = random(4);
  if(npc_direction_dice==1)      npc_move_direction = DIRECTION_WAST;
  else if(npc_direction_dice==2) npc_move_direction = DIRECTION_SOUTH;
  else if(npc_direction_dice==3) npc_move_direction = DIRECTION_NORTH;
  map_move(&gps_npc[0],&gps_npc[1],npc_move_direction, false);
}

bool player_npc_gps_check(){
  return (gps_player[0] == gps_npc[0] && gps_player[1] == gps_npc[1]);
}

void models_runout(){
  if(incounter_npc){
    incounter_player = false;
    incounter_npc    = false;
    Serial.println("npc 도망!");
  }else if(incounter_player){
    incounter_player = false;
    incounter_npc    = false;
    Serial.println("player 도망!, 섬뜩한 느낌이 들었다.");
  }
}

void models_incounter(bool model_type){
  if(model_type){
    if(gps_player[0] != gps_npc[0] || gps_player[1] != gps_npc[1]){
      models_runout();
    }else if(player_npc_gps_check() && (random(TRACKING_RATIO) != 0)){
      incounter_npc = true;
      //npc to player incount!
      //dice
      Serial.println("npc 출현!");
    }
  }else{
    if(gps_player[0] != gps_npc[0] || gps_player[1] != gps_npc[1]){
      models_runout();
    }else if(player_npc_gps_check() && (random(TRACKING_RATIO) != 0)){
      Serial.println("player 발견!");
      incounter_player = true;
    }
  }
}
/***** funtion command ****/
char command_buf[COMMAND_LENGTH] = {0x00,};
uint8_t command_num = 0;
void get_command(char ch) {
  if(ch=='\n'){
    command_num = 0;
    command_progress(command_buf);
    for(uint8_t index=0; index<COMMAND_LENGTH; index++) command_buf[index] = 0x00;
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
    if(scene_command == COMMAND_EXPLORE || scene_command == COMMAND_EDUCATION || scene_command == COMMAND_INFOMATION){
      scene_number = scene_command;
    }else{

    }
  }else if(scene_number == COMMAND_EXPLORE){
    uint8_t scene_command = recieve.toInt();
    if(scene_command == EXPLORE_AROUND){
      display_map(maps[gps_player[0]][gps_player[1]]);
      random_incounter();
    }else if(scene_command == EXPLORE_MOVE){
      mini_map(maps,&gps_player[0],&gps_player[1],&shelter,&gps_shelter[0],&gps_shelter[1]);
      scene_number = COMMAND_EXPLORE_MOVE;
    }else if(scene_command == EXPLORE_SHELTER){
      if(shelter){

      }else{

      }
    }
  }else if(scene_number == COMMAND_EXPLORE_MOVE){
    uint8_t scene_command = recieve.toInt();
    if(scene_command == COMMAND_CANCLE){
      scene_number = COMMAND_EXPLORE;
    }else{
      map_move(&gps_player[0],&gps_player[1],scene_command, true);
      models_incounter(true);
      mini_map(maps,&gps_player[0],&gps_player[1],&shelter,&gps_shelter[0],&gps_shelter[1]);
      temp_fn_npc_gps();
      display_map(maps[gps_player[0]][gps_player[1]]);
    }
  }
  display_scene();
}
/***** funtion gene *******/
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
void gene_blended(uint8_t child){
  info_class[child]->   generate(random(2), true);
  info_class[child]->   set_family(info_class_parents[child]->get_family());
  head_class[child]->   blend(head_class_parents[child*2],   head_class_parents[child*2+1]);
  body_class[child]->   blend(body_class_parents[child*2],   body_class_parents[child*2+1]);
  parts_class[child]->  blend(parts_class_parents[child*2],  parts_class_parents[child*2+1]);
  stat_class[child]->   blend(stat_class_parents[child*2],   stat_class_parents[child*2+1]);
  hole_class[child]->   blend(hole_class_parents[child*2],   hole_class_parents[child*2+1]);
  sense_class[child]->  blend(sense_class_parents[child*2],  sense_class_parents[child*2+1]);
  nature_class[child]-> blend(nature_class_parents[child*2], nature_class_parents[child*2+1]);
  eros_class[child]->   blend(eros_class_parents[child*2],   eros_class_parents[child*2+1]);
}
void pregnant(String family_name){
  info_class[e_baby]->generate(random(2), true);
  info_class[e_baby]->set_family(family_name);
  view_status("\nnew baby",info_class[e_baby],head_class[e_baby],body_class[e_baby],parts_class[e_baby],stat_class[e_baby],hole_class[e_baby],sense_class[e_baby],nature_class[e_baby],eros_class[e_baby]);
}
void random_incounter(){
  models_incounter(true);
  uint8_t random_counter = random(10);
  uint8_t incounter_cate = incounter_area(maps[gps_player[0]][gps_player[1]], random_counter);
  Serial.println("없네?");
}
void new_npc(){
  npc_alive = true;
  bool gender;
  #ifdef WOMAN_ONLY
    gender = false;
  #else
    gender = random(2);
  #endif
  new_model(false,info_class_parents[e_npc*2],head_class_parents[e_npc*2],body_class_parents[e_npc*2],parts_class_parents[e_npc*2],stat_class_parents[e_npc*2],hole_class_parents[e_npc*2],sense_class_parents[e_npc*2],nature_class_parents[e_npc*2],eros_class_parents[e_npc*2]);
  new_model(true,info_class_parents[e_npc*2+1],head_class_parents[e_npc*2+1],body_class_parents[e_npc*2+1],parts_class_parents[e_npc*2+1],stat_class_parents[e_npc*2+1],hole_class_parents[e_npc*2+1],sense_class_parents[e_npc*2+1],nature_class_parents[e_npc*2+1],eros_class_parents[e_npc*2+1]);
  gene_blended(e_npc);
  //view_status("\nnew NPC mother",info_class_parents[e_npc*2],head_class_parents[e_npc*2],body_class_parents[e_npc*2],parts_class_parents[e_npc*2],stat_class_parents[e_npc*2],hole_class_parents[e_npc*2],sense_class_parents[e_npc*2],nature_class_parents[e_npc*2],eros_class_parents[e_npc*2]);
  //view_status("\nnew NPC father",info_class_parents[e_npc*2+1],head_class_parents[e_npc*2+1],body_class_parents[e_npc*2+1],parts_class_parents[e_npc*2+1],stat_class_parents[e_npc*2+1],hole_class_parents[e_npc*2+1],sense_class_parents[e_npc*2+1],nature_class_parents[e_npc*2+1],eros_class_parents[e_npc*2+1]);
  //view_status("\nnew NPC",info_class[e_npc],head_class[e_npc],body_class[e_npc],parts_class[e_npc],stat_class[e_npc],hole_class[e_npc],sense_class[e_npc],nature_class[e_npc],eros_class[e_npc]);
}
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
    if(random(6)==0){ //npc move around
      npc_moving();
      models_incounter(false);
      temp_fn_npc_gps();
    }
  }
  return response;
}
void temp_fn_npc_gps(){
  Serial.print("npc gps ");
  Serial.print(gps_npc[0]);
  Serial.print(",");
  Serial.print(gps_npc[1]);
  Serial.print(" : player gps ");
  Serial.print(gps_player[0]);
  Serial.print(",");
  Serial.println(gps_player[1]);
}

void routine_days(){
  if(routine_hours()){
    calendar ++;
    for(uint8_t index=0; index<CLASS_ARRAY; index++){
      routines_day(info_class[index]->get_gender(),mens_class[index],current_class[index]);
    }
    display_newday(&calendar,info_class[e_player],stat_class[e_player],mens_class[e_player],current_class[e_player]);
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
  display_boot();
  while (scene_number==0)
  {
    if (Serial.available()) get_command(Serial.read());
  }
  if(scene_number == 1){
    for(uint8_t index=0; index<CLASS_ARRAY; index++){
      info_class[index]   = new INFO();
      head_class[index]   = new HEAD();
      body_class[index]   = new BODY();
      parts_class[index]  = new EROGENOUS();
      stat_class[index]   = new STAT();
      hole_class[index]   = new HOLE();
      sense_class[index]  = new SENSE();
      nature_class[index] = new NATURE();
      eros_class[index]   = new EROS();
      /*************************************/
      mens_class[index]   = new MENS();
      current_class[index]= new CURRENT();
      exp_class[index]    = new EXP();
      breed_class[index]  = new BREED();
      mens_class[index]->   generate();
      current_class[index]->generate();
      exp_class[index]->    generate();
      breed_class[index]->  generate();
    }
    for(uint8_t index=0; index<CLASS_ARRAY*2; index++){
      info_class_parents[index]   = new INFO();
      head_class_parents[index]   = new HEAD();
      body_class_parents[index]   = new BODY();
      parts_class_parents[index]  = new EROGENOUS();
      stat_class_parents[index]   = new STAT();
      hole_class_parents[index]   = new HOLE();
      sense_class_parents[index]  = new SENSE();
      nature_class_parents[index] = new NATURE();
      eros_class_parents[index]   = new EROS();
    }
    /***** HARDWARE *****/
    new_model(false,info_class_parents[e_player*2],head_class_parents[e_player*2],body_class_parents[e_player*2],parts_class_parents[e_player*2],stat_class_parents[e_player*2],hole_class_parents[e_player*2],sense_class_parents[e_player*2],nature_class_parents[e_player*2],eros_class_parents[e_player*2]);
    new_model(true,info_class_parents[e_player*2+1],head_class_parents[e_player*2+1],body_class_parents[e_player*2+1],parts_class_parents[e_player*2+1],stat_class_parents[e_player*2+1],hole_class_parents[e_player*2+1],sense_class_parents[e_player*2+1],nature_class_parents[e_player*2+1],eros_class_parents[e_player*2+1]);
    gene_blended(e_player);
    new_npc();
    /***** HARDWARE *****/
    display_prologue();
    scene_number = 100;
  }else{
    scene_number = 100; //get sd card
  }
  time_clock = millis();
  display_newday(&calendar,info_class[e_player],stat_class[e_player],mens_class[e_player],current_class[e_player]);
  map_generate();
  display_scene();
}
/***** loop ****************/
void loop() {
  if (Serial.available()) get_command(Serial.read());
  routine_days();
}
/***** CORE ****************/
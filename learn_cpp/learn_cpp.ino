#include "model_hard.h"
#include "model_soft.h"
#include "utility.h"
#include "database.h"
#include "funtions.h"

#define WOMAN_ONLY
//#define ESP32_CORE
#define CLASS_ARRAY 4
enum models{e_player, e_pet, e_baby, e_npc};
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
/*
void test(){
  INFO      *info_gene  = new INFO();
  delete info_gene;
}
*/
/***** funtions *****/
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
  bool gender = false;
  #ifdef WOMAN_ONLY
    gender = false;
  #else
    gender = random(2);
  #endif
  new_model(false,info_class_parents[e_npc*2],head_class_parents[e_npc*2],body_class_parents[e_npc*2],parts_class_parents[e_npc*2],stat_class_parents[e_npc*2],hole_class_parents[e_npc*2],sense_class_parents[e_npc*2],nature_class_parents[e_npc*2],eros_class_parents[e_npc*2]);
  new_model(true,info_class_parents[e_npc*2+1],head_class_parents[e_npc*2+1],body_class_parents[e_npc*2+1],parts_class_parents[e_npc*2+1],stat_class_parents[e_npc*2+1],hole_class_parents[e_npc*2+1],sense_class_parents[e_npc*2+1],nature_class_parents[e_npc*2+1],eros_class_parents[e_npc*2+1]);
  gene_blended(e_npc);
  
  view_status("\nnew NPC mother",info_class_parents[e_npc*2],head_class_parents[e_npc*2],body_class_parents[e_npc*2],parts_class_parents[e_npc*2],stat_class_parents[e_npc*2],hole_class_parents[e_npc*2],sense_class_parents[e_npc*2],nature_class_parents[e_npc*2],eros_class_parents[e_npc*2]);
  view_status("\nnew NPC father",info_class_parents[e_npc*2+1],head_class_parents[e_npc*2+1],body_class_parents[e_npc*2+1],parts_class_parents[e_npc*2+1],stat_class_parents[e_npc*2+1],hole_class_parents[e_npc*2+1],sense_class_parents[e_npc*2+1],nature_class_parents[e_npc*2+1],eros_class_parents[e_npc*2+1]);
  view_status("\nnew NPC",info_class[e_npc],head_class[e_npc],body_class[e_npc],parts_class[e_npc],stat_class[e_npc],hole_class[e_npc],sense_class[e_npc],nature_class[e_npc],eros_class[e_npc]);
}
/***** funtions *****/
void setup() {
  Serial.begin(115200);
  #ifdef ESP32_CORE
    randomSeed(analogRead(39));
    Serial.println("ESP32");
  #else
    randomSeed(analogRead(A0));
    Serial.println("ARDUINO");
  #endif
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
    exp_class[index]      = new EXP();
    current_class[index]  = new CURRENT();
    mens_class[index]     = new MENS();
    breed_class[index]    = new BREED();
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
  /***** HARDWARE *****/
  view_status("\nplayer mother",info_class_parents[e_player*2],head_class_parents[e_player*2],body_class_parents[e_player*2],parts_class_parents[e_player*2],stat_class_parents[e_player*2],hole_class_parents[e_player*2],sense_class_parents[e_player*2],nature_class_parents[e_player*2],eros_class_parents[e_player*2]);
  view_status("\nplayer father",info_class_parents[e_player*2+1],head_class_parents[e_player*2+1],body_class_parents[e_player*2+1],parts_class_parents[e_player*2+1],stat_class_parents[e_player*2+1],hole_class_parents[e_player*2+1],sense_class_parents[e_player*2+1],nature_class_parents[e_player*2+1],eros_class_parents[e_player*2+1]);
  view_status("\nPlayer",info_class[e_player],head_class[e_player],body_class[e_player],parts_class[e_player],stat_class[e_player],hole_class[e_player],sense_class[e_player],nature_class[e_player],eros_class[e_player]);
  
  random_incounter();
}

void loop() {
  // put your main code here, to run repeatedly:
  // newface();
}
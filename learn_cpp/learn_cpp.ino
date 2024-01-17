#include "model_hard.h"
#include "model_soft.h"
#include "utility.h"
#include "database.h"
#include "funtions.h"

#define WOMAN_ONLY
//#define ESP32_CORE
#define CLASS_ARRAY 12
enum models{
  e_player, e_pet, e_baby, e_npc, e_player_mother, e_player_father, e_pet_mother, e_pet_father, e_baby_mother, e_baby_father, e_npc_mother, e_npc_father
};
/***** GENE *********/
INFO      *info_class[CLASS_ARRAY];
HEAD      *head_class[CLASS_ARRAY];
BODY      *body_class[CLASS_ARRAY];
EROGENOUS *parts_class[CLASS_ARRAY];
STAT      *stat_class[CLASS_ARRAY];
HOLE      *hole_class[CLASS_ARRAY];
SENSE     *sense_class[CLASS_ARRAY];
NATURE    *nature_class[CLASS_ARRAY];
EROS      *eros_class[CLASS_ARRAY];
/***** GENE *********/
/*
void meiosis(){
  INFO      *info_gene  = new INFO();
  HEAD      *head_gene  = new HEAD();
  BODY      *body_gene  = new BODY();
  EROGENOUS *parts_gene = new EROGENOUS();
  STAT      *stat_gene  = new STAT();
  HOLE      *hole_gene  = new HOLE();
  SENSE     *sense_gene = new SENSE();
  NATURE    *nature_gene = new NATURE();
  EROS      *eros_gene  = new EROS();
  delete info_gene;
  delete head_gene;
  delete body_gene;
  delete parts_gene;
  delete stat_gene;
  delete hole_gene;
  delete sense_gene;
  delete nature_gene;
  delete eros_gene;
}
*/
/***** funtions *****/
void meiosis(){
  head_class[e_baby_father]->   meiosis(head_class[e_player_mother], head_class[e_player_father]);
  body_class[e_baby_father]->   meiosis(body_class[e_player_mother],body_class[e_player_father]);
  parts_class[e_baby_father]->  meiosis(parts_class[e_player_mother],parts_class[e_player_father]);
  stat_class[e_baby_father]->   meiosis(stat_class[e_player_mother], stat_class[e_player_father]);
  hole_class[e_baby_father]->   meiosis(hole_class[e_player_mother],hole_class[e_player_father]);
  sense_class[e_baby_father]->  meiosis(sense_class[e_player_mother],sense_class[e_player_father]);
  nature_class[e_baby_father]-> meiosis(nature_class[e_player_mother], nature_class[e_player_father]);
  eros_class[e_baby_father]->   meiosis(eros_class[e_player_mother],eros_class[e_player_father]);
}
void pregnant(String family_name){
  info_class[e_baby_father]->generate(random(2), true);
  info_class[e_baby_father]->update_family(family_name);
  view_status("new face",info_class[e_baby_father],head_class[e_baby_father],body_class[e_baby_father],parts_class[e_baby_father],stat_class[e_baby_father],hole_class[e_baby_father],sense_class[e_baby_father],nature_class[e_baby_father],eros_class[e_baby_father]);
}

void random_incounter(){
  bool gender = false;
  #ifdef WOMAN_ONLY
    gender = false;
  #else
    gender = random(2);
  #endif
  new_model(gender,info_class[e_baby_father],head_class[e_baby_father],body_class[e_baby_father],parts_class[e_baby_father],stat_class[e_baby_father],hole_class[e_baby_father],sense_class[e_baby_father],nature_class[e_baby_father],eros_class[e_baby_father]);
  view_status("new face",info_class[e_baby_father],head_class[e_baby_father],body_class[e_baby_father],parts_class[e_baby_father],stat_class[e_baby_father],hole_class[e_baby_father],sense_class[e_baby_father],nature_class[e_baby_father],eros_class[e_baby_father]);
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
  }
  /***** COMMON *****/
  new_model(false,info_class[e_player_mother],head_class[e_player_mother],body_class[e_player_mother],parts_class[e_player_mother],stat_class[e_player_mother],hole_class[e_player_mother],sense_class[e_player_mother],nature_class[e_player_mother],eros_class[e_player_mother]);
  new_model(true,info_class[e_player_father],head_class[e_player_father],body_class[e_player_father],parts_class[e_player_father],stat_class[e_player_father],hole_class[e_player_father],sense_class[e_player_father],nature_class[e_player_father],eros_class[e_player_father]);

  info_class[e_player]-> generate(random(2), true);
  info_class[e_player]-> update_family(info_class[e_player_mother]->get_family());
  head_class[e_player]-> blend(head_class[e_player_mother], head_class[e_player_father]);
  body_class[e_player]-> blend(body_class[e_player_mother],body_class[e_player_father]);
  parts_class[e_player]->blend(parts_class[e_player_mother],parts_class[e_player_father]);
  stat_class[e_player]-> blend(stat_class[e_player_mother], stat_class[e_player_father]);
  hole_class[e_player]-> blend(hole_class[e_player_mother],hole_class[e_player_father]);
  sense_class[e_player]->blend(sense_class[e_player_mother],sense_class[e_player_father]);
  nature_class[e_player]-> blend(nature_class[e_player_mother], nature_class[e_player_father]);
  eros_class[e_player]-> blend(eros_class[e_player_mother],eros_class[e_player_father]);
  /***** COMMON *****/  
  
  view_status("player_mother",info_class[e_player_mother],head_class[e_player_mother],body_class[e_player_mother],parts_class[e_player_mother],stat_class[e_player_mother],hole_class[e_player_mother],sense_class[e_player_mother],nature_class[e_player_mother],eros_class[e_player_mother]);
  view_status("player_father",info_class[e_player_father],head_class[e_player_father],body_class[e_player_father],parts_class[e_player_father],stat_class[e_player_father],hole_class[e_player_father],sense_class[e_player_father],nature_class[e_player_father],eros_class[e_player_father]);
  view_status("Player",info_class[e_player],head_class[e_player],body_class[e_player],parts_class[e_player],stat_class[e_player],hole_class[e_player],sense_class[e_player],nature_class[e_player],eros_class[e_player]);
  
  new_model(false,info_class[e_npc],head_class[e_npc],body_class[e_npc],parts_class[e_npc],stat_class[e_npc],hole_class[e_npc],sense_class[e_npc],nature_class[e_npc],eros_class[e_npc]);
  view_status("NPC",info_class[e_npc],head_class[e_npc],body_class[e_npc],parts_class[e_npc],stat_class[e_npc],hole_class[e_npc],sense_class[e_npc],nature_class[e_npc],eros_class[e_npc]);
  random_incounter();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  // newface();
}
#include "model_hard.h"
#include "model_soft.h"
#include "utility.h"
#include "database.h"
#include "funtions.h"

#define WOMAN_ONLY
//#define ESP32_CORE
#define CLASS_ARRAY 12
enum models{
  player, player_mother, player_father, pet, pet_mother, pet_father, baby, baby_mother, baby_father, npc, npc_mother, npc_father
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
  head_class[baby_father]->   meiosis(head_class[player_mother], head_class[player_father]);
  body_class[baby_father]->   meiosis(body_class[player_mother],body_class[player_father]);
  parts_class[baby_father]->  meiosis(parts_class[player_mother],parts_class[player_father]);
  stat_class[baby_father]->   meiosis(stat_class[player_mother], stat_class[player_father]);
  hole_class[baby_father]->   meiosis(hole_class[player_mother],hole_class[player_father]);
  sense_class[baby_father]->  meiosis(sense_class[player_mother],sense_class[player_father]);
  nature_class[baby_father]-> meiosis(nature_class[player_mother], nature_class[player_father]);
  eros_class[baby_father]->   meiosis(eros_class[player_mother],eros_class[player_father]);
}
void pregnant(String family_name){
  info_class[baby_father]->generate(random(2), true);
  info_class[baby_father]->update_family(family_name);
  view_status("new face",info_class[baby_father],head_class[baby_father],body_class[baby_father],parts_class[baby_father],stat_class[baby_father],hole_class[baby_father],sense_class[baby_father],nature_class[baby_father],eros_class[baby_father]);
}

void random_incounter(){
  bool gender = false;
  #ifdef WOMAN_ONLY
    gender = false;
  #else
    gender = random(2);
  #endif
  new_model(gender,info_class[baby_father],head_class[baby_father],body_class[baby_father],parts_class[baby_father],stat_class[baby_father],hole_class[baby_father],sense_class[baby_father],nature_class[baby_father],eros_class[baby_father]);
  view_status("new face",info_class[baby_father],head_class[baby_father],body_class[baby_father],parts_class[baby_father],stat_class[baby_father],hole_class[baby_father],sense_class[baby_father],nature_class[baby_father],eros_class[baby_father]);
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
  new_model(false,info_class[player_mother],head_class[player_mother],body_class[player_mother],parts_class[player_mother],stat_class[player_mother],hole_class[player_mother],sense_class[player_mother],nature_class[player_mother],eros_class[player_mother]);
  new_model(true,info_class[player_father],head_class[player_father],body_class[player_father],parts_class[player_father],stat_class[player_father],hole_class[player_father],sense_class[player_father],nature_class[player_father],eros_class[player_father]);
  new_model(false,info_class[npc],head_class[npc],body_class[npc],parts_class[npc],stat_class[npc],hole_class[npc],sense_class[npc],nature_class[npc],eros_class[npc]);

  info_class[player]-> generate(random(2), true);
  info_class[player]-> update_family(info_class[player_mother]->get_family());
  head_class[player]-> blend(head_class[player_mother], head_class[player_father]);
  body_class[player]-> blend(body_class[player_mother],body_class[player_father]);
  parts_class[player]->blend(parts_class[player_mother],parts_class[player_father]);
  stat_class[player]-> blend(stat_class[player_mother], stat_class[player_father]);
  hole_class[player]-> blend(hole_class[player_mother],hole_class[player_father]);
  sense_class[player]->blend(sense_class[player_mother],sense_class[player_father]);
  nature_class[player]-> blend(nature_class[player_mother], nature_class[player_father]);
  eros_class[player]-> blend(eros_class[player_mother],eros_class[player_father]);
  /***** COMMON *****/  
  
  view_status("player_mother",info_class[player_mother],head_class[player_mother],body_class[player_mother],parts_class[player_mother],stat_class[player_mother],hole_class[player_mother],sense_class[player_mother],nature_class[player_mother],eros_class[player_mother]);
  view_status("player_father",info_class[player_father],head_class[player_father],body_class[player_father],parts_class[player_father],stat_class[player_father],hole_class[player_father],sense_class[player_father],nature_class[player_father],eros_class[player_father]);
  view_status("NPC",info_class[npc],head_class[npc],body_class[npc],parts_class[npc],stat_class[npc],hole_class[npc],sense_class[npc],nature_class[npc],eros_class[npc]);
  random_incounter();
  view_status("Player",info_class[player],head_class[player],body_class[player],parts_class[player],stat_class[player],hole_class[player],sense_class[player],nature_class[player],eros_class[player]);
}

void loop() {
  // put your main code here, to run repeatedly:
  // newface();
}
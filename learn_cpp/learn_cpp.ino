#include "model_hard.h"
#include "model_soft.h"
#include "utility.h"
#include "database.h"
#include "funtions.h"

#define WOMAN_ONLY
//#define ESP32_CORE
#define CLASS_ARRAY 12
#define MODELS_MOTER 4
#define MODELS_FATER 5
enum models{
  e_player, e_pet, e_baby, e_npc, e_player_mother, e_player_father, e_pet_mother, e_pet_father, e_baby_mother, e_baby_father, e_npc_mother, e_npc_father
};
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
/***** HARDWARE *****/
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
void gene_meiosis(uint8_t child, uint8_t target){
  head_class[target]->   meiosis(head_class[child+MODELS_MOTER],  head_class[child+MODELS_FATER]);
  body_class[target]->   meiosis(body_class[child+MODELS_MOTER],  body_class[child+MODELS_FATER]);
  parts_class[target]->  meiosis(parts_class[child+MODELS_MOTER], parts_class[child+MODELS_FATER]);
  stat_class[target]->   meiosis(stat_class[child+MODELS_MOTER],  stat_class[child+MODELS_FATER]);
  hole_class[target]->   meiosis(hole_class[child+MODELS_MOTER],  hole_class[child+MODELS_FATER]);
  sense_class[target]->  meiosis(sense_class[child+MODELS_MOTER], sense_class[child+MODELS_FATER]);
  nature_class[target]-> meiosis(nature_class[child+MODELS_MOTER],nature_class[child+MODELS_FATER]);
  eros_class[target]->   meiosis(eros_class[child+MODELS_MOTER],  eros_class[child+MODELS_FATER]);
}
void gene_blended(uint8_t child){
  info_class[child]->   generate(random(2), true);
  info_class[child]->   update_family(info_class[e_player_mother]->get_family());
  head_class[child]->   blend(head_class[child+MODELS_MOTER],   head_class[child+MODELS_FATER]);
  body_class[child]->   blend(body_class[child+MODELS_MOTER],   body_class[child+MODELS_FATER]);
  parts_class[child]->  blend(parts_class[child+MODELS_MOTER],  parts_class[child+MODELS_FATER]);
  stat_class[child]->   blend(stat_class[child+MODELS_MOTER],   stat_class[child+MODELS_FATER]);
  hole_class[child]->   blend(hole_class[child+MODELS_MOTER],   hole_class[child+MODELS_FATER]);
  sense_class[child]->  blend(sense_class[child+MODELS_MOTER],  sense_class[child+MODELS_FATER]);
  nature_class[child]-> blend(nature_class[child+MODELS_MOTER], nature_class[child+MODELS_FATER]);
  eros_class[child]->   blend(eros_class[child+MODELS_MOTER],   eros_class[child+MODELS_FATER]);
}
void pregnant(String family_name){
  info_class[e_baby]->generate(random(2), true);
  info_class[e_baby]->update_family(family_name);
  view_status("\nnew baby",info_class[e_baby],head_class[e_baby],body_class[e_baby],parts_class[e_baby],stat_class[e_baby],hole_class[e_baby],sense_class[e_baby],nature_class[e_baby],eros_class[e_baby]);
}

void random_incounter(){
  bool gender = false;
  #ifdef WOMAN_ONLY
    gender = false;
  #else
    gender = random(2);
  #endif
  new_model(false,info_class[e_npc_mother],head_class[e_npc_mother],body_class[e_npc_mother],parts_class[e_npc_mother],stat_class[e_npc_mother],hole_class[e_npc_mother],sense_class[e_npc_mother],nature_class[e_npc_mother],eros_class[e_npc_mother]);
  new_model(false,info_class[e_npc_father],head_class[e_npc_father],body_class[e_npc_father],parts_class[e_npc_father],stat_class[e_npc_father],hole_class[e_npc_father],sense_class[e_npc_father],nature_class[e_npc_father],eros_class[e_npc_father]);
  gene_blended(e_npc);
  
  view_status("\nnew NPC mother",info_class[e_npc_mother],head_class[e_npc_mother],body_class[e_npc_mother],parts_class[e_npc_mother],stat_class[e_npc_mother],hole_class[e_npc_mother],sense_class[e_npc_mother],nature_class[e_npc_mother],eros_class[e_npc_mother]);
  view_status("\nnew NPC father",info_class[e_npc_father],head_class[e_npc_father],body_class[e_npc_father],parts_class[e_npc_father],stat_class[e_npc_father],hole_class[e_npc_father],sense_class[e_npc_father],nature_class[e_npc_father],eros_class[e_npc_father]);
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
  }
  /***** HARDWARE *****/
  new_model(false,info_class[e_player_mother],head_class[e_player_mother],body_class[e_player_mother],parts_class[e_player_mother],stat_class[e_player_mother],hole_class[e_player_mother],sense_class[e_player_mother],nature_class[e_player_mother],eros_class[e_player_mother]);
  new_model(true,info_class[e_player_father],head_class[e_player_father],body_class[e_player_father],parts_class[e_player_father],stat_class[e_player_father],hole_class[e_player_father],sense_class[e_player_father],nature_class[e_player_father],eros_class[e_player_father]);
  gene_blended(e_player);
  /***** HARDWARE *****/
  
  view_status("\nplayer mother",info_class[e_player_mother],head_class[e_player_mother],body_class[e_player_mother],parts_class[e_player_mother],stat_class[e_player_mother],hole_class[e_player_mother],sense_class[e_player_mother],nature_class[e_player_mother],eros_class[e_player_mother]);
  view_status("\nplayer father",info_class[e_player_father],head_class[e_player_father],body_class[e_player_father],parts_class[e_player_father],stat_class[e_player_father],hole_class[e_player_father],sense_class[e_player_father],nature_class[e_player_father],eros_class[e_player_father]);
  view_status("\nPlayer",info_class[e_player],head_class[e_player],body_class[e_player],parts_class[e_player],stat_class[e_player],hole_class[e_player],sense_class[e_player],nature_class[e_player],eros_class[e_player]);
  
  random_incounter();
}

void loop() {
  // put your main code here, to run repeatedly:
  // newface();
}
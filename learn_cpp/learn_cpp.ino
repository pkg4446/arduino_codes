#include "model_common.h"
#include "model_status.h"
#include "utility.h"
#include "database.h"

#define WOMAN_ONLY
#define DEBUG
//#define ESP32_CORE
/***** GENE *********/
INFO      *info_mother  = new INFO();
HEAD      *head_mother  = new HEAD();
BODY      *body_mother  = new BODY();
EROGENOUS *parts_mother = new EROGENOUS();
STAT      *stat_mother  = new STAT();
HOLE      *hole_mother  = new HOLE();
SENSE     *sense_mother = new SENSE();
NATURE    *nature_mother = new NATURE();
EROS      *eros_mother  = new EROS();
/***** GENE *********/
INFO      *info_father  = new INFO();
HEAD      *head_father  = new HEAD();
BODY      *body_father  = new BODY();
EROGENOUS *parts_father = new EROGENOUS();
STAT      *stat_father  = new STAT();
HOLE      *hole_father  = new HOLE();
SENSE     *sense_father = new SENSE();
NATURE    *nature_father = new NATURE();
EROS      *eros_father  = new EROS();
/***** GENE *********/
/***** PLAYER *******/
INFO      *info_player  = new INFO();
HEAD      *head_player  = new HEAD();
BODY      *body_player  = new BODY();
EROGENOUS *parts_player = new EROGENOUS();
STAT      *stat_player  = new STAT();
HOLE      *hole_player  = new HOLE();
SENSE     *sense_player = new SENSE();
NATURE    *nature_player = new NATURE();
EROS      *eros_player  = new EROS();
/***** PLAYER *******/
/***** NPC **********/
INFO      *info_npc  = new INFO();
HEAD      *head_npc  = new HEAD();
BODY      *body_npc  = new BODY();
EROGENOUS *parts_npc = new EROGENOUS();
STAT      *stat_npc  = new STAT();
HOLE      *hole_npc  = new HOLE();
SENSE     *sense_npc = new SENSE();
NATURE    *nature_npc = new NATURE();
EROS      *eros_npc  = new EROS();
/***** NPC **********/
/***** TEMP *********/
INFO      *info_temp  = new INFO();
HEAD      *head_temp  = new HEAD();
BODY      *body_temp  = new BODY();
EROGENOUS *parts_temp = new EROGENOUS();
STAT      *stat_temp  = new STAT();
HOLE      *hole_temp  = new HOLE();
SENSE     *sense_temp = new SENSE();
NATURE    *nature_temp = new NATURE();
EROS      *eros_temp  = new EROS();
/***** TEMP *********/
/***** funtions *****/
void view_status(String title,INFO *class_info,HEAD *class_head,BODY *class_body,EROGENOUS *class_parts,STAT *class_stat,HOLE *class_hole,SENSE *class_sense,NATURE *class_nature,EROS *class_eros){
    #ifdef DEBUG
    perforation(title);
    class_info-> status();
    class_head-> status();
    class_body-> status();
    class_body-> weight();
    class_parts->status();
    class_stat-> status();
    class_hole-> status(class_info->get_gender());
    class_sense->status(class_info->get_gender());
    class_nature->status();
    class_eros-> status();
    #endif
}
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
  head_gene-> meiosis(head_mother, head_father);
  body_gene-> meiosis(body_mother,body_father);
  parts_gene->meiosis(parts_mother,parts_father);
  stat_gene-> meiosis(stat_mother, stat_father);
  hole_gene-> meiosis(hole_mother,hole_father);
  sense_gene->meiosis(sense_mother,sense_father);
  nature_gene->meiosis(nature_mother, nature_father);
  eros_gene-> meiosis(eros_mother,eros_father);
  delete head_gene;
  delete body_gene;
  delete parts_gene;
  delete stat_gene;
  delete hole_gene;
  delete sense_gene;
  delete nature_gene;
  delete eros_gene;
}
void newface(bool baby, String family_name){
  bool gender = false;
  #ifdef WOMAN_ONLY
    gender = false;
  #else
    gender = random(2);
  #endif
  info_temp->generate(gender, baby);
  if(baby) info_temp->update_family(family_name);
  head_temp->generate(info_temp->get_gender());
  body_temp->generate(info_temp->get_gender());
  parts_temp->generate(info_temp->get_gender());
  stat_temp->generate();
  hole_temp->generate();
  sense_temp->generate();
  nature_temp->generate();
  eros_temp->generate();
  view_status("new face",info_temp,head_temp,body_temp,parts_temp,stat_temp,hole_temp,sense_temp,nature_temp,eros_temp);
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
  /***** COMMON *****/
  info_mother->generate(false, false);
  head_mother->generate(info_mother->get_gender());
  body_mother->generate(info_mother->get_gender());
  parts_mother->generate(info_mother->get_gender());
  stat_mother->generate();
  hole_mother->generate();
  sense_mother->generate();
  nature_mother->generate();
  eros_mother->generate();

  info_father->generate(true, false);
  head_father->generate(info_father->get_gender());
  body_father->generate(info_father->get_gender());
  parts_father->generate(info_father->get_gender());
  stat_father->generate();
  hole_father->generate();
  sense_father->generate();
  nature_father->generate();
  eros_father->generate();

  info_npc  ->generate(false, false);
  head_npc  ->generate(info_npc->get_gender());
  body_npc  ->generate(info_npc->get_gender());
  parts_npc ->generate(info_npc->get_gender());
  stat_npc  ->generate();
  hole_npc  ->generate();
  sense_npc ->generate();
  nature_npc->generate();
  eros_npc  ->generate();

  info_player-> generate(random(2), true);
  head_player-> blend(head_mother, head_father);
  body_player-> blend(body_mother,body_father);
  parts_player->blend(parts_mother,parts_father);
  stat_player-> blend(stat_mother, stat_father);
  hole_player-> blend(hole_mother,hole_father);
  sense_player->blend(sense_mother,sense_father);
  nature_player-> blend(nature_mother, nature_father);
  eros_player-> blend(eros_mother,eros_father);
  /***** COMMON *****/  
  
  view_status("mother",info_mother,head_mother,body_mother,parts_mother,stat_mother,hole_mother,sense_mother,nature_mother,eros_mother);
  view_status("father",info_father,head_father,body_father,parts_father,stat_father,hole_father,sense_father,nature_father,eros_father);
  view_status("NPC",info_npc,head_npc,body_npc,parts_npc,stat_npc,hole_npc,sense_npc,nature_npc,eros_npc);
  newface(false,"null");
  view_status("Player",info_player,head_player,body_player,parts_player,stat_player,hole_player,sense_player,nature_player,eros_player);
}

void loop() {
  // put your main code here, to run repeatedly:
  // newface();
}
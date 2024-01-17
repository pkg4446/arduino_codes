#include "model_common.h"
#include "model_status.h"
#include "utility.h"
#include "database.h"
bool gender;
#define WOMAN_ONLY
#define DEBUG
//#define ESP32_CORE
/***** PLAYER *****/
INFO      *info_player;
HEAD      *head_player;
BODY      *body_player;
EROGENOUS *parts_player;
STAT      *stat_player;
HOLE      *hole_player;
SENSE     *sense_player;
NATURE    *nature_player;
EROS      *eros_player;
/***** PLAYER *****/
/***** GENE *****/
HEAD      *head_mother;
BODY      *body_mother;
EROGENOUS *parts_mother;
STAT      *stat_mother;
HOLE      *hole_mother;
SENSE     *sense_mother;
NATURE    *nature_mother;
EROS      *eros_mother;

HEAD      *head_father;
BODY      *body_father;
EROGENOUS *parts_father;
STAT      *stat_father;
HOLE      *hole_father;
SENSE     *sense_father;
NATURE    *nature_father;
EROS      *eros_father;
/***** GENE *****/
/***** NPC *****/
bool      inner_shot = false;
HEAD      *gene_head;
BODY      *gene_body;
EROGENOUS *gene_parts;
STAT      *gene_stat;
HOLE      *gene_hole;
SENSE     *gene_sense;
NATURE    *gene_nature;
EROS      *gene_eros;
/***** NPC *****/
/***** funtions *****/
void meiosis(){
  if(inner_shot){
    bool      new_gender  = random(2);
    HEAD      *new_head   = new HEAD(new_gender);
    BODY      *new_body   = new BODY(new_gender);
    EROGENOUS *new_parts  = new EROGENOUS(new_gender);
    STAT      *new_stat   = new STAT();
    HOLE      *new_hole   = new HOLE();
    SENSE     *new_sense  = new SENSE();
    NATURE    *new_nature = new NATURE();
    EROS      *new_eros   = new EROS();
    new_head-> meiosis(head_mother, head_father);
    new_body-> meiosis(body_mother,body_father);
    new_parts->meiosis(parts_mother,parts_father);
    new_stat-> meiosis(stat_mother, stat_father);
    new_hole-> meiosis(hole_mother,hole_father);
    new_sense->meiosis(sense_mother,sense_father);
    new_nature->meiosis(nature_mother, nature_father);
    new_eros-> meiosis(eros_mother,eros_father);
  }
}
void newface(){
  bool      new_gender  = random(2);
  HEAD      *new_head   = new HEAD(new_gender);
  BODY      *new_body   = new BODY(new_gender);
  EROGENOUS *new_parts  = new EROGENOUS(new_gender);
  STAT      *new_stat   = new STAT();
  HOLE      *new_hole   = new HOLE();
  SENSE     *new_sense  = new SENSE();
  NATURE    *new_nature = new NATURE();
  EROS      *new_eros   = new EROS();
  #ifdef DEBUG
    perforation("new face");
    new_head->  status();
    new_body->  status();
    new_body->  weight();
    new_parts-> status();
    new_stat->  status();
    new_hole->  status(gender);
    new_sense-> status(gender);
    new_nature->status();
    new_eros->  status();
  #endif
  
  gene_head   = new HEAD(!gender);
  gene_body   = new BODY(!gender);
  gene_parts  = new EROGENOUS(!gender);
  gene_stat   = new STAT();
  gene_hole   = new HOLE();
  gene_sense  = new SENSE();
  gene_nature = new NATURE();
  gene_eros   = new EROS();

  delete new_head;
  delete new_body;
  delete new_parts;
  delete new_stat;
  delete new_hole;
  delete new_sense;
  delete new_nature;
  delete new_eros;
}
/***** funtions *****/
void setup() {
  Serial.begin(115200);
  #ifdef WOMAN_ONLY
    gender = false;
  #else
    gender = random(2);
  #endif
  #ifdef ESP32_CORE
    randomSeed(analogRead(39));
    Serial.println("ESP32");
  #else
    randomSeed(analogRead(A0));
    Serial.println("ARDUINO");
  #endif
  /***** COMMON *****/
  head_mother   = new HEAD(false);
  body_mother   = new BODY(false);
  parts_mother  = new EROGENOUS(false);
  stat_mother   = new STAT();
  hole_mother   = new HOLE();
  sense_mother  = new SENSE();
  nature_mother = new NATURE();
  eros_mother   = new EROS();

  head_father   = new HEAD(true);
  body_father   = new BODY(true);
  parts_father  = new EROGENOUS(true);
  stat_father   = new STAT();
  hole_father   = new HOLE();
  sense_father  = new SENSE();
  nature_father = new NATURE();
  eros_father   = new EROS();

  head_player   = new HEAD(gender);
  body_player   = new BODY(gender);
  parts_player  = new EROGENOUS(gender);
  stat_player   = new STAT();
  hole_player   = new HOLE();
  sense_player  = new SENSE();
  nature_player = new NATURE();
  eros_player   = new EROS();

  gene_head     = new HEAD(!gender);
  gene_body     = new BODY(!gender);
  gene_parts    = new EROGENOUS(!gender);
  gene_stat     = new STAT();
  gene_hole     = new HOLE();
  gene_sense    = new SENSE();
  gene_nature   = new NATURE();
  gene_eros     = new EROS();
  /***** COMMON *****/
  
  #ifdef DEBUG
    perforation("mother");
    head_mother-> status();
    body_mother-> status();
    body_mother-> weight();
    parts_mother->status();
    stat_mother-> status();
    hole_mother-> status(false);
    sense_mother->status(false);
    nature_mother->status();
    eros_mother-> status();
    perforation("father");
    head_father-> status();
    body_father-> status();
    body_father-> weight();
    parts_father->status();
    stat_father-> status();
    hole_father-> status(true);
    sense_father->status(true);
    nature_father->status();
    eros_father-> status();
  #endif

  head_player->   blend(head_mother, head_father);
  body_player->   blend(body_mother,body_father);
  parts_player->  blend(parts_mother,parts_father);
  stat_player->   blend(stat_mother, stat_father);
  hole_player->   blend(hole_mother,hole_father);
  sense_player->  blend(sense_mother,sense_father);
  nature_player-> blend(nature_mother, nature_father);
  eros_player->   blend(eros_mother,eros_father);
  
  Serial.print("Gender:");
  if(gender)  Serial.println("male");
  else        Serial.println("female");

  #ifdef DEBUG
    perforation("mine");
    head_player-> status();
    body_player-> status();
    body_player-> weight();
    parts_player->status();

    stat_player-> status();
    hole_player-> status(gender);
    sense_player->status(gender);
    nature_player->status();
    eros_player-> status();
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:
  // newface();
}
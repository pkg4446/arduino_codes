#include "model_common.h"
#include "model_status.h"
#include "utility.h"
#include "database.h"
bool gender;
#define WOMAN_ONLY
#define DEBUG
//#define ESP32_CORE
/***** PLAYER *****/
HEAD      *play_head;
BODY      *play_body;
EROGENOUS *play_parts;
STAT      *play_stat;
HOLE      *play_hole;
SENSE     *play_sense;
NATURE    *play_nature;
EROS      *play_eros;
/***** PLAYER *****/
/***** GENE *****/
HEAD      *mother_head;
BODY      *mother_body;
EROGENOUS *mother_parts;
STAT      *mother_stat;
HOLE      *mother_hole;
SENSE     *mother_sense;
NATURE    *mother_nature;
EROS      *mother_eros;

HEAD      *father_head;
BODY      *father_body;
EROGENOUS *father_parts;
STAT      *father_stat;
HOLE      *father_hole;
SENSE     *father_sense;
NATURE    *father_nature;
EROS      *father_eros;
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
    new_head-> meiosis(mother_head, father_head);
    new_body-> meiosis(mother_body,father_body);
    new_parts->meiosis(mother_parts,father_parts);
    new_stat-> meiosis(mother_stat, father_stat);
    new_hole-> meiosis(mother_hole,father_hole);
    new_sense->meiosis(mother_sense,father_sense);
    new_nature->meiosis(mother_nature, father_nature);
    new_eros-> meiosis(mother_eros,father_eros);
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
    new_head->status();
    new_body->status();
    new_body->weight();
    new_parts->status();
    new_stat->status();
    new_hole->status(gender);
    new_sense->status(gender);
    new_nature->status();
    new_eros->status();
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
  /***** BODY *****/
  mother_head   = new HEAD(false);
  mother_body   = new BODY(false);
  mother_parts  = new EROGENOUS(false);
  father_head   = new HEAD(true);
  father_body   = new BODY(true);
  father_parts  = new EROGENOUS(true);
  play_head     = new HEAD(gender);
  play_body     = new BODY(gender);
  play_parts    = new EROGENOUS(gender);
  gene_head     = new HEAD(!gender);
  gene_body     = new BODY(!gender);
  gene_parts    = new EROGENOUS(!gender);
  /***** BODY *****/
  /***** STATUS ***/
  mother_stat   = new STAT();
  mother_hole   = new HOLE();
  mother_sense  = new SENSE();
  mother_nature = new NATURE();
  mother_eros   = new EROS();
  father_stat   = new STAT();
  father_hole   = new HOLE();
  father_sense  = new SENSE();
  father_nature = new NATURE();
  father_eros   = new EROS();
  play_stat     = new STAT();
  play_hole     = new HOLE();
  play_sense    = new SENSE();
  play_nature   = new NATURE();
  play_eros     = new EROS();
  gene_stat     = new STAT();
  gene_hole     = new HOLE();
  gene_sense    = new SENSE();
  gene_nature   = new NATURE();
  gene_eros     = new EROS();
  /***** STATUS ***/
  
  #ifdef DEBUG
    perforation("mother");
    mother_head-> status();
    mother_body-> status();
    mother_body-> weight();
    mother_parts->status();
    mother_stat-> status();
    mother_hole-> status(false);
    mother_sense->status(false);
    mother_nature->status();
    mother_eros-> status();
    perforation("father");
    father_head-> status();
    father_body-> status();
    father_body-> weight();
    father_parts->status();
    father_stat-> status();
    father_hole-> status(true);
    father_sense->status(true);
    father_nature->status();
    father_eros-> status();
  #endif

  play_head->blend(mother_head, father_head);
  play_body->blend(mother_body,father_body);
  play_parts->blend(mother_parts,father_parts);
  play_stat->blend(mother_stat, father_stat);
  play_hole->blend(mother_hole,father_hole);
  play_sense->blend(mother_sense,father_sense);
  play_nature->blend(mother_nature, father_nature);
  play_eros->blend(mother_eros,father_eros);
  
  Serial.print("Gender:");
  if(gender)  Serial.println("male");
  else        Serial.println("female");

  #ifdef DEBUG
    perforation("mine");
    play_head-> status();
    play_body-> status();
    play_body-> weight();
    play_parts->status();

    play_stat-> status();
    play_hole-> status(gender);
    play_sense->status(gender);
    play_nature->status();
    play_eros-> status();
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:
  // newface();
}
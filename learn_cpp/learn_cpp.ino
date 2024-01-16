#include "Body.h"
#include "Status.h"
bool gender;
//#define DEBUG

HEAD *play_head;
BODY *play_body;
EROGENOUS *play_parts;

STAT   *play_stat;
HOLE   *play_hole;
SENSE  *play_sense;
NATURE *play_nature;
EROS   *play_eros;

void setup() {
  randomSeed(analogRead(0));
  gender = random(2);
  Serial.begin(115200);
  /***** CHARACTER *****/
  play_head   = new HEAD();
  play_body   = new BODY(gender);
  play_parts  = new EROGENOUS(gender);
  play_stat   = new STAT();
  play_hole   = new HOLE();
  play_sense  = new SENSE();
  play_nature = new NATURE();
  play_eros   = new EROS();
  /***** CHARACTER *****/
  /***** BODY *****/
  HEAD *mother_head       = new HEAD();
  BODY *mother_body       = new BODY(false);
  EROGENOUS *mother_parts = new EROGENOUS(false);
  HEAD *father_head       = new HEAD();
  BODY *father_body       = new BODY(true);
  EROGENOUS *father_parts = new EROGENOUS(true);
  /***** BODY *****/
  /***** STATUS ***/
  STAT   *mother_stat     = new STAT();
  HOLE   *mother_hole     = new HOLE();
  SENSE  *mother_sense    = new SENSE();
  NATURE *mother_nature   = new NATURE();
  EROS   *mother_eros     = new EROS();
  STAT   *father_stat     = new STAT();
  HOLE   *father_hole     = new HOLE();
  SENSE  *father_sense    = new SENSE();
  NATURE *father_nature   = new NATURE();
  EROS   *father_eros     = new EROS();
  /***** STATUS ***/

  #ifdef DEBUG
    Serial.println("************** mother **************");
    mother_head->status();
    mother_body->status();
    mother_body->weight();
    mother_parts->status();
    mother_stat->status();
    mother_hole->status(false);
    mother_sense->status(false);
    mother_nature->status();
    mother_eros->status();
    Serial.println("************** father **************");
    father_head->status();
    father_body->status();
    father_body->weight();
    father_parts->status();
    father_stat->status();
    father_hole->status(true);
    father_sense->status(true);
    father_nature->status();
    father_eros->status();
  #endif

  
  play_head->blend(mother_head, father_head, gender);
  play_body->blend(mother_body,father_body);
  play_parts->blend(mother_parts,father_parts);
  delete mother_head;
  delete mother_body;
  delete mother_parts;
  delete father_head;
  delete father_body;
  delete father_parts;
  
  play_stat->blend(mother_stat, father_stat);
  play_hole->blend(mother_hole,father_hole);
  play_sense->blend(mother_sense,father_sense);
  play_nature->blend(mother_nature, father_nature);
  play_eros->blend(mother_eros,father_eros);
  delete mother_stat;
  delete mother_hole;
  delete mother_sense;
  delete mother_nature;
  delete mother_eros;
  delete father_stat;
  delete father_hole;
  delete father_sense;
  delete father_nature;
  delete father_eros;

  Serial.print("Gender:");
  if(gender)  Serial.println("male");
  else        Serial.println("female");

  #ifdef DEBUG
    Serial.println("************** mine ****************");
    play_head->status();
    play_body->status();
    play_body->weight();
    play_parts->status();

    play_stat->status();
    play_hole->status(gender);
    play_sense->status(gender);
    play_nature->status();
    play_eros->status();
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:

}
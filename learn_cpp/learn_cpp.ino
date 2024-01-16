#include "Body.h"
#include "Status.h"
bool gender;
#define DEBUG

void setup() {
  randomSeed(analogRead(0));
  gender = random(2);
  Serial.begin(115200);
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
    mother_hole->status();
    mother_sense->status();
    mother_nature->status();
    mother_eros->status();
    Serial.println("************** father **************");
    father_head->status();
    father_body->status();
    father_body->weight();
    father_parts->status();
    father_stat->status();
    father_hole->status();
    father_sense->status();
    father_nature->status();
    father_eros->status();
  #endif

  HEAD *my_head       = new HEAD();
  BODY *my_body       = new BODY(gender);
  EROGENOUS *my_parts = new EROGENOUS(gender);
  my_head->blend(mother_head, father_head, gender);
  my_body->blend(mother_body,father_body);
  my_parts->blend(mother_parts,father_parts);
  delete mother_head;
  delete mother_body;
  delete mother_parts;
  delete father_head;
  delete father_body;
  delete father_parts;
  STAT   *my_stat     = new STAT();
  HOLE   *my_hole     = new HOLE();
  SENSE  *my_sense    = new SENSE();
  NATURE *my_nature   = new NATURE();
  EROS   *my_eros     = new EROS();
  my_stat->blend(mother_stat, father_stat);
  my_hole->blend(mother_hole,father_hole);
  my_sense->blend(mother_sense,father_sense);
  my_nature->blend(mother_nature, father_nature);
  my_eros->blend(mother_eros,father_eros);
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
    my_head->status();
    my_body->status();
    my_body->weight();
    my_parts->status();

    my_stat->status();
    my_hole->status();
    my_sense->status();
    my_nature->status();
    my_eros->status();
  #endif

  delete my_head;
  delete my_body;
  delete my_parts;
}

void loop() {
  // put your main code here, to run repeatedly:

}
#include "Body.h"
bool gender;
#define DEBUG

void setup() {
  randomSeed(analogRead(0));
  gender = random(2);
  Serial.begin(115200);
  
  HEAD *mother_head       = new HEAD();
  BODY *mother_body       = new BODY(false);
  EROGENOUS *mother_parts = new EROGENOUS(false);
  HEAD *father_head       = new HEAD();
  BODY *father_body       = new BODY(true);
  EROGENOUS *father_parts = new EROGENOUS(true);

  #ifdef DEBUG
    Serial.println("************** mother **************");
    mother_head->status();
    mother_body->status();
    mother_body->weight();
    mother_parts->status();
    Serial.println("************** father **************");
    father_head->status();
    father_body->status();
    father_body->weight();
    father_parts->status();
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

  my_head->status();
  my_body->status();
  my_body->weight();
  my_parts->status();

  Serial.print("Gender:");Serial.println(gender);

  delete my_head;
  delete my_body;
  delete my_parts;
}

void loop() {
  // put your main code here, to run repeatedly:

}
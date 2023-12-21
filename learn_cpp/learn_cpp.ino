#include "Body.h"
bool gender;

void setup() {
  randomSeed(analogRead(0));
  gender = random(2);
  Serial.begin(115200);
  
  HEAD *mother_head = new HEAD();
  BODY *mother_body = new BODY(false);
  mother_head->status();
  mother_body->status();

  HEAD *father_head = new HEAD();
  BODY *father_body = new BODY(true);
  father_head->status();
  father_body->status();

  HEAD *appearance_head = new HEAD();
  BODY *appearance_body = new BODY(true);
  appearance_head->blend(mother_head, father_head, gender);
  appearance_head->status();
  appearance_body->status();

  Serial.print("Gender:");Serial.println(gender);

  delete mother_head;
  delete father_head;
  delete appearance_head;
}

void loop() {
  // put your main code here, to run repeatedly:

}
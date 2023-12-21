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

  HEAD *my_head = new HEAD();
  BODY *my_body = new BODY(gender);
  my_head->blend(mother_head, father_head, gender);
  my_head->status();
  my_body->meiosis(mother_body,father_body);
  my_body->status();

  Serial.print("Gender:");Serial.println(gender);

  delete mother_head;
  delete mother_body;
  delete father_head;
  delete father_body;
  delete my_head;
  delete my_body;
}

void loop() {
  // put your main code here, to run repeatedly:

}
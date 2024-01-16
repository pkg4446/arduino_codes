#include "Body.h"
bool gender;
#define DEBUG

void setup() {
  randomSeed(analogRead(0));
  gender = random(2);
  Serial.begin(115200);
  
  HEAD *mother_head = new HEAD();
  BODY *mother_body = new BODY(false);
  HEAD *father_head = new HEAD();
  BODY *father_body = new BODY(true);

  #ifdef DEBUG
    Serail.println("************** mother **************");
    mother_head->status();
    mother_body->status();
    mother_body->weight();
    Serail.println("************** father **************");
    father_head->status();
    father_body->status();
    father_body->weight();
  #endif

  HEAD *my_head = new HEAD();
  BODY *my_body = new BODY(gender);
  my_head->blend(mother_head, father_head, gender);
  my_body->blend(mother_body,father_body);
  delete mother_head;
  delete mother_body;
  delete father_head;
  delete father_body;

  my_head->status();
  my_body->status();
  my_body->weight();

  Serial.print("Gender:");Serial.println(gender);

  delete my_head;
  delete my_body;
}

void loop() {
  // put your main code here, to run repeatedly:

}
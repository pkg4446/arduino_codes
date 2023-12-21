#include "Body.h"

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(115200);
  HEAD *h1 = new HEAD();
  h1->status();
  HEAD *h2 = new HEAD();
  h2->status();
  HEAD *h3 = new HEAD();
  h3->meiosis(h1,h2);
  h3->status();

  delete h1;
  delete h2;
  delete h3;
}

void loop() {
  // put your main code here, to run repeatedly:

}


/*
int main()
{
    
    
    delete h1;
    delete h2;
    delete h3;
    return 0;
}
*/
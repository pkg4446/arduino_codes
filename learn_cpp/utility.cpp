#include "utility.h"

void swap(uint16_t *num_a, uint16_t *num_b){
    uint16_t temporary = *num_a;
    *num_a = *num_b;
    *num_b = temporary;
}
void perforation(){
    for(uint8_t index=0; index<35; index++){
        Serial.print("*");
    }
    Serial.println("*");
}
void spacebar(String text){
    Serial.print(text);
    for(uint8_t index=text.length(); index<15; index++){
        Serial.print(" ");
    }
    Serial.print(": ");
}
void destruct(){
    Serial.println("Destructed");
}
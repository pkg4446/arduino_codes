#include "utility.h"

void swap(uint16_t *num_a, uint16_t *num_b){
    uint16_t temporary = *num_a;
    *num_a = *num_b;
    *num_b = temporary;
}
void perforation(String text){
    Serial.print(text);
    for(uint8_t index=text.length(); index<35; index++){
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
void unit_split(uint16_t values, uint16_t divides){
    Serial.print(values/divides);
    Serial.print(".");
    Serial.print(values%divides);
}
void unit_mm(){
    Serial.println(" mm");
}
void destruct(){
    Serial.println("Destructed");
}
#include "utility.h"
//distribution table
//0.13 ,0.26 ,0.39 ,0.52 ,0.67 ,0.84 ,1.04 ,1.28 ,1.65
uint16_t gaussian_range(uint16_t average, uint16_t standard_deviation){
    uint16_t response = average;
    uint8_t stage     = random(50);
    bool flage        = random(2);
    uint16_t range    = 0;

    if(stage<5){range = standard_deviation*13/100;}
    else if(stage<10){range = standard_deviation*26/100;}
    else if(stage<15){range = standard_deviation*39/100;}
    else if(stage<20){range = standard_deviation*52/100;}
    else if(stage<25){range = standard_deviation*67/100;}
    else if(stage<30){range = standard_deviation*84/100;}
    else if(stage<35){range = standard_deviation*104/100;}
    else if(stage<40){range = standard_deviation*128/100;}
    else if(stage<45){range = standard_deviation*126/100;}
    else{range = standard_deviation*4;}

    if(flage){response += range;}
    else if(range > response){response = range-response;}
    else{response -= range;}

    return response;
}

uint16_t get_gaussian(uint16_t average, uint16_t standard_deviation, uint16_t value){
    uint16_t gap = 0;
    if(average>value){
        gap = average - value;
    }else{
        gap = value - average;
    }
    uint32_t response = uint32_t(gap) * 100 / uint32_t(standard_deviation);
    return uint16_t(response);
}

void paging(){
    for(uint8_t index=0; index<LINE_NUM; index++){
        Serial.print("-");
    }
    Serial.println("-");
}

void swap(uint16_t *num_a, uint16_t *num_b){
    uint16_t temporary = *num_a;
    *num_a = *num_b;
    *num_b = temporary;
}
void perforation(String text){
    Serial.print(text);
    for(uint8_t index=text.length(); index<LINE_NUM; index++){
        Serial.print("*");
    }
    Serial.println("*");
}
void spacebar(bool korea, String text){
    uint8_t text_end = 15;
    if(korea) text_end += text.length()/3;
    Serial.print(text);
    for(uint8_t index=text.length(); index<text_end; index++){
        Serial.print(" ");
    }
    Serial.print(": ");
}
void spacebar_option(bool korea, uint8_t number, String text){
    Serial.print("[");
    if(number<100) Serial.print("0");
    if(number<10)  Serial.print("0");
    Serial.print(number);
    Serial.print("] ");
    uint8_t text_end = 18;
    if(korea) text_end += text.length()/3;
    Serial.print(text);
    for(uint8_t index=text.length(); index<text_end; index++){
        Serial.print(" ");
    }
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
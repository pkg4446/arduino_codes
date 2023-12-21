#include "heredity.h"
#include "Body.h"

#define RAND_NUM   100
#define RAND_FLAGE 2

HEAD::HEAD() { //생성자
    Serial.println("Constructing Head...");
    uint8_t rand_num;
    rand_num = random(RAND_NUM);
    //color_hair
    if(rand_num<22){color_hair = "Black";}
    else if(rand_num<41){color_hair = "Blue";}
    else if(rand_num<59){color_hair = "Orange";}
    else if(rand_num<74){color_hair = "Red";}
    else if(rand_num<88){color_hair = "Gold";}
    else{color_hair = "Silver";}
    //color_eye
    rand_num = random(RAND_NUM);;
    if(rand_num<8){color_eye = "Gold";}
    else if(rand_num<27){color_eye = "Obsidian";}
    else if(rand_num<47){color_eye = "Topaz";}
    else if(rand_num<59){color_eye = "Emerald";}
    else if(rand_num<91){color_eye = "Sapphire";}
    else{color_eye = "Ruby";}

    hair       = random(RAND_NUM);
    eyelid     = random(RAND_FLAGE);
    dimple     = random(RAND_FLAGE);
    bald       = random(RAND_FLAGE);
}

HEAD::~HEAD(){
    Serial.println("Destructing Head...");
}

void HEAD::meiosis(HEAD *mother, HEAD *father){
    
    color_hair = mutation_string(mother->color_hair,father->color_hair);
    color_eye  = mutation_string(mother->color_eye, father->color_eye);
    hair       = mutation_u8(mother->hair,          father->hair);
    eyelid     = mutation_bool(mother->eyelid,      father->eyelid);
    dimple     = mutation_bool(mother->dimple,      father->dimple);
    bald       = mutation_bool(mother->bald,        father->bald);
}

void HEAD::blend(HEAD *mother, HEAD *father, bool gender){
    color_hair = mutation_string(mother->color_hair,father->color_hair);
    color_eye  = mutation_string(mother->color_eye, father->color_eye);
    hair       = mutation_u8(mother->hair,          father->hair);
    eyelid     = mutation_bool(mother->eyelid,      father->eyelid);
    dimple     = mutation_bool(mother->dimple,      father->dimple);
    bald       = mutation_bool(mother->bald,        father->bald);
}

void HEAD::status(){
    Serial.print("color_hair: ");Serial.println(color_hair);
    Serial.print("color_eye: "); Serial.println(color_eye);
    Serial.print("hair: ");      Serial.println(hair);
    Serial.print("eyelid: ");    Serial.println(eyelid);
    Serial.print("dimple: ");    Serial.println(dimple);
    Serial.print("bald: ");      Serial.println(bald);
}


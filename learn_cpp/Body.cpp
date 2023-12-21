#include "heredity.h"
#include "Body.h"

#define RAND_NUM   100
#define RAND_FLAGE 2
/*********************************** HEAD CLASS FUNCTION ***********************************/
HEAD::HEAD() { //생성자
    Serial.println("Constructing Head...");
    hair_color = random(RAND_NUM);
    eye_color  = random(RAND_NUM);
    hair       = random(RAND_NUM);
    eyelid     = random(RAND_FLAGE);
    dimple     = random(RAND_FLAGE);
    bald       = random(RAND_FLAGE);
}
/*******************************************************************************************/
HEAD::~HEAD(){
    Serial.println("Destructing Head...");
}
/*******************************************************************************************/
void HEAD::change(HEAD *gene){
    hair_color = gene->hair_color;
    eye_color  = gene->eye_color;
    hair       = gene->hair;
    eyelid     = gene->eyelid;
    dimple     = gene->dimple;
    bald       = gene->bald;
}
/*******************************************************************************************/
void HEAD::meiosis(HEAD *mother, HEAD *father){
    hair_color = mutation_u8(mother->hair_color,father->hair_color);
    eye_color  = mutation_u8(mother->eye_color, father->eye_color);
    hair       = mutation_u8(mother->hair,      father->hair);
    eyelid     = mutation_bool(mother->eyelid,  father->eyelid);
    dimple     = mutation_bool(mother->dimple,  father->dimple);
    bald       = mutation_bool(mother->bald,    father->bald);
}
/*******************************************************************************************/
void HEAD::blend(HEAD *mother, HEAD *father, bool gender){
    hair_color = heredity_u8(mother->hair_color,father->hair_color);
    eye_color  = heredity_u8(mother->eye_color, father->eye_color);
    hair       = heredity_u8(mother->hair,      father->hair);
    eyelid     = mutation_bool(mother->eyelid,  father->eyelid);
    dimple     = mutation_bool(mother->dimple,  father->dimple);
    bald       = heredity_bool(mother->bald,    father->bald, true);
    if(!gender) bald = false;
}
/*******************************************************************************************/
void HEAD::status(){
    String color_hair;
    if(hair_color<22){color_hair = "Black";}
    else if(hair_color<41){color_hair = "Blue";}
    else if(hair_color<59){color_hair = "Orange";}
    else if(hair_color<74){color_hair = "Red";}
    else if(hair_color<88){color_hair = "Gold";}
    else{color_hair = "Silver";}
    String color_eye;
    if(eye_color<8){color_eye = "Gold";}
    else if(eye_color<27){color_eye = "Obsidian";}
    else if(eye_color<47){color_eye = "Topaz";}
    else if(eye_color<59){color_eye = "Emerald";}
    else if(eye_color<91){color_eye = "Sapphire";}
    else{color_eye = "Ruby";}
    Serial.println("*************************************");
    Serial.print("hair color: ");Serial.print(hair_color);Serial.print(" : ");Serial.println(color_hair);
    Serial.print("eye color : ");Serial.print(color_eye);Serial.print(" : ");Serial.println(eye_color);
    Serial.print("hair      : ");Serial.println(hair);
    Serial.print("eyelid    : ");Serial.println(eyelid);
    Serial.print("dimple    : ");Serial.println(dimple);
    Serial.print("bald      : ");Serial.println(bald);
}
/*********************************** HEAD CLASS FUNCTION ***********************************/
/*********************************** BODY CLASS FUNCTION ***********************************/
BODY::BODY(bool gender) { //생성자
    Serial.println("Constructing Body...");
    gen_xy     = gender;
    blood_A    = random(3);
    blood_B    = random(3);
    body_color = random(RAND_NUM);;
    breast     = gaussian_range(830,77);
    if(gender){
        height    = gaussian_range(1750,60);
        chest     = gaussian_range(930,53);
        waist     = gaussian_range(770,50);
        hip       = gaussian_range(930,43);
        leg_ratio = gaussian_range(453,20);
    }else{
        height    = gaussian_range(1612,59);
        chest     = gaussian_range(700,77);
        waist     = gaussian_range(600,65);
        hip       = gaussian_range(880,70);
        leg_ratio = gaussian_range(458,18);
    }
}
/*******************************************************************************************/
BODY::~BODY(){
    Serial.println("Destructing Body...");
}
/*******************************************************************************************/
void BODY::change(BODY *gene){
    blood_A    = gene->blood_A;
    blood_B    = gene->blood_B;
    body_color = gene->body_color;
    breast     = gene->breast;
    height     = gene->height;
    chest      = gene->chest;
    waist      = gene->waist;
    hip        = gene->hip;
    leg_ratio  = gene->leg_ratio;
}
/*******************************************************************************************/
void BODY::meiosis(BODY *mother, BODY *father){
    
    if(random(RAND_FLAGE) == 0){blood_A = mother->blood_A;}
    else{blood_A = mother->blood_B;}
    if(random(RAND_FLAGE) == 0){blood_B = father->blood_A;}
    else{blood_B = father->blood_B;}
    
    body_color = mutation_u8(mother->body_color, father->body_color);
    breast     = mother->breast;

    uint16_t temp_value = 0;

    if(gen_xy){
        temp_value = change_gender(1612,1750,60,mother->height,get_gaussian(1612,59,mother->height));
        height = mutation_u16(temp_value, father->height);

        temp_value = change_gender(700,930,53,mother->chest,get_gaussian(700,77,mother->chest));
        chest = mutation_u16(temp_value, father->chest);

        temp_value = change_gender(600,770,50,mother->waist,get_gaussian(600,65,mother->waist));
        waist = mutation_u16(temp_value, father->waist);

        temp_value = change_gender(880,930,43,mother->hip,get_gaussian(880,70,mother->hip));
        hip = mutation_u16(temp_value, father->hip);

        temp_value = change_gender(458,453,20,mother->waist,get_gaussian(458,18,mother->leg_ratio));
        leg_ratio = mutation_u16(temp_value, father->leg_ratio);
    }else{
        temp_value = change_gender(1750,1612,59,mother->waist,get_gaussian(1750,60,father->height));
        height = mutation_u16(mother->height, temp_value);

        temp_value = change_gender(930,700,77,mother->waist,get_gaussian(930,53,father->chest));
        chest = mutation_u16(mother->chest, temp_value);

        temp_value = change_gender(770,600,65,mother->waist,get_gaussian(770,50,father->waist));
        waist = mutation_u16(mother->waist, temp_value);

        temp_value = change_gender(930,880,70,mother->waist,get_gaussian(930,43,father->hip));
        hip = mutation_u16(mother->hip, temp_value);

        temp_value = change_gender(453,758,18,mother->waist,get_gaussian(453,20,father->leg_ratio));
        leg_ratio = mutation_u16(mother->leg_ratio, temp_value);
    }
}
/*******************************************************************************************/
void BODY::blend(BODY *mother, BODY *father, bool gender){

}
/*******************************************************************************************/
void BODY::status(){
    String blood_type;
    if(blood_A == 0){
        if(blood_B == 1){blood_type = "A";}
        else if(blood_B == 2){blood_type = "B";}
        else{blood_type = "O";}
    }else if(blood_A == 1){
        if(blood_B == 1){blood_type = "A";}
        else if(blood_B == 2){blood_type = "AB";}
        else{blood_type = "A";}
    }else if(blood_A == 2){
        if(blood_B == 1){blood_type = "AB";}
        else if(blood_B == 2){blood_type = "B";}
        else{blood_type = "B";}
    }
    String color_body;
    if(body_color<20){color_body = "Black";}
    else if(body_color<40){color_body = "brown";}
    else if(body_color<60){color_body = "medium";}
    else if(body_color<80){color_body = "fiar";}
    else{color_body = "light";}


    Serial.println("*************************************");
    Serial.print("blood type: ");Serial.println(blood_type);
    Serial.print("skin color: ");Serial.print(body_color);Serial.print(" : ");Serial.println(color_body);
    if(!gen_xy){Serial.print("breast    : ");Serial.println(breast);}
    Serial.print("height    : ");Serial.println(height);
    Serial.print("chest     : ");Serial.println(chest);
    Serial.print("waist     : ");Serial.println(waist);
    Serial.print("hip       : ");Serial.println(hip);
    Serial.print("leg_ratio : ");Serial.println(leg_ratio);
}
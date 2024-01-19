#include "heredity.h"
#include "utility.h"
#include "database.h"
#include "model_hard.h"

#define RAND_NUM   100
#define RAND_FLAGE 2
/*********************************** INFO CLASS FUNCTION ***********************************/
INFO::INFO(){}
/*******************************************************************************************/
INFO::~INFO(){destruct();}
/*******************************************************************************************/
void INFO::generate(bool gender, bool baby){
    gen_xy  = gender;
    name    = new_name(gen_xy);
    if(baby){
        family = "";
        age    = 19;
    }else{
        family = new_family();
        age    = random(20,36);
    }
}
/*******************************************************************************************/
void INFO::set_family(String familyname){
    family = familyname;
}
/*******************************************************************************************/
void INFO::aging(){
    age += 1;
}
/*******************************************************************************************/
void INFO::status(){
    perforation("info");
    spacebar(false,"gender");Serial.println(gen_xy);
    spacebar(false,"full name");Serial.print(family);Serial.println(name);
    spacebar(false,"age");Serial.println(age);
}
/*******************************************************************************************/
bool    INFO::get_gender(){return gen_xy;}
/*******************************************************************************************/
String  INFO::get_family(){return family;}
/*******************************************************************************************/
String  INFO::get_name(){return name;}
/*******************************************************************************************/
uint8_t INFO::get_age(){return age;}
/*********************************** HEAD CLASS FUNCTION ***********************************/
/*********************************** HEAD CLASS FUNCTION ***********************************/
HEAD::HEAD() {}
/*******************************************************************************************/
HEAD::~HEAD(){destruct();}
/*******************************************************************************************/
void HEAD::generate(bool gender){
    gen_xy     = gender;
    hair_color = random(RAND_NUM);
    eye_color  = random(RAND_NUM);
    hair_curl  = random(RAND_NUM);
    eyelid     = random(RAND_FLAGE);
    dimple     = random(RAND_FLAGE);
    bald       = random(RAND_FLAGE);
}
/*******************************************************************************************/
void HEAD::change(HEAD *gene){
    gen_xy     = gene->gen_xy;
    hair_color = gene->hair_color;
    eye_color  = gene->eye_color;
    hair_curl  = gene->hair_curl;
    eyelid     = gene->eyelid;
    dimple     = gene->dimple;
    bald       = gene->bald;
}
/*******************************************************************************************/
void HEAD::meiosis(HEAD *mother, HEAD *father){
    hair_color = mutation_u8(mother->hair_color,father->hair_color);
    eye_color  = mutation_u8(mother->eye_color, father->eye_color);
    hair_curl  = mutation_u8(mother->hair_curl, father->hair_curl);
    eyelid     = mutation_bool(mother->eyelid,  father->eyelid);
    dimple     = mutation_bool(mother->dimple,  father->dimple);
    bald       = mutation_bool(mother->bald,    father->bald);
}
/*******************************************************************************************/
void HEAD::blend(HEAD *mother, HEAD *father){
    hair_color = heredity_u8(mother->hair_color,father->hair_color);
    eye_color  = heredity_u8(mother->eye_color, father->eye_color);
    hair_curl  = heredity_u8(mother->hair_curl, father->hair_curl);
    eyelid     = heredity_bool(mother->eyelid,  father->eyelid, true);
    dimple     = heredity_bool(mother->dimple,  father->dimple, true);
    bald       = heredity_bool(mother->bald,    father->bald,   true);
    if(!gen_xy) bald = false;
}
/*******************************************************************************************/
void HEAD::status(){
    perforation("head");
    spacebar(false,"hair color"); Serial.println(hair_color);
    spacebar(false,"eye color");  Serial.println(eye_color);
    spacebar(false,"hair curl");  Serial.println(hair_curl);
    spacebar(false,"eyelid");     Serial.println(eyelid);
    spacebar(false,"dimple");     Serial.println(dimple);
    spacebar(false,"bald");       Serial.println(bald);
}
/*********************************** HEAD CLASS FUNCTION ***********************************/
String  HEAD::get_hair_color(){
    String color_hair;
    if(hair_color<22){color_hair = "Black";}
    else if(hair_color<41){color_hair = "Blue";}
    else if(hair_color<59){color_hair = "Orange";}
    else if(hair_color<74){color_hair = "Red";}
    else if(hair_color<88){color_hair = "Gold";}
    else{color_hair = "Silver";}
    return color_hair;
}
/*******************************************************************************************/
uint8_t HEAD::get_hair_curl(){return hair_curl;}
/*******************************************************************************************/
String  HEAD::get_eye_color(){
    String color_eye;
    if(eye_color<8){color_eye = "Gold";}
    else if(eye_color<27){color_eye = "Obsidian";}
    else if(eye_color<47){color_eye = "Topaz";}
    else if(eye_color<59){color_eye = "Emerald";}
    else if(eye_color<91){color_eye = "Sapphire";}
    else{color_eye = "Ruby";}
    return color_eye;
}
/*******************************************************************************************/
bool    HEAD::get_eyelid(){return eyelid;}
/*******************************************************************************************/
bool    HEAD::get_dimple(){return dimple;}
/*******************************************************************************************/
bool    HEAD::get_bald(){return bald;}
/*********************************** HEAD CLASS FUNCTION ***********************************/
/*********************************** BODY CLASS FUNCTION ***********************************/
BODY::BODY() {}
/*******************************************************************************************/
BODY::~BODY(){destruct();}
/*******************************************************************************************/
void BODY::generate(bool gender){
    gen_xy     = gender;
    blood_A    = random(3);
    blood_B    = random(3);
    body_color = random(RAND_NUM);;
    breast     = gaussian_range(830,77);
    if(gen_xy){
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
    if(chest < waist)               swap(&chest,&waist);
    if(!gen_xy && breast < chest)   swap(&chest,&breast);
}
/*******************************************************************************************/
void BODY::change(BODY *gene){
    gen_xy     = gene->gen_xy;
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
    breast     = mutation_u16(mother->breast, father->breast);

    uint16_t temp_value = 0;

    if(gen_xy){
        temp_value = change_gender(1612,1750,60,mother->height,get_gaussian(1612,59,mother->height));
        height     = mutation_u16(temp_value, father->height);

        temp_value = change_gender(700,930,53,mother->chest,get_gaussian(700,77,mother->chest));
        chest      = mutation_u16(temp_value, father->chest);

        temp_value = change_gender(600,770,50,mother->waist,get_gaussian(600,65,mother->waist));
        waist      = mutation_u16(temp_value, father->waist);

        temp_value = change_gender(880,930,43,mother->hip,get_gaussian(880,70,mother->hip));
        hip        = mutation_u16(temp_value, father->hip);

        temp_value = change_gender(458,453,20,mother->leg_ratio,get_gaussian(458,18,mother->leg_ratio));
        leg_ratio  = mutation_u16(temp_value, father->leg_ratio);
    }else{
        temp_value = change_gender(1750,1612,59,father->height,get_gaussian(1750,60,father->height));
        height     = mutation_u16(mother->height, temp_value);

        temp_value = change_gender(930,700,77,father->chest,get_gaussian(930,53,father->chest));
        chest      = mutation_u16(mother->chest, temp_value);

        temp_value = change_gender(770,600,65,father->waist,get_gaussian(770,50,father->waist));
        waist      = mutation_u16(mother->waist, temp_value);

        temp_value = change_gender(930,880,70,father->hip,get_gaussian(930,43,father->hip));
        hip        = mutation_u16(mother->hip, temp_value);

        temp_value = change_gender(453,758,18,father->leg_ratio,get_gaussian(453,20,father->leg_ratio));
        leg_ratio  = mutation_u16(mother->leg_ratio, temp_value);
    }
}
/*******************************************************************************************/
void BODY::blend(BODY *mother, BODY *father){
    if(random(RAND_FLAGE) == 0){blood_A = mother->blood_A;}
    else{blood_A = mother->blood_B;}
    if(random(RAND_FLAGE) == 0){blood_B = father->blood_A;}
    else{blood_B = father->blood_B;}
    
    body_color = heredity_u8(mother->body_color, father->body_color);
    breast     = heredity_u16(mother->breast, father->breast);

    uint16_t temp_value = 0;

    if(gen_xy){
        temp_value = change_gender(1612,1750,60,mother->height,get_gaussian(1612,59,mother->height));
        height     = heredity_u16(temp_value, father->height);

        temp_value = change_gender(700,930,53,mother->chest,get_gaussian(700,77,mother->chest));
        chest      = heredity_u16(temp_value, father->chest);

        temp_value = change_gender(600,770,50,mother->waist,get_gaussian(600,65,mother->waist));
        waist      = heredity_u16(temp_value, father->waist);

        temp_value = change_gender(880,930,43,mother->hip,get_gaussian(880,70,mother->hip));
        hip        = heredity_u16(temp_value, father->hip);

        temp_value = change_gender(458,453,20,mother->leg_ratio,get_gaussian(458,18,mother->leg_ratio));
        leg_ratio  = heredity_u16(temp_value, father->leg_ratio);
    }else{
        temp_value = change_gender(1750,1612,59,father->height,get_gaussian(1750,60,father->height));
        height     = heredity_u16(mother->height, temp_value);

        temp_value = change_gender(930,700,77,father->chest,get_gaussian(930,53,father->chest));
        chest      = heredity_u16(mother->chest, temp_value);

        temp_value = change_gender(770,600,65,father->waist,get_gaussian(770,50,father->waist));
        waist      = heredity_u16(mother->waist, temp_value);

        temp_value = change_gender(930,880,70,father->hip,get_gaussian(930,43,father->hip));
        hip        = heredity_u16(mother->hip, temp_value);

        temp_value = change_gender(453,758,18,father->leg_ratio,get_gaussian(453,20,father->leg_ratio));
        leg_ratio  = heredity_u16(mother->leg_ratio, temp_value);
    }

    if(chest < waist)               swap(&chest,&waist);
    if(!gen_xy && breast < chest)   swap(&chest,&breast);
}
/*******************************************************************************************/
void BODY::status(){
    perforation("body");
    spacebar(false,"bloodA");     Serial.println(blood_A);
    spacebar(false,"bloodB");     Serial.println(blood_B);
    spacebar(false,"skin color"); Serial.println(body_color);
    spacebar(false,"height");     Serial.print(height);unit_mm();
    if(gen_xy){
        spacebar(false,"hidden");Serial.print(breast);unit_mm();
    }else{
        spacebar(false,"breast");Serial.print(breast);unit_mm();
    }
    spacebar(false,"chest");      Serial.print(chest);unit_mm();
    spacebar(false,"waist");      Serial.print(waist);unit_mm();
    spacebar(false,"hip");        Serial.print(hip);unit_mm();
    spacebar(false,"leg ratio");  unit_split(leg_ratio,10);Serial.println(" %");
}
/*******************************************************************************************/
String   BODY::get_blood(){
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
    return blood_type;
}
/*******************************************************************************************/
uint16_t BODY::get_weight(){
    float math_pi   = 3.14159265359;
    float height_c  = float(height);
    float chest_c   = float(chest);
    float waist_c   = float(waist);
    float hip_c     = float(hip);
    
    float id_weight   = (height_c / 100.0) * (height_c / 100.0) * 19.0;
    float id_bust     = height_c  * 0.515;
    float id_waist_c    = height_c  * 0.37;
    float id_hip_c      = height_c  * 0.545;

    float tmp_ba = math_pi * (chest_c  / 2.0 / math_pi) * (chest_c / 2.0 / math_pi);
    float tmp_wa = 2.0 * math_pi * (waist_c / 2.0 / math_pi) * (waist_c / 2.0 / math_pi);
    float tmp_ha = 2.0 * math_pi * (hip_c / 2.0 / math_pi) * (hip_c / 2.0 / math_pi);

    float tmp_bb = math_pi * (id_bust / 2.0 / math_pi) * (id_bust / 2.0 / math_pi);
    float tmp_wb = 2.0 * math_pi * (id_waist_c / 2.0 / math_pi) * (id_waist_c / 2.0 / math_pi);
    float tmp_hb = 2.0 * math_pi * (id_hip_c / 2.0 / math_pi) * (id_hip_c / 2.0 / math_pi);

    float ans = id_weight * (tmp_ba + tmp_wa + tmp_ha) / (tmp_bb + tmp_wb + tmp_hb);

    if(gen_xy){
        ans*=1.05;
    }else{
        float cupsize      = breast - chest;
        float diameter     = chest/13;
        float hight_breast = 0;
        if(cupsize*cupsize/4.9>diameter*diameter){hight_breast = sqrt(cupsize*cupsize/4.9 - diameter*diameter);}
        float hight_cal    = (2*hight_breast + cupsize)/5;
        if(hight_breast>cupsize/3){hight_cal = hight_breast;}
        float cup_w     = (hight_cal/3)*(diameter/10)*(diameter/50);
        ans += cup_w;
        spacebar(false,"cup weight");unit_split(cup_w,100);Serial.println(" kg");
    }
    float BMI = ans/((height_c/100)*(height_c/100));
    spacebar(false,"weight"); unit_split(ans,100);Serial.println(" kg");
    spacebar(false,"BMI");    Serial.println(BMI);
    return ans;
}
uint16_t BODY::get_BMI(uint16_t weight){
    float BMI = weight/(height/10*height/10);
    return BMI;
}
/*******************************************************************************************/
String   BODY::get_body_color(){
    String color_body;
    if(body_color<20){color_body = "Black";}
    else if(body_color<40){color_body = "brown";}
    else if(body_color<60){color_body = "medium";}
    else if(body_color<80){color_body = "fiar";}
    else{color_body = "light";}
    return color_body;
}
/*******************************************************************************************/
String   BODY::get_cup(){
    String   cup     = "NULL";
    if(!gen_xy){
        uint16_t cupsize = breast - chest;
        if (cupsize < 75)      { cup = "AA";}
        else if (cupsize <100) { cup = "A";}
        else if (cupsize <125) { cup = "B";}
        else if (cupsize <150) { cup = "C";}
        else if (cupsize <175) { cup = "D";}
        else if (cupsize <200) { cup = "E";}
        else if (cupsize <225) { cup = "F";}
        else if (cupsize <250) { cup = "G";}
        else if (cupsize <275) { cup = "H";}
        else if (cupsize <300) { cup = "I";}
        else if (cupsize <325) { cup = "J";}
        else if (cupsize <350) { cup = "K";}
        else if (cupsize <375) { cup = "L";}
        else if (cupsize <400) { cup = "M";}
        else if (cupsize <425) { cup = "N";}
        else if (cupsize <450) { cup = "O";}
        else if (cupsize <475) { cup = "P";}
        else if (cupsize <500) { cup = "Q";}
        else if (cupsize <525) { cup = "R";}
        else if (cupsize <550) { cup = "S";}
        else if (cupsize <575) { cup = "T";}
        else if (cupsize <600) { cup = "U";}
        else if (cupsize <625) { cup = "V";}
        else if (cupsize <650) { cup = "W";}
        else if (cupsize <675) { cup = "X";}
        else if (cupsize <700) { cup = "Y";}
    }
    return cup;
}
/*******************************************************************************************/
uint16_t BODY::get_breast(){return breast;}
/*******************************************************************************************/
uint16_t BODY::get_height(){return height;}
/*******************************************************************************************/
uint16_t BODY::get_chest(){return chest;}
/*******************************************************************************************/
uint16_t BODY::get_waist(){return waist;}
/*******************************************************************************************/
uint16_t BODY::get_hip(){return hip;}
/*******************************************************************************************/
uint16_t BODY::get_leg_ratio(){return leg_ratio;}
/*********************************** BODY CLASS FUNCTION ***********************************/
/*********************************** EROGENOUS CLASS FUNCTION ******************************/
EROGENOUS::EROGENOUS(){}
/*******************************************************************************************/
EROGENOUS::~EROGENOUS(){destruct();}
/*******************************************************************************************/
void EROGENOUS::generate(bool gender){
    gen_xy          = gender;
    
    hood_shape          = random(RAND_NUM);
    hood_start      = random(RAND_NUM);
    hood_texture    = random(RAND_NUM);
    hood_width      = gaussian_range(1400,30);
    hood_length     = gaussian_range(2600,400);

    lip_i_shape     = random(RAND_NUM);
    lip_i_texture   = random(RAND_NUM);
    lip_i_width     = gaussian_range(500,100);
    lip_i_length    = gaussian_range(5200,800);
    lip_i_length_r  = gaussian_range(1900,450);
    lip_i_length_l  = gaussian_range(1900,450);
    lip_o_shape     = random(RAND_NUM);
    lip_o_texture   = random(RAND_NUM);

    clit_d          = gaussian_range(340,10);
    clit_d_e        = gaussian_range(390,15);
    clit_l          = gaussian_range(510,14);
    clit_l_e        = gaussian_range(667,28);
    if(clit_d > clit_d_e) swap(&clit_d,&clit_d_e);
    if(clit_l > clit_l_e) swap(&clit_l,&clit_l_e);
    if(hood_width/10*5 < clit_d)   clit_d   = hood_width/10*5;
    if(hood_width/10*7 < clit_d_e) clit_d_e = hood_width/10*7;
    if(hood_width/10*7 < clit_l)   clit_l   = hood_width/10*7;
    if(hood_width < clit_l_e)      clit_l_e = hood_width;
    
    glans_d         = gaussian_range(2600,200);
    glans_d_e       = gaussian_range(3400,400);
    glans_l         = gaussian_range(2700,300);
    glans_l_e       = gaussian_range(3100,400);
    
    if(glans_d > glans_d_e) swap(&glans_d,&glans_d_e);
    if(glans_l > glans_l_e) swap(&glans_l,&glans_l_e);

    body_d          = gaussian_range(2960,290);
    body_d_e        = gaussian_range(3710,350);
    body_l          = gaussian_range(9160,157);
    body_l_e        = gaussian_range(1312,166);
    if(body_d > body_d_e) swap(&body_d,&body_d_e);
    if(body_l > body_l_e) swap(&body_l,&body_l_e);
    ball_r          = gaussian_range(1350,250);
    ball_l          = gaussian_range(1350,250);
    
    prepuce         = random(RAND_FLAGE);
    invert          = random(RAND_FLAGE);
    fork            = random(RAND_NUM);

    perineum        = random(RAND_NUM);
    wrinkle         = gaussian_range(36,6);
    areola          = gaussian_range(3090,400);
    nipple_d        = gaussian_range(1120,130);
    nipple_d_e      = gaussian_range(1570,130);
    nipple_h        = gaussian_range(900,200);
    nipple_h_e      = gaussian_range(1550,200);
    if(gen_xy){
        areola      = gaussian_range(2600,250);
        nipple_d    = gaussian_range(680,130);
        nipple_d_e  = gaussian_range(755,130);
        nipple_h    = gaussian_range(200,40);
        nipple_h_e  = gaussian_range(350,40);
    }
    if(nipple_d > nipple_d_e) swap(&nipple_d,&nipple_d_e);
    if(nipple_h > nipple_h_e) swap(&nipple_h,&nipple_h_e);
}
/*******************************************************************************************/
void EROGENOUS::change(EROGENOUS *gene){
    gen_xy          = gene->gen_xy;
    hood_shape          = gene->hood_shape;
    hood_start      = gene->hood_start;
    hood_texture    = gene->hood_texture;
    hood_width      = gene->hood_width;
    hood_length     = gene->hood_length;
    lip_i_shape     = gene->lip_i_shape;
    lip_i_texture   = gene->lip_i_texture;
    lip_i_width     = gene->lip_i_width;
    lip_i_length    = gene->lip_i_length;
    lip_i_length_r  = gene->lip_i_length_r;
    lip_i_length_l  = gene->lip_i_length_l;
    lip_o_shape     = gene->lip_o_shape;
    lip_o_texture   = gene->lip_o_texture;
    clit_d          = gene->clit_d;
    clit_d_e        = gene->clit_d_e;
    clit_l          = gene->clit_l;
    clit_l_e        = gene->clit_l_e;
    glans_d         = gene->glans_d;
    glans_d_e       = gene->glans_d_e;
    glans_l         = gene->glans_l;
    glans_l_e       = gene->glans_l_e;
    body_d          = gene->body_d;
    body_d_e        = gene->body_d_e;
    body_l          = gene->body_l;
    body_l_e        = gene->body_l_e;
    ball_r          = gene->ball_r;
    ball_l          = gene->ball_l;
    prepuce         = gene->prepuce;
    invert          = gene->invert;
    fork            = gene->fork;
    perineum        = gene->perineum;
    wrinkle         = gene->wrinkle;
    areola          = gene->areola;
    nipple_d        = gene->nipple_d;
    nipple_d_e      = gene->nipple_d_e;
    nipple_h        = gene->nipple_h;
    nipple_h_e      = gene->nipple_h_e;
}
/*******************************************************************************************/
void EROGENOUS::meiosis(EROGENOUS *mother, EROGENOUS *father){
    hood_shape          = mutation_u8(mother->hood_shape,           father->hood_shape);
    hood_start      = mutation_u8(mother->hood_start,       father->hood_start);
    hood_texture    = mutation_u8(mother->hood_texture,     father->hood_texture);

    hood_width      = mutation_u16(mother->hood_width,      father->hood_width);
    hood_length     = mutation_u16(mother->hood_length,     father->hood_length);

    lip_i_shape     = mutation_u8(mother->lip_i_shape,      father->lip_i_shape);
    lip_i_texture   = mutation_u8(mother->lip_i_texture,    father->lip_i_texture);
    lip_i_width     = mutation_u16(mother->lip_i_width,     father->lip_i_width);
    lip_i_length    = mutation_u16(mother->lip_i_length,    father->lip_i_length);
    lip_i_length_r  = mutation_u16(mother->lip_i_length_r,  father->lip_i_length_r);
    lip_i_length_l  = mutation_u16(mother->lip_i_length_l,  father->lip_i_length_l);
    lip_o_shape     = mutation_u8(mother->lip_o_shape,      father->lip_o_shape);
    lip_o_texture   = mutation_u8(mother->lip_o_texture,    father->lip_o_texture);
    
    clit_d          = mutation_u16(mother->clit_d,      father->clit_d);
    clit_d_e        = mutation_u16(mother->clit_d_e,    father->clit_d_e);
    clit_l          = mutation_u16(mother->clit_l,      father->clit_l);
    clit_l_e        = mutation_u16(mother->clit_l_e,    father->clit_l_e);

    glans_d         = mutation_u16(mother->glans_d,     father->glans_d);
    glans_d_e       = mutation_u16(mother->glans_d_e,   father->glans_d_e);
    glans_l         = mutation_u16(mother->glans_l,     father->glans_l);
    glans_l_e       = mutation_u16(mother->glans_l_e,   father->glans_l_e);

    body_d          = mutation_u16(mother->body_d,      father->body_d);
    body_d_e        = mutation_u16(mother->body_d_e,    father->body_d_e);
    body_l          = mutation_u16(mother->body_l,      father->body_l);
    body_l_e        = mutation_u16(mother->body_l_e,    father->body_l_e);
    ball_r          = mutation_u8(mother->ball_r,       father->ball_r);
    ball_l          = mutation_u8(mother->ball_l,       father->ball_l);

    prepuce         = mutation_bool(mother->prepuce,    father->prepuce);
    invert          = mutation_bool(mother->invert,     father->invert);
    fork            = mutation_u8(mother->fork,         father->fork);

    perineum        = mutation_u8(mother->perineum,     father->perineum);
    wrinkle         = mutation_u8(mother->wrinkle,      father->wrinkle);

    areola          = mutation_u16(mother->areola,      father->areola);
    nipple_d        = mutation_u16(mother->nipple_d,    father->nipple_d);
    nipple_d_e      = mutation_u16(mother->nipple_d_e,  father->nipple_d_e);
    nipple_h        = mutation_u16(mother->nipple_h,    father->nipple_h);
    nipple_h_e      = mutation_u16(mother->nipple_h_e,  father->nipple_h_e);
}
/*******************************************************************************************/
void EROGENOUS::blend(EROGENOUS *mother, EROGENOUS *father){
    hood_shape          = heredity_u8(mother->hood_shape,           father->hood_shape);
    hood_start      = heredity_u8(mother->hood_start,       father->hood_start);
    hood_texture    = heredity_u8(mother->hood_texture,     father->hood_texture);

    hood_width      = heredity_u16(mother->hood_width,      father->hood_width);
    hood_length     = heredity_u16(mother->hood_length,     father->hood_length);

    lip_i_shape     = heredity_u8(mother->lip_i_shape,      father->lip_i_shape);
    lip_i_texture   = heredity_u8(mother->lip_i_texture,    father->lip_i_texture);
    lip_i_width     = heredity_u16(mother->lip_i_width,     father->lip_i_width);
    lip_i_length    = heredity_u16(mother->lip_i_length,    father->lip_i_length);
    lip_i_length_r  = heredity_u16(mother->lip_i_length_r,  father->lip_i_length_r);
    lip_i_length_l  = heredity_u16(mother->lip_i_length_l,  father->lip_i_length_l);
    lip_o_shape     = heredity_u8(mother->lip_o_shape,      father->lip_o_shape);
    lip_o_texture   = heredity_u8(mother->lip_o_texture,    father->lip_o_texture);
    
    clit_d          = heredity_u16(mother->clit_d,      father->clit_d);
    clit_d_e        = heredity_u16(mother->clit_d_e,    father->clit_d_e);
    clit_l          = heredity_u16(mother->clit_l,      father->clit_l);
    clit_l_e        = heredity_u16(mother->clit_l_e,    father->clit_l_e);

    glans_d         = heredity_u16(mother->glans_d,     father->glans_d);
    glans_d_e       = heredity_u16(mother->glans_d_e,   father->glans_d_e);
    glans_l         = heredity_u16(mother->glans_l,     father->glans_l);
    glans_l_e       = heredity_u16(mother->glans_l_e,   father->glans_l_e);

    body_d          = heredity_u16(mother->body_d,      father->body_d);
    body_d_e        = heredity_u16(mother->body_d_e,    father->body_d_e);
    body_l          = heredity_u16(mother->body_l,      father->body_l);
    body_l_e        = heredity_u16(mother->body_l_e,    father->body_l_e);
    ball_r          = heredity_u16(mother->ball_r,      father->ball_r);
    ball_l          = heredity_u16(mother->ball_l,      father->ball_l);

    prepuce         = heredity_bool(mother->prepuce,    father->prepuce,    true);
    invert          = heredity_bool(mother->invert,     father->invert,     false);
    fork            = heredity_u8(mother->fork,         father->fork);

    perineum        = heredity_u8(mother->perineum,     father->perineum);
    wrinkle         = heredity_u8(mother->wrinkle,      father->wrinkle);

    areola          = heredity_u16(mother->areola,      father->areola);
    nipple_d        = heredity_u16(mother->nipple_d,    father->nipple_d);
    nipple_d_e      = heredity_u16(mother->nipple_d_e,  father->nipple_d_e);
    nipple_h        = heredity_u16(mother->nipple_h,    father->nipple_h);
    nipple_h_e      = heredity_u16(mother->nipple_h_e,  father->nipple_h_e);
}
void EROGENOUS::status(){
    perforation("erogenous");
    if(gen_xy){
        spacebar(false,"glans_d");    unit_split(glans_d,100);unit_mm();
        spacebar(false,"glans_d_e");  unit_split(glans_d_e,100);unit_mm();
        spacebar(false,"glans_l");    unit_split(glans_l,100);unit_mm();
        spacebar(false,"glans_l_e");  unit_split(glans_l_e,100);unit_mm();
        
        spacebar(false,"body_d");     unit_split(body_d,100);unit_mm();
        spacebar(false,"body_d_e");   unit_split(body_d_e,100);unit_mm();
        spacebar(false,"body_l");     unit_split(body_l,100);unit_mm();
        spacebar(false,"body_l_e");   unit_split(body_l_e,100);unit_mm();

        spacebar(false,"ball_r");     unit_split(ball_r,100);Serial.println(" ml");
        spacebar(false,"ball_l");     unit_split(ball_l,100);Serial.println(" ml");

        spacebar(false,"prepuce");    Serial.println(prepuce);
    }else{
        spacebar(false,"hood_shape");     Serial.println(hood_shape);
        spacebar(false,"hood_start"); Serial.println(hood_start);
        spacebar(false,"hood_texture");Serial.println(hood_texture);
        spacebar(false,"hood_width"); unit_split(hood_width,100);unit_mm();
        spacebar(false,"hood_length");unit_split(hood_length,100);unit_mm();

        spacebar(false,"clit_d");     unit_split(clit_d,100);unit_mm();
        spacebar(false,"clit_d_e");   unit_split(clit_d_e,100);unit_mm();
        spacebar(false,"clit_l");     unit_split(clit_l,100);unit_mm();
        spacebar(false,"clit_l_e");   unit_split(clit_l_e,100);unit_mm();

        spacebar(false,"wing_shape"); Serial.println(lip_i_shape);
        spacebar(false,"wing_texture");Serial.println(lip_i_texture);
        spacebar(false,"wing_width"); unit_split(lip_i_width,100);unit_mm();
        spacebar(false,"wing_length");unit_split(lip_i_length,100);unit_mm();
        spacebar(false,"wing_r");     unit_split(lip_i_length_r,100);unit_mm();
        spacebar(false,"wing_l");     unit_split(lip_i_length_l,100);unit_mm();
        spacebar(false,"lip_shape");  Serial.println(lip_o_shape);
        spacebar(false,"lip_texture");Serial.println(lip_o_texture);
    }
    spacebar(false,"invert");     Serial.println(invert);
    spacebar(false,"perineum");   Serial.println(perineum);
    spacebar(false,"wrinkle");    Serial.println(wrinkle);
    
    spacebar(false,"areola");     unit_split(areola,100);unit_mm();
    spacebar(false,"nipple_d");   unit_split(nipple_d,100);unit_mm();
    spacebar(false,"nipple_d_e"); unit_split(nipple_d_e,100);unit_mm();
    spacebar(false,"nipple_h");   unit_split(nipple_h,100);unit_mm();
    spacebar(false,"nipple_h_e"); unit_split(nipple_h_e,100);unit_mm();
}
/*******************************************************************************************/
uint16_t EROGENOUS::get_hood_width(){return hood_width;}
/*******************************************************************************************/
uint16_t EROGENOUS::get_hood_length(){return hood_length;}
/*******************************************************************************************/
String   EROGENOUS::get_hood(){
    String hood = "";
    if(hood_start < 15){hood+="둥근 모양의 시작점에서";}
    else if(hood_start < 38){hood+="삼각형으로 뻗어";}
    else if(hood_start < 93){hood+="일자로 뻗어";}
    else{hood+="역상각형으로";}
    hood+=" 클리토리스까지 이어지는 ";
    if(hood_texture < 11){hood+="반들반들한 질감의";}
    else if(hood_texture < 43){hood+="매끄러운 질감의";}
    else if(hood_texture < 88){hood+="보드라운 질감의";}
    else{hood+="주름이 가득한";}
    hood+=", ";
    if(hood_shape < 17){hood+="살짝 접힌 모양의 포피가";}
    else if(hood_shape < 55){hood+="완전히 접힌 모양의 포피가";}
    else if(hood_shape < 60){hood+="매끄러운 포피가 부드럽게";}
    else if(hood_shape < 78){hood+="두겹으로 주름진 포피가 살짝";}
    else if(hood_shape < 98){hood+="두겹으로 주름진 포피가 완전히";}
    else{hood+="두겹으로 부드럽게 주름진 포피가";}
    hood+=" 클리토리스를 덮고 있다.";
    Serial.println(hood);
    return hood;
}
/*******************************************************************************************/
String   EROGENOUS::get_lips(){
    String lip = "";
    if(lip_o_texture < 7){lip+="반들거리고";}
    else if(lip_o_texture < 26){lip+="매끈하고";}
    else if(lip_o_texture < 75){lip+="보드랍고";}
    else{lip+="부드럽고";}
    lip += " ";
    if(lip_o_shape < 8){lip+="말랑한";}
    else if(lip_o_shape < 68){lip+="도톰한";}
    else{lip+="통통한";}
    lip += " 대음순의 안쪽에, 클리토리스 아래로 ";
    if(lip_i_shape < 22){lip+="앙다문 일자의";}
    else if(lip_i_shape < 37){lip+="둥글게 펼쳐진";}
    else if(lip_i_shape < 47){lip+="다이아몬드 같은 마르모 모양의";}
    else if(lip_i_shape < 80){lip+="도자기 모양의";}
    else if(lip_i_shape < 88){lip+="하트 모양의";}
    else if(lip_i_shape < 96){lip+="잠자리 모양같은";}
    else{lip+="호리병 모양같은";}
    lip += " ";
    if(lip_i_texture < 7){lip+="아름다운";}
    else if(lip_i_texture < 26){lip+="매력적인";}
    else if(lip_i_texture < 75){lip+="음란한";}
    else{lip+="불고기같은";}
    lip += " 소음순이 질구 아래에서 ";
    if(fork < 8){lip+="매끄럽게";}
    else if(fork < 68){lip+="둥글게 주름져";}
    else{lip+="잔뜩 주름져";}
    lip += " 만난다";
    Serial.println(lip);
    return lip;
}
/*******************************************************************************************/
uint16_t EROGENOUS::get_wing(uint8_t item){
    if(item == 1) return lip_i_width;
    else if(item == 2) return lip_i_length;
    else if(item == 3) return lip_i_length_r;
    else if(item == 4) return lip_i_length_l;
}
/*******************************************************************************************/
uint16_t EROGENOUS::get_clit(uint8_t item){
    if(item == 1) return clit_d;
    else if(item == 2) return clit_d_e;
    else if(item == 3) return clit_l;
    else if(item == 4) return clit_l_e;
}
/*******************************************************************************************/
uint16_t EROGENOUS::get_glans(uint8_t item){
    if(item == 1) return glans_d;
    else if(item == 2) return glans_d_e;
    else if(item == 3) return glans_l;
    else if(item == 4) return glans_l_e;
}
/*******************************************************************************************/
uint16_t EROGENOUS::get_body(uint8_t item){
    if(item == 1) return body_d;
    else if(item == 2) return body_d_e;
    else if(item == 3) return body_l;
    else if(item == 4) return body_l_e;
}
/*******************************************************************************************/
uint8_t  EROGENOUS::get_balls(bool item){
    if(item) return ball_r;
    return ball_l;
}
/*******************************************************************************************/
bool     EROGENOUS::get_prepuce(){return prepuce;}
/*******************************************************************************************/
bool     EROGENOUS::get_invert(){return invert;}
/*******************************************************************************************/
String   EROGENOUS::get_perineum(){
    String perineum_txt = "회음부는 ";
    if(perineum < 66){perineum_txt+="매끄럽다.";}
    else if(perineum < 83){perineum_txt+="회음봉선의 흔적이 보인다.";}
    else if(perineum < 93){perineum_txt+="회음봉선이 항문까지 이어진다.";}
    else{perineum_txt+="도드라지는 회음봉선이 항문까지 이어진다.";}
    return perineum_txt ;
}
/*******************************************************************************************/
uint8_t EROGENOUS::get_wrinkle(){return wrinkle;}
/*******************************************************************************************/
uint16_t EROGENOUS::get_areola(){return areola;}
/*******************************************************************************************/
uint16_t EROGENOUS::get_nipple(uint8_t item){
    if(item == 1) return nipple_d;
    else if(item == 2) return nipple_d_e;
    else if(item == 3) return nipple_h;
    else if(item == 4) return nipple_h_e;
}
/*********************************** EROGENOUS CLASS FUNCTION ******************************/
/*********************************** STAT CLASS FUNCTION ***********************************/
STAT::STAT(){}
/*******************************************************************************************/
STAT::~STAT(){destruct();}
/*******************************************************************************************/
void STAT::generate(){
    intelligence = gaussian_range(50,12);
    strength     = gaussian_range(50,12);
    dexterity    = gaussian_range(50,12);
    charisma     = gaussian_range(50,12);
    constitution = gaussian_range(50,12);
}
/*******************************************************************************************/
void STAT::change(STAT *gene){
    intelligence = gene->intelligence;
    strength     = gene->strength;
    dexterity    = gene->dexterity;
    charisma     = gene->charisma;
    constitution = gene->constitution;
}
/*******************************************************************************************/
void STAT::meiosis(STAT *mother, STAT *father){
    intelligence = mutation_u8(mother->intelligence, father->intelligence); 
    strength     = mutation_u8(mother->strength,     father->strength); 
    dexterity    = mutation_u8(mother->dexterity,    father->dexterity); 
    charisma     = mutation_u8(mother->charisma,     father->charisma); 
    constitution = mutation_u8(mother->constitution, father->constitution); 
}
/*******************************************************************************************/
void STAT::blend(STAT *mother, STAT *father){
    intelligence = heredity_u8(mother->intelligence, father->intelligence); 
    strength     = heredity_u8(mother->strength,     father->strength); 
    dexterity    = heredity_u8(mother->dexterity,    father->dexterity); 
    charisma     = heredity_u8(mother->charisma,     father->charisma); 
    constitution = heredity_u8(mother->constitution, father->constitution); 
}
/*******************************************************************************************/
void STAT::status(){
    perforation("stat");
    spacebar(false,"intelligence");   Serial.println(intelligence);
    spacebar(false,"strength");       Serial.println(strength);
    spacebar(false,"dexterity");      Serial.println(dexterity);
    spacebar(false,"charisma");       Serial.println(charisma);
    spacebar(false,"constitution");   Serial.println(constitution);
}
/*******************************************************************************************/
uint8_t STAT::get_intelligence(){return intelligence;}
/*******************************************************************************************/
uint8_t STAT::get_strength(){return strength;}
/*******************************************************************************************/
uint8_t STAT::get_dexterity(){return dexterity;}
/*******************************************************************************************/
uint8_t STAT::get_charisma(){return charisma;}
/*******************************************************************************************/
uint8_t STAT::get_constitution(){return constitution;}
/*********************************** STAT CLASS FUNCTION ***********************************/
/*********************************** HOLE CLASS FUNCTION ***********************************/
HOLE::HOLE(){}
/*******************************************************************************************/
HOLE::~HOLE(){destruct();}
/*******************************************************************************************/
void HOLE::generate(){
    gape_u = gaussian_range(650,40);
    gape_v = gaussian_range(5550,400);
    gape_a = gaussian_range(6950,800);
    pressure_u = gaussian_range(2500,300);
    pressure_v = gaussian_range(6950,800);
    pressure_a = gaussian_range(9220,1300);
}
/*******************************************************************************************/
void HOLE::change(HOLE *gene){
    gape_u     = gene->gape_u;
    gape_v     = gene->gape_v;
    gape_a     = gene->gape_a;
    pressure_u = gene->pressure_u;
    pressure_v = gene->pressure_v;
    pressure_a = gene->pressure_a;
}
/*******************************************************************************************/
void HOLE::meiosis(HOLE *mother, HOLE *father){
    gape_u      = mutation_u16(mother->gape_u,      father->gape_u); 
    gape_v      = mutation_u16(mother->gape_v,      father->gape_v);
    gape_a      = mutation_u16(mother->gape_a,      father->gape_a);
    pressure_u  = mutation_u16(mother->pressure_u,  father->pressure_u);
    pressure_v  = mutation_u16(mother->pressure_v,  father->pressure_v);
    pressure_a  = mutation_u16(mother->pressure_a,  father->pressure_a);
}
/*******************************************************************************************/
void HOLE::blend(HOLE *mother, HOLE *father){
    gape_u      = heredity_u16(mother->gape_u,      father->gape_u);
    gape_v      = heredity_u16(mother->gape_v,      father->gape_v);
    gape_a      = heredity_u16(mother->gape_a,      father->gape_a);
    pressure_u  = heredity_u16(mother->pressure_u,  father->pressure_u);
    pressure_v  = heredity_u16(mother->pressure_v,  father->pressure_v);
    pressure_a  = heredity_u16(mother->pressure_a,  father->pressure_a);
}
/*******************************************************************************************/
void HOLE::status(bool gender){
    perforation("hole");
    if(!gender){
        spacebar(false,"gape_v");     unit_split(gape_v,100);unit_mm();
        spacebar(false,"pressure_v"); unit_split(pressure_v,10);Serial.println(" torr");
    }
    spacebar(false,"gape_u");     unit_split(gape_u,100);unit_mm();
    spacebar(false,"pressure_u"); unit_split(pressure_u,10);Serial.println(" torr");
    spacebar(false,"gape_a");     unit_split(gape_a,100);unit_mm();
    spacebar(false,"pressure_a"); unit_split(gape_v,10);Serial.println(" torr");
}
/*******************************************************************************************/
uint16_t HOLE::get_gape(uint8_t item){
    if(item == 1) return gape_u;
    else if(item == 2) return gape_v;
    else if(item == 3) return gape_a;
}
/*******************************************************************************************/
uint16_t HOLE::get_pressure(uint8_t item){
    if(item == 1) return pressure_u;
    else if(item == 2) return pressure_v;
    else if(item == 3) return pressure_a;
}
/*********************************** HOLE CLASS FUNCTION ***********************************/
/*********************************** SENSE CLASS FUNCTION **********************************/
SENSE::SENSE(){}
/*******************************************************************************************/
SENSE::~SENSE(){destruct();}
/*******************************************************************************************/
void SENSE::generate(){
    cervix          = gaussian_range(50,12);
    skin            = gaussian_range(50,12);
    vagina_balls    = gaussian_range(50,12);
    urethra         = gaussian_range(50,12);
    anal            = gaussian_range(50,12);
    nipple          = gaussian_range(50,12);
}
/*******************************************************************************************/
void SENSE::change(SENSE *gene){
    cervix       = gene->cervix;
    skin         = gene->skin;
    vagina_balls = gene->vagina_balls;
    urethra      = gene->urethra;
    anal         = gene->anal;
    nipple       = gene->nipple;
}
/*******************************************************************************************/
void SENSE::meiosis(SENSE *mother, SENSE *father){
    cervix          = mutation_u8(mother->cervix,       father->cervix); 
    skin            = mutation_u8(mother->skin,         father->skin); 
    vagina_balls    = mutation_u8(mother->vagina_balls, father->vagina_balls); 
    urethra         = mutation_u8(mother->urethra,      father->urethra); 
    anal            = mutation_u8(mother->anal,         father->anal); 
    nipple          = mutation_u8(mother->nipple,       father->nipple); 
}
/*******************************************************************************************/
void SENSE::blend(SENSE *mother, SENSE *father){
    cervix          = heredity_u8(mother->cervix,       father->cervix); 
    skin            = heredity_u8(mother->skin,         father->skin); 
    vagina_balls    = heredity_u8(mother->vagina_balls, father->vagina_balls); 
    urethra         = heredity_u8(mother->urethra,      father->urethra); 
    anal            = heredity_u8(mother->anal,         father->anal); 
    nipple          = heredity_u8(mother->nipple,       father->nipple);  
}
/*******************************************************************************************/
void SENSE::status(bool gender){
    perforation("sense");
    if(gender){
        spacebar(false,"balls");  Serial.println(vagina_balls);
    }else{
        spacebar(false,"cervix"); Serial.println(cervix);
        spacebar(false,"vagina"); Serial.println(vagina_balls);
    }
    spacebar(false,"skin");   Serial.println(skin);
    spacebar(false,"urethra");Serial.println(urethra);
    spacebar(false,"anal");   Serial.println(anal);
    spacebar(false,"nipple"); Serial.println(nipple);
}
/*******************************************************************************************/
uint8_t SENSE::get_cervix(){return cervix;}
/*******************************************************************************************/
uint8_t SENSE::get_skin(){return skin;}
/*******************************************************************************************/
uint8_t SENSE::get_clit_glans(){return clit_glans;}
/*******************************************************************************************/
uint8_t SENSE::get_vagina_balls(){return vagina_balls;}
/*******************************************************************************************/
uint8_t SENSE::get_urethra(){return urethra;}
/*******************************************************************************************/
uint8_t SENSE::get_anal(){return anal;}
/*******************************************************************************************/
uint8_t SENSE::get_nipple(){return nipple;}
/*********************************** SENSE CLASS FUNCTION **********************************/
/*********************************** NATURE CLASS FUNCTION *********************************/
NATURE::NATURE(){}
/*******************************************************************************************/
NATURE::~NATURE(){destruct();}
/*******************************************************************************************/
void NATURE::generate(){
    at_e_i  = gaussian_range(50,12);
    at_s_n  = gaussian_range(50,12);
    fn_t_f  = gaussian_range(50,12);
    fn_j_p  = gaussian_range(50,12);
    mbti    = 0b00000000;
    if(at_e_i>50)mbti|=0b00001000;
    if(at_s_n>50)mbti|=0b00000100;
    if(fn_t_f>50)mbti|=0b00000010;
    if(fn_j_p>50)mbti|=0b00000001;
}
/*******************************************************************************************/
void NATURE::change(NATURE *gene){
    mbti    = gene->mbti;
    at_e_i  = gene->at_e_i;
    at_s_n  = gene->at_s_n;
    fn_t_f  = gene->fn_t_f;
    fn_j_p  = gene->fn_j_p;
}
/*******************************************************************************************/
void NATURE::meiosis(NATURE *mother, NATURE *father){
    at_e_i  = mutation_u8(mother->at_e_i, father->at_e_i);
    at_s_n  = mutation_u8(mother->at_s_n, father->at_s_n);
    fn_t_f  = mutation_u8(mother->fn_t_f, father->fn_t_f);
    fn_j_p  = mutation_u8(mother->fn_j_p, father->fn_j_p);
    mbti    = 0b00000000;
    if(at_e_i>50)mbti|=0b00001000;
    if(at_s_n>50)mbti|=0b00000100;
    if(fn_t_f>50)mbti|=0b00000010;
    if(fn_j_p>50)mbti|=0b00000001;
}
/*******************************************************************************************/
void NATURE::blend(NATURE *mother, NATURE *father){
    at_e_i  = heredity_u8(mother->at_e_i, father->at_e_i); 
    at_s_n  = heredity_u8(mother->at_s_n, father->at_s_n); 
    fn_t_f  = heredity_u8(mother->fn_t_f, father->fn_t_f); 
    fn_j_p  = heredity_u8(mother->fn_j_p, father->fn_j_p);    
    mbti    = 0b00000000;
    if(at_e_i>50)mbti|=0b00001000;
    if(at_s_n>50)mbti|=0b00000100;
    if(fn_t_f>50)mbti|=0b00000010;
    if(fn_j_p>50)mbti|=0b00000001;
}
/*******************************************************************************************/
void NATURE::status(){
    perforation("nature");
    spacebar(false,"MBTI");
    if(at_e_i>50)Serial.print("E");
    else Serial.print("I");
    if(at_s_n>50)Serial.print("S");
    else Serial.print("N");
    if(fn_t_f>50)Serial.print("T");
    else Serial.print("F");
    if(fn_j_p>50)Serial.print("J");
    else Serial.print("P");
    Serial.print(" [");
    Serial.print(at_e_i);
    Serial.print(",");
    Serial.print(at_s_n);
    Serial.print(",");
    Serial.print(fn_t_f);
    Serial.print(",");
    Serial.print(fn_j_p);
    Serial.print("] ");
    Serial.println(mbti);    
}
/*******************************************************************************************/
uint8_t NATURE::get_MBTI(){return mbti;}
/*********************************** NATURE CLASS FUNCTION *********************************/
/*********************************** EROS CLASS FUNCTION ***********************************/
EROS::EROS(){}
/*******************************************************************************************/
EROS::~EROS(){destruct();}
/*******************************************************************************************/
void EROS::generate(){
    lust       = gaussian_range(50,12);
    sadism     = gaussian_range(50,12);
    masohism   = gaussian_range(50,12);
    exhibition = gaussian_range(50,12);
    service    = gaussian_range(50,12);
}
/*******************************************************************************************/
void EROS::change(EROS *gene){
    lust       = gene->lust;
    sadism     = gene->sadism;
    masohism   = gene->masohism;
    exhibition = gene->exhibition;
    service    = gene->service;
}
/*******************************************************************************************/
void EROS::meiosis(EROS *mother, EROS *father){
    lust       = mutation_u8(mother->lust, father->lust); 
    sadism     = mutation_u8(mother->sadism, father->sadism); 
    masohism   = mutation_u8(mother->masohism, father->masohism); 
    exhibition = mutation_u8(mother->exhibition, father->exhibition);
    service    = mutation_u8(mother->service, father->service);
}
/*******************************************************************************************/
void EROS::blend(EROS *mother, EROS *father){
    lust       = heredity_u8(mother->lust, father->lust); 
    sadism     = heredity_u8(mother->sadism, father->sadism); 
    masohism   = heredity_u8(mother->masohism, father->masohism); 
    exhibition = heredity_u8(mother->exhibition, father->exhibition);
    service    = heredity_u8(mother->service, father->service);
}
/*******************************************************************************************/
void EROS::status(){
    perforation("eros");
    spacebar(false,"lust");       Serial.println(lust);
    spacebar(false,"sadism");     Serial.println(sadism);
    spacebar(false,"masohism");   Serial.println(masohism);
    spacebar(false,"exhibition"); Serial.println(exhibition);
    spacebar(false,"service");    Serial.println(service);
}
/*******************************************************************************************/
uint8_t EROS::get_lust(){return lust;}
/*******************************************************************************************/
uint8_t EROS::get_sadism(){return sadism;}
/*******************************************************************************************/
uint8_t EROS::get_masohism(){return masohism;}
/*******************************************************************************************/
uint8_t EROS::get_exhibition(){return exhibition;}
/*******************************************************************************************/
uint8_t EROS::get_service(){return service;}
/*********************************** EROS CLASS FUNCTION ***********************************/
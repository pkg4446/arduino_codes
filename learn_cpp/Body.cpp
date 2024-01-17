#include "heredity.h"
#include "utility.h"
#include "Body.h"

#define RAND_NUM   100
#define RAND_FLAGE 2

/*********************************** HEAD CLASS FUNCTION ***********************************/
HEAD::HEAD(bool gender) { //생성자
    gen_xy     = gender;
    name       = "";
    hair_color = random(RAND_NUM);
    eye_color  = random(RAND_NUM);
    hair_curl  = random(RAND_NUM);
    eyelid     = random(RAND_FLAGE);
    dimple     = random(RAND_FLAGE);
    bald       = random(RAND_FLAGE);
}
/*******************************************************************************************/
HEAD::~HEAD(){destruct();}
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
    perforation("head");
    spacebar("hair color"); Serial.print(hair_color);Serial.print(" : ");Serial.println(color_hair);
    spacebar("eye color");  Serial.print(color_eye);Serial.print(" : ");Serial.println(eye_color);
    spacebar("hair curl");  Serial.println(hair_curl);
    spacebar("eyelid");     Serial.println(eyelid);
    spacebar("dimple");     Serial.println(dimple);
    spacebar("bald");       Serial.println(bald);
}
/*********************************** HEAD CLASS FUNCTION ***********************************/
/*********************************** BODY CLASS FUNCTION ***********************************/
BODY::BODY(bool gender) { //생성자
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
    if(chest < waist)               swap(&chest,&waist);
    if(!gen_xy && breast < chest)   swap(&chest,&breast);
}
/*******************************************************************************************/
BODY::~BODY(){destruct();}
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


    perforation("body");
    spacebar("blood type"); Serial.println(blood_type);
    spacebar("skin color"); Serial.print(body_color);Serial.print(" : ");Serial.println(color_body);
    spacebar("height");     Serial.println(height);
    if(gen_xy){
        spacebar("hidden");Serial.println(breast);
    }else{
        uint16_t cupsize = breast - chest;
        String   cup     = "Z";
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
        spacebar("breast");Serial.println(breast);
        spacebar("breast");Serial.println(cup);
    }
    spacebar("chest");      Serial.println(chest);
    spacebar("waist");      Serial.println(waist);
    spacebar("hip");        Serial.println(hip);
    spacebar("leg ratio");  Serial.println(leg_ratio);
}

uint16_t BODY::weight(){
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
        spacebar("cup weight");Serial.println(uint16_t(cup_w));
    }
    float BMI = ans/((height_c/100)*(height_c/100));
    spacebar("weight"); Serial.println(uint16_t(ans));
    spacebar("BMI");    Serial.println(BMI);
    return ans;
}
/*********************************** BODY CLASS FUNCTION ***********************************/
/*********************************** EROGENOUS CLASS FUNCTION ******************************/
EROGENOUS::EROGENOUS(bool gender) { //생성자
    gen_xy          = gender;
    
    hood_c          = random(RAND_NUM);
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
EROGENOUS::~EROGENOUS(){destruct();}
void EROGENOUS::change(EROGENOUS *gene){
    gen_xy          = gene->gen_xy;
    hood_c          = gene->hood_c;
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
    hood_c          = mutation_u8(mother->hood_c,           father->hood_c);
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
    hood_c          = heredity_u8(mother->hood_c,           father->hood_c);
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
        spacebar("glans_d");    Serial.println(glans_d);
        spacebar("glans_d_e");  Serial.println(glans_d_e);
        spacebar("glans_l");    Serial.println(glans_l);
        spacebar("glans_l_e");  Serial.println(glans_l_e);
        
        spacebar("body_d");     Serial.println(body_d);
        spacebar("body_d_e");   Serial.println(body_d_e);
        spacebar("body_l");     Serial.println(body_l);
        spacebar("body_l_e");   Serial.println(body_l_e);

        spacebar("ball_r");     Serial.println(ball_r);
        spacebar("ball_l");     Serial.println(ball_l);

        spacebar("prepuce");    Serial.println(prepuce);
    }else{
        spacebar("hood_c");     Serial.println(hood_c);
        spacebar("hood_start"); Serial.println(hood_start);
        spacebar("hood_texture");Serial.println(hood_texture);
        spacebar("hood_width"); Serial.println(hood_width);
        spacebar("hood_length");Serial.println(hood_length);

        spacebar("clit_d");     Serial.println(clit_d);
        spacebar("clit_d_e");   Serial.println(clit_d_e);
        spacebar("clit_l");     Serial.println(clit_l);
        spacebar("clit_l_e");   Serial.println(clit_l_e);

        spacebar("wing_shape"); Serial.println(lip_i_shape);
        spacebar("wing_texture");Serial.println(lip_i_texture);
        spacebar("wing_width"); Serial.println(lip_i_width);
        spacebar("wing_length");Serial.println(lip_i_length);
        spacebar("wing_r");     Serial.println(lip_i_length_r);
        spacebar("wing_l");     Serial.println(lip_i_length_l);
        spacebar("lip_shape");  Serial.println(lip_o_shape);
        spacebar("lip_texture");Serial.println(lip_o_texture);
    }
    spacebar("invert");     Serial.println(invert);
    spacebar("perineum");   Serial.println(perineum);
    spacebar("wrinkle");    Serial.println(wrinkle);
    
    spacebar("areola");     Serial.println(areola);
    spacebar("nipple_d");   Serial.println(nipple_d);
    spacebar("nipple_d_e"); Serial.println(nipple_d_e);
    spacebar("nipple_h");   Serial.println(nipple_h);
    spacebar("nipple_h_e"); Serial.println(nipple_h_e);
}

void EROGENOUS::inspection(){
    perforation("parts inspection");
    if(gen_xy){
        Serial.print("prepuce   : ");Serial.println(prepuce);
    }else{
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
        if(hood_start < 17){hood+="살짝 접힌 모양의 덮개가";}
        else if(hood_start < 55){hood+="완전히 접힌 모양의 덮개가";}
        else if(hood_start < 60){hood+="매끄러운 덮개가 부드럽게";}
        else if(hood_start < 78){hood+="두겹으로 주름진 덮개가 살짝";}
        else if(hood_start < 98){hood+="두겹으로 주름진 덮개가 완전히";}
        else{hood+="두겹으로 부드럽게 주름진 덮개가";}
        hood+=" 클리토리스를 덮고 있다.";
        Serial.println(hood);

        String lip = "";
        if(lip_o_texture < 7){lip+="반들거리고";}
        else if(lip_o_texture < 26){lip+="매끈하고";}
        else if(lip_o_texture < 75){lip+="보드랍고";}
        else{lip+="부드럽고";}
        lip += " ";
        if(lip_o_shape < 8){lip+="엷은";}
        else if(lip_o_shape < 68){lip+="도톰한";}
        else{lip+="두툼한";}
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
        else{lip+="걸래같은";}
        lip += " 소음순이 질구 아래에서 ";
        if(fork < 8){lip+="매끄럽게";}
        else if(fork < 68){lip+="둥글게 주름져";}
        else{lip+="잔뜩 주름져";}
        lip += " 만난다";
        Serial.println(lip);
    }
    String perineum_txt = "회음부는 ";
    if(perineum < 66){perineum_txt+="매끄럽다.";}
    else if(perineum < 83){perineum_txt+="회음봉선의 흔적이 보인다.";}
    else if(perineum < 93){perineum_txt+="회음봉선이 항문까지 이어진다.";}
    else{perineum_txt+="도드라지는 회음봉선이 항문까지 이어진다.";}
    Serial.println(perineum);
}
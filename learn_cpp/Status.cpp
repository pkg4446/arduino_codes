#include "heredity.h"
#include "utility.h"
#include "status.h"

#define RAND_NUM   100
#define RAND_FLAGE 2

/*********************************** STAT CLASS FUNCTION ***********************************/
STAT::STAT() { //생성자
    intelligence = gaussian_range(50,12);
    strength     = gaussian_range(50,12);
    dexterity    = gaussian_range(50,12);
    charisma     = gaussian_range(50,12);
    constitution = gaussian_range(50,12);
}
/*******************************************************************************************/
STAT::~STAT(){destruct();}
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
    perforation();
    spacebar("intelligence");   Serial.println(intelligence);
    spacebar("strength");       Serial.println(strength);
    spacebar("dexterity");      Serial.println(dexterity);
    spacebar("charisma");       Serial.println(charisma);
    spacebar("constitution");   Serial.println(constitution);
}
/*********************************** STAT CLASS FUNCTION ***********************************/
/*********************************** HOLE CLASS FUNCTION ***********************************/
HOLE::HOLE() { //생성자
    gape_u = gaussian_range(650,40);
    gape_v = gaussian_range(5550,400);
    gape_a = gaussian_range(6950,800);
    pressure_u = gaussian_range(2500,300);
    pressure_v = gaussian_range(6950,800);
    pressure_a = gaussian_range(9220,1300);
}
/*******************************************************************************************/
HOLE::~HOLE(){destruct();}
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
    perforation();
    if(!gender){
        spacebar("gape_v");     Serial.println(gape_v);
        spacebar("pressure_v"); Serial.println(pressure_v);
    }
    spacebar("gape_u");     Serial.println(gape_u);
    spacebar("pressure_u"); Serial.println(pressure_u);
    spacebar("gape_a");     Serial.println(gape_a);
    spacebar("pressure_a"); Serial.println(pressure_a);
}
/*********************************** HOLE CLASS FUNCTION ***********************************/
/*********************************** SENSE CLASS FUNCTION **********************************/
SENSE::SENSE() { //생성자
    cervix          = gaussian_range(50,12);
    skin            = gaussian_range(50,12);
    vagina_balls    = gaussian_range(50,12);
    urethra         = gaussian_range(50,12);
    anal            = gaussian_range(50,12);
    nipple          = gaussian_range(50,12);
}
/*******************************************************************************************/
SENSE::~SENSE(){destruct();}
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
    perforation();
    if(gender){
        spacebar("balls");  Serial.println(vagina_balls);
    }else{
        spacebar("cervix"); Serial.println(cervix);
        spacebar("vagina"); Serial.println(vagina_balls);
    }
    spacebar("skin");   Serial.println(skin);
    spacebar("urethra");Serial.println(urethra);
    spacebar("anal");   Serial.println(anal);
    spacebar("nipple"); Serial.println(nipple);
}
/*********************************** SENSE CLASS FUNCTION **********************************/
/*********************************** NATURE CLASS FUNCTION *********************************/
NATURE::NATURE() { //생성자 
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
NATURE::~NATURE(){destruct();}
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
    perforation();
    spacebar("MBTI");
    if(at_e_i>50)Serial.print("E");
    else Serial.print("I");
    if(at_s_n>50)Serial.print("S");
    else Serial.print("N");
    if(fn_t_f>50)Serial.print("T");
    else Serial.print("F");
    if(fn_j_p>50)Serial.print("J");
    else Serial.print("P");
    Serial.print("[");
    Serial.print(at_e_i);
    Serial.print(",");
    Serial.print(at_s_n);
    Serial.print(",");
    Serial.print(fn_t_f);
    Serial.print(",");
    Serial.print(fn_j_p);
    Serial.print("]");
    Serial.println(mbti);    
}
/*********************************** NATURE CLASS FUNCTION *********************************/
/*********************************** EROS CLASS FUNCTION ***********************************/
EROS::EROS() { //생성자
    lust       = gaussian_range(50,12);
    sadism     = gaussian_range(50,12);
    masohism   = gaussian_range(50,12);
    exhibition = gaussian_range(50,12);
    service    = gaussian_range(50,12);
}
/*******************************************************************************************/
EROS::~EROS(){destruct();}
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
    perforation();
    spacebar("lust");       Serial.println(lust);
    spacebar("sadism");     Serial.println(sadism);
    spacebar("masohism");   Serial.println(masohism);
    spacebar("exhibition"); Serial.println(exhibition);
    spacebar("service");    Serial.println(service);
}
/*********************************** EROS CLASS FUNCTION ***********************************/
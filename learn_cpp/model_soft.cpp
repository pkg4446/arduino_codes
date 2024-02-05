#include "model_soft.h"

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
/*******************************************************************************************/
String  STAT::get_csv(){
    String response = String(intelligence);
    make_csv(&response, String(strength));
    make_csv(&response, String(dexterity));
    make_csv(&response, String(charisma));
    make_csv(&response, String(constitution));
    return response;
}
/*******************************************************************************************/
void  STAT::set_csv(char* save_file){
    intelligence    = atoi(strtok(save_file, ","));
    strength        = atoi(strtok(0x00, ","));
    dexterity       = atoi(strtok(0x00, ","));
    charisma        = atoi(strtok(0x00, ","));
    constitution    = atoi(strtok(0x00, ","));
}
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
    return 0;
}
/*******************************************************************************************/
uint16_t HOLE::get_pressure(uint8_t item){
    if(item == 1) return pressure_u;
    else if(item == 2) return pressure_v;
    else if(item == 3) return pressure_a;
    return 0;
}
/*******************************************************************************************/
String  HOLE::get_csv(){
    String response = String(gape_u);
    make_csv(&response, String(gape_v));
    make_csv(&response, String(gape_a));
    make_csv(&response, String(pressure_u));
    make_csv(&response, String(pressure_v));
    make_csv(&response, String(pressure_a));
    return response;
}
/*******************************************************************************************/
void  HOLE::set_csv(char* save_file){
    gape_u  = atoi(strtok(save_file, ","));
    gape_v  = atoi(strtok(0x00, ","));
    gape_a  = atoi(strtok(0x00, ","));
    pressure_u  = atoi(strtok(0x00, ","));
    pressure_v  = atoi(strtok(0x00, ","));
    pressure_a  = atoi(strtok(0x00, ","));
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
/*******************************************************************************************/
String  SENSE::get_csv(){
    String response = String(cervix);
    make_csv(&response, String(skin));
    make_csv(&response, String(clit_glans));
    make_csv(&response, String(vagina_balls));
    make_csv(&response, String(urethra));
    make_csv(&response, String(anal));
    make_csv(&response, String(nipple));
    return response;
}
/*******************************************************************************************/
void  SENSE::set_csv(char* save_file){
    cervix  = atoi(strtok(save_file, ","));
    skin    = atoi(strtok(0x00, ","));
    clit_glans      = atoi(strtok(0x00, ","));
    vagina_balls    = atoi(strtok(0x00, ","));
    urethra = atoi(strtok(0x00, ","));
    anal    = atoi(strtok(0x00, ","));
    nipple  = atoi(strtok(0x00, ","));
}
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
/*******************************************************************************************/
String  NATURE::get_csv(){
    String response = String(mbti);
    make_csv(&response, String(at_e_i));
    make_csv(&response, String(at_s_n));
    make_csv(&response, String(fn_t_f));
    make_csv(&response, String(fn_j_p));
    return response;
}
/*******************************************************************************************/
void  NATURE::set_csv(char* save_file){
    mbti    = atoi(strtok(save_file, ","));
    at_e_i  = atoi(strtok(0x00, ","));
    at_s_n  = atoi(strtok(0x00, ","));
    fn_t_f  = atoi(strtok(0x00, ","));
    fn_j_p  = atoi(strtok(0x00, ","));
}
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
/*******************************************************************************************/
String  EROS::get_csv(){
    String response = String(lust);
    make_csv(&response, String(sadism));
    make_csv(&response, String(masohism));
    make_csv(&response, String(exhibition));
    make_csv(&response, String(service));
    return response;
}
/*******************************************************************************************/
void  EROS::set_csv(char* save_file){
    lust        = atoi(strtok(save_file, ","));
    sadism      = atoi(strtok(0x00, ","));
    masohism    = atoi(strtok(0x00, ","));
    exhibition  = atoi(strtok(0x00, ","));
    service     = atoi(strtok(0x00, ","));
}
/*********************************** EROS CLASS FUNCTION ***********************************/
/*********************************** EXP CLASS FUNCTION ***********************************/
EXP::EXP() {}
/*******************************************************************************************/
EXP::~EXP(){destruct();}
/*******************************************************************************************/
void EXP::generate(){
    orgasm  = 0;
    squirt  = 0;
    mouth   = 0;
    vagina  = 0;
    anal    = 0;
    urethra = 0;
    expans_v= 0;
    expans_a= 0;
    sperm_m = 0;
    sperm_v = 0;
    sperm_a = 0;
}
/*******************************************************************************************/
void EXP::update(uint8_t item){
    if(item == 1)      orgasm++;
    else if(item == 2) squirt++;
    else if(item == 3) mouth++;
    else if(item == 4) vagina++;
    else if(item == 5) anal++;
    else if(item == 6) urethra++;
}
/*******************************************************************************************/
void EXP::update_expansion(bool item){
    if(item) expans_v++;
    else     expans_a++;
}
/*******************************************************************************************/
void EXP::update_shot(uint8_t item){
    if(item == 1)      sperm_m++;
    else if(item == 2) sperm_v++;
    else if(item == 3) sperm_a++;
}
/*******************************************************************************************/
void EXP::status(bool gender){
    perforation("EXP");
    spacebar(false,"orgasm");     Serial.println(orgasm);
    spacebar(false,"squirt");     Serial.println(squirt);
    spacebar(false,"mouth");      Serial.println(mouth);
    if(!gender){
        spacebar(false,"vagina");     Serial.println(vagina);
        spacebar(false,"expans_v");   Serial.println(expans_v);
        spacebar(false,"sperm_v");    Serial.println(sperm_v);
    }
    spacebar(false,"anal");       Serial.println(anal);
    spacebar(false,"urethra");    Serial.println(urethra);
    spacebar(false,"expans_a");   Serial.println(expans_a);
    spacebar(false,"sperm_m");    Serial.println(sperm_m);
    spacebar(false,"sperm_a");    Serial.println(sperm_a);
}
/*******************************************************************************************/
uint8_t EXP::get(uint8_t item){
    if(item == 1)      return orgasm;
    else if(item == 2) return squirt;
    else if(item == 3) return mouth;
    else if(item == 4) return vagina;
    else if(item == 5) return anal;
    else if(item == 6) return urethra;
    return 0;
}
/*******************************************************************************************/
uint8_t EXP::get_expansion(bool item){
    if(item) return expans_v;
    return expans_a;
}
/*******************************************************************************************/
uint8_t EXP::get_shot(uint8_t item){
    if(item == 1)      return sperm_m;
    else if(item == 2) return sperm_v;
    else if(item == 3) return sperm_a;
    return 0;
}
/*******************************************************************************************/
String  EXP::get_csv(){
    String response = String(orgasm);
    make_csv(&response, String(squirt));
    make_csv(&response, String(mouth));
    make_csv(&response, String(vagina));
    make_csv(&response, String(anal));
    make_csv(&response, String(urethra));
    make_csv(&response, String(expans_v));
    make_csv(&response, String(expans_a));
    make_csv(&response, String(sperm_m));
    make_csv(&response, String(sperm_v));
    make_csv(&response, String(sperm_a));
    return response;
}
/*******************************************************************************************/
void    EXP::set_csv(char* save_file){
    orgasm   = atoi(strtok(save_file, ","));
    squirt   = atoi(strtok(0x00, ","));
    mouth    = atoi(strtok(0x00, ","));
    vagina   = atoi(strtok(0x00, ","));
    anal     = atoi(strtok(0x00, ","));
    urethra  = atoi(strtok(0x00, ","));
    expans_v = atoi(strtok(0x00, ","));
    expans_a = atoi(strtok(0x00, ","));
    sperm_m  = atoi(strtok(0x00, ","));
    sperm_v  = atoi(strtok(0x00, ","));
    sperm_a  = atoi(strtok(0x00, ","));
}
/*********************************** EXP CLASS FUNCTION ***********************************/
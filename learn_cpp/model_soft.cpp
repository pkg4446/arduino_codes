#include "heredity.h"
#include "utility.h"
#include "model_soft.h"

/*********************************** MENS CLASS FUNCTION ***********************************/
MENS::MENS() {}
/*******************************************************************************************/
MENS::~MENS(){destruct();}
/*******************************************************************************************/
void MENS::generate(){
    blood     = 0;
    cycle     = 0;
    ovulation = 0;
    pregnant  = 0;
    d_day     = 0;
}
/*******************************************************************************************/
void MENS::daily(){
}
/*******************************************************************************************/
void MENS::status(){
    perforation("mens");
    spacebar("blood");     Serial.println(blood);
    spacebar("cycle");     Serial.println(cycle);
    spacebar("ovulation"); Serial.println(ovulation);
    spacebar("pregnant");  Serial.println(pregnant);
    spacebar("d_day");     Serial.println(d_day);
}
/*******************************************************************************************/
uint8_t MENS::get(){
    //normal_safe == 0
    //warning     == 1
    //blooding    == 2
    return 1;
}
/*********************************** MENS CLASS FUNCTION ***********************************/
/*********************************** CURRENT CLASS FUNCTION ***********************************/
CURRENT::CURRENT() {}
/*******************************************************************************************/
CURRENT::~CURRENT(){destruct();}
/*******************************************************************************************/
void CURRENT::generate(){
    furr    = 0;
    stamina = 0;
    mental  = 0;
    stress  = 0;
    horny   = 0;
    fain    = 0;
    ecstasy = 0;
    lubric  = 0;
    pee     = 0;
    poo     = 0;
}
/*******************************************************************************************/
void CURRENT::grown(){
    furr += 20;
}
/*******************************************************************************************/
void CURRENT::update(uint8_t item, int8_t count){
    if(item == 1) stamina += count;
    else if(item == 2) mental  += count;
    else if(item == 3) stress  += count;
    else if(item == 4) horny   += count;
    else if(item == 5) fain    += count;
    else if(item == 6) ecstasy += count;
    else if(item == 7) lubric  += count;
    else if(item == 8) pee     += count;
    else if(item == 9) poo     += count;
}
/*******************************************************************************************/
void CURRENT::status(){
    perforation("currnet");
    spacebar("furr");   Serial.println(furr);
    spacebar("stamina");Serial.println(stamina);
    spacebar("mental"); Serial.println(mental);
    spacebar("stress"); Serial.println(stress);
    spacebar("horny");  Serial.println(horny);
    spacebar("fain");   Serial.println(fain);
    spacebar("ecstasy");Serial.println(ecstasy);
    spacebar("lubric"); Serial.println(lubric);
    spacebar("pee");    Serial.println(pee);
    spacebar("poo");    Serial.println(poo);
}
/*******************************************************************************************/
uint16_t CURRENT::get_furr(){
    return  furr;
}
/*******************************************************************************************/
uint8_t CURRENT::get(uint8_t item){
    if(item == 1) stamina;
    else if(item == 2) return mental;
    else if(item == 3) return stress;
    else if(item == 4) return horny;
    else if(item == 5) return fain;
    else if(item == 6) return ecstasy;
    else if(item == 7) return lubric;
    else if(item == 8) return pee;
    else if(item == 9) return poo;
}
/*********************************** CURRENT CLASS FUNCTION ***********************************/
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
    spacebar("orgasm");     Serial.println(orgasm);
    spacebar("squirt");     Serial.println(squirt);
    spacebar("mouth");      Serial.println(mouth);
    if(!gender){
        spacebar("vagina");     Serial.println(vagina);
        spacebar("expans_v");   Serial.println(expans_v);
        spacebar("sperm_v");    Serial.println(sperm_v);
    }
    spacebar("anal");       Serial.println(anal);
    spacebar("urethra");    Serial.println(urethra);
    spacebar("expans_a");   Serial.println(expans_a);
    spacebar("sperm_m");    Serial.println(sperm_m);
    spacebar("sperm_a");    Serial.println(sperm_a);
}
/*******************************************************************************************/
uint8_t EXP::get(uint8_t item){
    if(item == 1)      return orgasm;
    else if(item == 2) return squirt;
    else if(item == 3) return mouth;
    else if(item == 4) return vagina;
    else if(item == 5) return anal;
    else if(item == 6) return urethra;
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
}
/*********************************** EXP CLASS FUNCTION ***********************************/
/*********************************** BREED CLASS FUNCTION ***********************************/
BREED::BREED() {}
/*******************************************************************************************/
BREED::~BREED(){destruct();}
/*******************************************************************************************/
void BREED::generate(){
    pregnancy   = 0;
    birth       = 0;
    miscarriage = 0;
}
/*******************************************************************************************/
void BREED::update(){
}
/*******************************************************************************************/
void BREED::status(){
    perforation("mens");
    spacebar("pregnancy");   Serial.println(pregnancy);
    spacebar("birth");       Serial.println(birth);
    spacebar("miscarriage"); Serial.println(miscarriage);
}
/*******************************************************************************************/
uint8_t BREED::get(uint8_t item){
    if(item == 1)      return pregnancy;
    else if(item == 2) return birth;
    else if(item == 3) return miscarriage;
}
/*********************************** BREED CLASS FUNCTION ***********************************/
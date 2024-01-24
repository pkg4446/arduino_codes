#include "model_soft.h"

/*********************************** MENS CLASS FUNCTION ***********************************/
MENS::MENS() {}
/*******************************************************************************************/
MENS::~MENS(){destruct();}
/*******************************************************************************************/
void MENS::generate(){
    periode   = 28;
    blood     = 0;
    cycle     = 0;
    ovulation = 0;
    pregnant  = 0;
    d_day     = 0;
}
/*******************************************************************************************/
void MENS::daily(bool gender){
    if(!gender){
        if(pregnant){
            if(d_day > 0) d_day--;
            else{
                pregnant = false;
                //baby out
            }
        }else{
            if(cycle == 0){
                if(periode<21)      periode = 20;
                else if(periode>35) periode = 36;
                cycle = gaussian_range(periode,2);
                blood = random(1,cycle/5);
                ovulation = gaussian_range(cycle/2,1);
                periode   = (2*periode + cycle)/3;
            }else{
                if(cycle > 0) cycle -= 1;
                if(blood > 0) blood -= 1;
                if(ovulation > 0) ovulation -= 1;
            }
        }
    }
}
/*******************************************************************************************/
void MENS::status(){
    perforation("mens");
    spacebar(false,"periode");   Serial.println(periode);
    spacebar(false,"blood");     Serial.println(blood);
    spacebar(false,"cycle");     Serial.println(cycle);
    spacebar(false,"ovulation"); Serial.println(ovulation);
    spacebar(false,"pregnant");  Serial.println(pregnant);
    spacebar(false,"d_day");     Serial.println(d_day);
}
/*******************************************************************************************/
uint8_t MENS::get(){
    //normal_safe == 0
    //egg drop    == 1
    //blooding    == 2
    if(blood>0) return 2;
    else if((ovulation != 0) && (ovulation < 3)) return 1;
    return 0;
}
/*******************************************************************************************/
bool MENS::get_pregnant(){
    return pregnant;
}
/*********************************** MENS CLASS FUNCTION ***********************************/
/*********************************** CURRENT CLASS FUNCTION ***********************************/
CURRENT::CURRENT() {}
/*******************************************************************************************/
CURRENT::~CURRENT(){destruct();}
/*******************************************************************************************/
void CURRENT::generate(){
    furr    = random(5000);
    lubric  = 0;
    pee     = 0;
    poo     = 0;
    stamina = 0;
    mental  = 0;
    stress  = 0;
    horny   = 0;
    fain    = 0;
    ecstasy = 0;
}
/*******************************************************************************************/
void CURRENT::daily(){
    if(pee < 5000) furr += random(10,30);
    if(pee < 100)  pee += random(5,50);
    if(poo < 100)  poo += random(1,10);
}
/*******************************************************************************************/
void CURRENT::update(uint8_t item, int8_t count){
    if(item == 1) stamina += count;
    else if(item == 2) lubric  += count;
    else if(item == 3) pee     += count;
    else if(item == 4) poo     += count;
    else if(item == 5) mental  += count;
    else if(item == 6) stress  += count;
    else if(item == 7) horny   += count;
    else if(item == 8) fain    += count;
    else if(item == 9) ecstasy += count;
}
/*******************************************************************************************/
void CURRENT::status(){
    perforation("currnet");
    spacebar(false,"furr");   Serial.println(furr);
    spacebar(false,"lubric"); Serial.println(lubric);
    spacebar(false,"pee");    Serial.println(pee);
    spacebar(false,"poo");    Serial.println(poo);
    spacebar(false,"stamina");Serial.println(stamina);
    spacebar(false,"mental"); Serial.println(mental);
    spacebar(false,"stress"); Serial.println(stress);
    spacebar(false,"horny");  Serial.println(horny);
    spacebar(false,"fain");   Serial.println(fain);
    spacebar(false,"ecstasy");Serial.println(ecstasy);
}
/*******************************************************************************************/
uint16_t CURRENT::get_furr(){
    return  furr;
}
/*******************************************************************************************/
uint8_t CURRENT::get(uint8_t item){
    if(item == 1)      return lubric;
    else if(item == 2) return pee;
    else if(item == 3) return poo;
    else if(item == 4) return stamina;
    else if(item == 5) return mental;
    else if(item == 6) return stress;
    else if(item == 7) return horny;
    else if(item == 8) return fain;
    else if(item == 9) return ecstasy;
    return 0;
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
    spacebar(false,"pregnancy");   Serial.println(pregnancy);
    spacebar(false,"birth");       Serial.println(birth);
    spacebar(false,"miscarriage"); Serial.println(miscarriage);
}
/*******************************************************************************************/
uint8_t BREED::get(uint8_t item){
    if(item == 1)      return pregnancy;
    else if(item == 2) return birth;
    else if(item == 3) return miscarriage;
    return 0;
}
/*********************************** BREED CLASS FUNCTION ***********************************/
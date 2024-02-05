#include "model_status.h"

/*********************************** MENS CLASS FUNCTION ***********************************/
MENS::MENS() {}
/*******************************************************************************************/
MENS::~MENS(){destruct();}
/*******************************************************************************************/
void MENS::generate(bool gender){
    gen_xy    = gender;
    pregnant  = false;
    periode   = 28;
    blood     = 0;
    cycle     = 0;
    ovulation = 0;
    d_day     = 0;
}
/*******************************************************************************************/
void MENS::daily(){
    if(!gen_xy){
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
    spacebar(false,"pregnant");  Serial.println(pregnant);
    spacebar(false,"periode");   Serial.println(periode);
    spacebar(false,"blood");     Serial.println(blood);
    spacebar(false,"cycle");     Serial.println(cycle);
    spacebar(false,"ovulation"); Serial.println(ovulation);
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
/*******************************************************************************************/
String  MENS::get_csv(){
    String response = String(gen_xy);
    make_csv(&response, String(pregnant));
    make_csv(&response, String(periode));
    make_csv(&response, String(blood));
    make_csv(&response, String(cycle));
    make_csv(&response, String(ovulation));
    make_csv(&response, String(d_day));
    return response;
}
/*******************************************************************************************/
void    MENS::set_csv(char* save_file){
    gen_xy      = String(strtok(save_file, ",")) == "0" ? false:true;
    pregnant    = String(strtok(0x00, ",")) == "0" ? false:true;
    periode     = atoi(strtok(0x00, ","));
    blood       = atoi(strtok(0x00, ","));
    cycle       = atoi(strtok(0x00, ","));
    ovulation   = atoi(strtok(0x00, ","));
    d_day       = atoi(strtok(0x00, ","));
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
/*******************************************************************************************/
String  CURRENT::get_csv(){
    String response = String(furr);
    make_csv(&response, String(lubric));
    make_csv(&response, String(pee));
    make_csv(&response, String(poo));
    make_csv(&response, String(stamina));
    make_csv(&response, String(mental));
    make_csv(&response, String(stress));
    make_csv(&response, String(horny));
    make_csv(&response, String(fain));
    make_csv(&response, String(ecstasy));
    return response;
}
/*******************************************************************************************/
void    CURRENT::set_csv(char* save_file){
    furr    = atoi(strtok(save_file, ","));
    lubric  = atoi(strtok(0x00, ","));
    pee     = atoi(strtok(0x00, ","));
    poo     = atoi(strtok(0x00, ","));
    stamina = atoi(strtok(0x00, ","));
    mental  = atoi(strtok(0x00, ","));
    stress  = atoi(strtok(0x00, ","));
    horny   = atoi(strtok(0x00, ","));
    fain    = atoi(strtok(0x00, ","));
    ecstasy = atoi(strtok(0x00, ","));
}
/*********************************** CURRENT CLASS FUNCTION ***********************************/
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
/*******************************************************************************************/
String  BREED::get_csv(){
    String response = String(pregnancy);
    make_csv(&response, String(birth));
    make_csv(&response, String(miscarriage));
    return response;
}
/*******************************************************************************************/
void    BREED::set_csv(char* save_file){
    pregnancy   = atoi(strtok(save_file, ","));
    birth       = atoi(strtok(0x00, ","));
    miscarriage = atoi(strtok(0x00, ","));
}
/*********************************** BREED CLASS FUNCTION ***********************************/
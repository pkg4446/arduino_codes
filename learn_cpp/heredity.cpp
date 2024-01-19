#include "heredity.h"

#define RAND_FLAGE 2

uint16_t change_gender(uint16_t ex_average, uint16_t average, uint16_t standard_deviation, uint16_t value, uint16_t ratio){
    uint16_t response = 0;
    if(value > ex_average){response = average + (ratio*standard_deviation/100);}
    else{response = average - (ratio*standard_deviation/100);}
    return response;
}
/*******************************************************************************************/
String mutation_string(String mother, String father){
    bool flage = random(RAND_FLAGE);
    if(flage){
        return father;
    }else{
        return mother;
    }
}

bool mutation_bool(bool mother, bool father){
    bool flage = random(RAND_FLAGE);
    if(flage){
        return father;
    }else{
        return mother;
    }
}

uint8_t mutation_u8(uint8_t mother, uint8_t father){
    bool flage = random(RAND_FLAGE);
    if(flage){
        return father;
    }else{
        return mother;
    }
}

uint16_t mutation_u16(uint16_t mother, uint16_t father){
    bool flage = random(RAND_FLAGE);
    if(flage){
        return father;
    }else{
        return mother;
    }
}

bool heredity_bool(bool mother, bool father, bool dominant){
    if(dominant){
        return (mother || father);
    }else{
        return (mother && father);
    }
}

uint8_t heredity_u8(uint8_t mother, uint8_t father){
    uint8_t range_small = mother;
    uint8_t range_large = father;
    if(mother > father){
        range_small = father;
        range_large = mother;
    }
    uint8_t response = random(range_small,range_large);
    return response;
}

uint16_t heredity_u16(uint16_t mother, uint16_t father){
    uint16_t range_small = mother;
    uint16_t range_large = father;
    if(mother > father){
        range_small = father;
        range_large = mother;
    }
    uint16_t response = random(range_small,range_large);
    return response;
}
#include "Body.h"

#define RAND_FLAGE 2

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

uint16_t heredity_ratio(uint16_t mother, uint16_t father, bool gender, uint16_t ratio_moter, uint16_t ratio_fater){
    uint16_t range_small = mother;
    uint16_t range_large = father;
    if(gender){
        uint32_t temp_moter = uint32_t(mother)*uint32_t(ratio_fater)/100;
        range_small = temp_moter;
    }else{
        uint32_t temp_father = uint32_t(father)*uint32_t(ratio_moter)/100;
        range_large = temp_father;
    }
    if(range_small > range_large){
        uint16_t temp_change = range_small;
        range_small = range_large;
        range_large = temp_change;
    }
    uint16_t response = random(range_small,range_large);
    return response;
}
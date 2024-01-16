#include "heredity.h"

#define RAND_FLAGE 2

//distribution table
//0.13 ,0.26 ,0.39 ,0.52 ,0.67 ,0.84 ,1.04 ,1.28 ,1.65
uint16_t gaussian_range(uint16_t average, uint16_t standard_deviation){
    uint16_t response = average;
    uint8_t stage     = random(50);
    bool flage        = random(RAND_FLAGE);
    uint16_t range    = 0;

    if(stage<5){range = standard_deviation*13/100;}
    else if(stage<10){range = standard_deviation*26/100;}
    else if(stage<15){range = standard_deviation*39/100;}
    else if(stage<20){range = standard_deviation*52/100;}
    else if(stage<25){range = standard_deviation*67/100;}
    else if(stage<30){range = standard_deviation*84/100;}
    else if(stage<35){range = standard_deviation*104/100;}
    else if(stage<40){range = standard_deviation*128/100;}
    else if(stage<45){range = standard_deviation*126/100;}
    else{range = standard_deviation*4;}

    if(flage){response += range;}
    else{response -= range;}

    return response;
}

uint16_t get_gaussian(uint16_t average, uint16_t standard_deviation, uint16_t value){
    uint16_t gap = 0;
    if(average>value){
        gap = average - value;
    }else{
        gap = value - average;
    }
    uint32_t response = uint32_t(gap) * 100 / uint32_t(standard_deviation);
    return uint16_t(response);
}

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
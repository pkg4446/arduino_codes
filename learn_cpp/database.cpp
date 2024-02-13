#include "database.h"
/*************** NAME ***************/
String new_family(void){
    String response = "";
    uint8_t index_family  = random(strlen_P(family)/3)*3;
    for(uint16_t index=index_family; index<index_family+3; index++){
        response += char(pgm_read_byte_near(family+index));
    }
    return response;
};
String new_name(bool gender){
    uint16_t index_female = random(strlen_P(names_female)/6)*6;
    uint16_t index_male   = random(strlen_P(names_male)/6)*6;
    String response = "";
    if(gender){
        for(uint16_t index=index_male; index<index_male+6; index++){
            response += char(pgm_read_byte_near(names_male+index));
        }
    }else{
        for(uint16_t index=index_female; index<index_female+6; index++){
            response += char(pgm_read_byte_near(names_female+index));
        }
    }
    return response;
};
/*************** NAME ***************/
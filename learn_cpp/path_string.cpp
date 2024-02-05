#include "path_string.h"

/*******************************************************/
String path_avatar(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(path_avatar_str); index++){
        response += char(pgm_read_byte_near(path_avatar_str+index));
    }
    return response;
};
/*******************************************************/
String file_mother(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(file_mother_str); index++){
        response += char(pgm_read_byte_near(file_mother_str+index));
    }
    return response;
};
/*******************************************************/
String file_father(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(file_father_str); index++){
        response += char(pgm_read_byte_near(file_father_str+index));
    }
    return response;
};
/*******************************************************/
String file_hard(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(file_hard_str); index++){
        response += char(pgm_read_byte_near(file_hard_str+index));
    }
    return response;
};
/*******************************************************/
String file_soft(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(file_soft_str); index++){
        response += char(pgm_read_byte_near(file_soft_str+index));
    }
    return response;
};
/*******************************************************/
String file_mens(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(file_mens_str); index++){
        response += char(pgm_read_byte_near(file_mens_str+index));
    }
    return response;
};
/*******************************************************/
String file_current(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(file_current_str); index++){
        response += char(pgm_read_byte_near(file_current_str+index));
    }
    return response;
};
/*******************************************************/
String file_exp(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(file_exp_str); index++){
        response += char(pgm_read_byte_near(file_exp_str+index));
    }
    return response;
};
/*******************************************************/
String file_hash(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(file_hash_str); index++){
        response += char(pgm_read_byte_near(file_hash_str+index));
    }
    return response;
};
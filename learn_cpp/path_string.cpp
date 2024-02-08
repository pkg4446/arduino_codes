#include "path_string.h"
/*******************************************************/
String path_root(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(path_root_str); index++){
        response += char(pgm_read_byte_near(path_root_str+index));
    }
    return response;
};
/*******************************************************/
String path_config(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(path_config_str); index++){
        response += char(pgm_read_byte_near(path_config_str+index));
    }
    return path_root() + response;
};
/*******************************************************/
String path_cryobank(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(path_cryobank_str); index++){
        response += char(pgm_read_byte_near(path_cryobank_str+index));
    }
    return path_root() + response;
};
/*******************************************************/
String path_avatar(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(path_avatar_str); index++){
        response += char(pgm_read_byte_near(path_avatar_str+index));
    }
    return path_root() + response;
};
/*******************************************************/
String path_assist(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(path_assist_str); index++){
        response += char(pgm_read_byte_near(path_assist_str+index));
    }
    return path_root() + response;
};
/*******************************************************/
String path_womb(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(path_womb_str); index++){
        response += char(pgm_read_byte_near(path_womb_str+index));
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
#include "path_string.h"

/*******************************************************/
String path_avatar(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(path_avatar_str); index++){
        response += char(pgm_read_byte_near(path_avatar_str+index));
    }
    return response;
};
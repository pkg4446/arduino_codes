#include "word.h"
/*******************************************************/
String word_yes(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(word_yes_str); index++){
        response += char(pgm_read_byte_near(word_yes_str+index));
    }
    return response;
};
/*******************************************************/
String word_no(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(word_no_str); index++){
        response += char(pgm_read_byte_near(word_no_str+index));
    }
    return response;
};
/*******************************************************/
String word_cancle(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(word_cancle_str); index++){
        response += char(pgm_read_byte_near(word_cancle_str+index));
    }
    return response;
};
/*******************************************************/
String word_male(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(word_male_str); index++){
        response += char(pgm_read_byte_near(word_male_str+index));
    }
    return response;
};
/*******************************************************/
String word_female(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(word_female_str); index++){
        response += char(pgm_read_byte_near(word_female_str+index));
    }
    return response;
};
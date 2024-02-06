#include "display.h"
#include "utility.h"

/*******************************************************/
void display_cmd(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(interface_cmd); index++){
        response += char(pgm_read_byte_near(interface_cmd+index));
    }
    Serial.print(response);
};
/*******************************************************/
void display_model_err(){
    String response = "";
    for(uint16_t index=0; index<strlen_P(interface_model_err); index++){
        response += char(pgm_read_byte_near(interface_model_err+index));
    }
    Serial.print(response);
};
/*******************************************************/
void display_boot(){
    paging();
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_boot); index++){
        response += char(pgm_read_byte_near(scene_boot+index));
    }
    Serial.println(response);

    spacebar_option(true,2,word_yes());
    spacebar_option(true,1,word_no());

    response = "";
    for(uint16_t index=0; index<strlen_P(scene_boot_cmd); index++){
        response += char(pgm_read_byte_near(scene_boot_cmd+index));
    }
    spacebar_option(false,99,response);
    Serial.println();
};
/*******************************************************/
void display_help_cmd(){
    paging();
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_help_cmd); index++){
        response += char(pgm_read_byte_near(scene_help_cmd+index));
    }
    Serial.println(response);
};
/*******************************************************/
void display_make_assist(){
    paging();
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_make_assist); index++){
        response += char(pgm_read_byte_near(scene_make_assist+index));
    }
    Serial.println(response);
};
/*******************************************************/
void display_make_user(){
    paging();
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_make_user); index++){
        response += char(pgm_read_byte_near(scene_make_user+index));
    }
    Serial.println(response);
};
/*******************************************************/
void display_prologue(String name, bool gender){
    paging();
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_prologue1); index++){
        response += char(pgm_read_byte_near(scene_prologue1+index));
    }
    Serial.print(response);
    Serial.print(name);
    response = "";
    for(uint16_t index=0; index<strlen_P(scene_prologue2); index++){
        response += char(pgm_read_byte_near(scene_prologue2+index));
    }
    Serial.print(response);

    if(gender)  response = word_male();
    else        response = word_female();
    Serial.print(response);
    
    response = "";
    for(uint16_t index=0; index<strlen_P(scene_prologue3); index++){
        response += char(pgm_read_byte_near(scene_prologue3+index));
    }
    Serial.println(response);
};
/*******************************************************/
void display_continue(){
    paging();
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_continue); index++){
        response += char(pgm_read_byte_near(scene_continue+index));
    }
    Serial.println(response);
};
/*******************************************************/
void display_game_help(){
    paging();
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_help_game); index++){
        response += char(pgm_read_byte_near(scene_help_game+index));
    }
    Serial.println(response);
    spacebar_option(true,2,word_yes());
    spacebar_option(true,1,word_no());
    Serial.println();
};
/*******************************************************/
void display_hour(uint8_t *clock_hours){
    int8_t index_start = 0;
    int8_t index_end   = 83;
    String response    = "\n";
    if(*clock_hours<12){
        if(*clock_hours<3){index_start=0;index_end=20;}
        else if(*clock_hours<6){index_start=20;index_end=43;}
        else if(*clock_hours<9){index_start=43;index_end=63;}
        else {index_start=63;index_end=83;}
        for(uint16_t index=index_start; index<index_end; index++){
            response += char(pgm_read_byte_near(scene_sun_rise+index));
        }
    }else{
        if(*clock_hours<15){index_start=0;index_end=24;}
        else if(*clock_hours<18){index_start=24;index_end=41;}
        else if(*clock_hours<21){index_start=41;index_end=58;}
        else{index_start=58;index_end=78;}
        for(uint16_t index=index_start; index<index_end; index++){
            response += char(pgm_read_byte_near(scene_sun_fall+index));
        }
    }
    if(*clock_hours % 3 == 1) Serial.println(response);
}
/*******************************************************/
void cancle_cmd(bool line_break){
    if(line_break)  Serial.println();
    spacebar_option(true,COMMAND_CANCLE,"취소");
    Serial.println();
};
/*******************************************************/
void display_edu(){
    paging();
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_edu); index++){
        response += char(pgm_read_byte_near(scene_edu+index));
    }
    Serial.println(response);
};
/*******************************************************/
void display_info(){
    paging();
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_info); index++){
        response += char(pgm_read_byte_near(scene_info+index));
    }
    Serial.println(response);
};
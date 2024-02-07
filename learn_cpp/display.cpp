#include "display.h"
#include "utility.h"

void type_dly(){
    delay(10);
}
/*******************************************************/
void display_cmd(){
    for(uint16_t index=0; index<strlen_P(interface_cmd); index++){
       Serial.print(char(pgm_read_byte_near(interface_cmd+index)));
    }
};
/*******************************************************/
void display_model_err(){
    for(uint16_t index=0; index<strlen_P(interface_model_err); index++){
        Serial.print(char(pgm_read_byte_near(interface_model_err+index)));
    }
};
/*******************************************************/
void display_boot(){
    paging();
    for(uint16_t index=0; index<strlen_P(scene_boot); index++){
        Serial.print(char(pgm_read_byte_near(scene_boot+index)));
    }
    Serial.println();

    spacebar_option(true,COMMAND_YES,word_yes());
    spacebar_option(true,COMMAND_NO,word_no());

    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_boot_cmd); index++){
        response += char(pgm_read_byte_near(scene_boot_cmd+index));
    }
    spacebar_option(false,COMMAND_CANCLE,response);
    Serial.println();
};
/*******************************************************/
void display_hash_check(){
    paging();
    for(uint16_t index=0; index<strlen_P(scene_hash_check); index++){
        Serial.print(char(pgm_read_byte_near(scene_hash_check+index)));
    }
    Serial.println();
};
/*******************************************************/
void display_help_cmd(){
    paging();
    for(uint16_t index=0; index<strlen_P(scene_help_cmd); index++){
        Serial.print(char(pgm_read_byte_near(scene_help_cmd+index)));
    }
    Serial.println();
};
/*******************************************************/
void display_make_assist(){
    paging();
    for(uint16_t index=0; index<strlen_P(scene_make_assist); index++){
        Serial.print(char(pgm_read_byte_near(scene_make_assist+index)));
    }
    Serial.println();
};
/*******************************************************/
void display_make_user(){
    paging();
    for(uint16_t index=0; index<strlen_P(scene_make_user); index++){
        Serial.print(char(pgm_read_byte_near(scene_make_user+index)));
    }
    Serial.println();
};
/*******************************************************/
void display_prologue(String name, bool gender){
    paging();
    for(uint16_t index=0; index<strlen_P(scene_prologue1); index++){
        Serial.print(char(pgm_read_byte_near(scene_prologue1+index)));
        type_dly();
    }
    Serial.print(name);
    for(uint16_t index=0; index<strlen_P(scene_prologue2); index++){
        Serial.print(char(pgm_read_byte_near(scene_prologue2+index)));
        type_dly();
    }
    String response = "";
    if(gender)  response = word_male();
    else        response = word_female();
    Serial.print(response);
    
    for(uint16_t index=0; index<strlen_P(scene_prologue3); index++){
        Serial.print(char(pgm_read_byte_near(scene_prologue3+index)));
        type_dly();
    }
    Serial.println();
};
/*******************************************************/
void display_continue(){
    paging();
    for(uint16_t index=0; index<strlen_P(scene_continue); index++){
        Serial.print(char(pgm_read_byte_near(scene_continue+index)));
    }
    Serial.println();
};
/*******************************************************/
void display_game_help(){
    paging();
    for(uint16_t index=0; index<strlen_P(scene_help_game); index++){
        Serial.print(char(pgm_read_byte_near(scene_help_game+index)));
    }
    spacebar_option(true,COMMAND_YES,word_yes());
    spacebar_option(true,COMMAND_NO,word_no());
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
    spacebar_option(true,COMMAND_CANCLE,"word_cancle()");
    Serial.println();
};
/*******************************************************/
void display_cmd_main(){
    paging();
    for(uint16_t index=0; index<strlen_P(scene_main_cmd); index++){
        Serial.print(char(pgm_read_byte_near(scene_main_cmd+index)));
    }
    Serial.println();

    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_main_opt1); index++){
        response += char(pgm_read_byte_near(scene_main_opt1+index));
    }
    spacebar_option(true,COMMAND_RESOURCE,response);
    response = "";
    for(uint16_t index=0; index<strlen_P(scene_main_opt2); index++){
        response += char(pgm_read_byte_near(scene_main_opt2+index));
    }
    spacebar_option(true,COMMAND_DUNGEON,response);
    response = "";
    for(uint16_t index=0; index<strlen_P(scene_main_opt3); index++){
        response += char(pgm_read_byte_near(scene_main_opt3+index));
    }
    spacebar_option(true,COMMAND_STORE,response);
    Serial.println();
    response = "";
    for(uint16_t index=0; index<strlen_P(scene_main_opt4); index++){
        response += char(pgm_read_byte_near(scene_main_opt4+index));
    }
    spacebar_option(true,COMMAND_INVASION,response);
    response = "";
    for(uint16_t index=0; index<strlen_P(scene_main_opt5); index++){
        response += char(pgm_read_byte_near(scene_main_opt5+index));
    }
    spacebar_option(true,COMMAND_INFOMATION,response);
    response = "";
    for(uint16_t index=0; index<strlen_P(scene_main_opt6); index++){
        response += char(pgm_read_byte_near(scene_main_opt6+index));
    }
    spacebar_option(true,COMMAND_TRAINING,response);
    Serial.println();
};
/*******************************************************/
void display_info(){
    paging();
    for(uint16_t index=0; index<strlen_P(scene_info); index++){
        Serial.print(char(pgm_read_byte_near(scene_info+index)));
    }
    Serial.println();
};
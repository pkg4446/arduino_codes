#include "display.h"

void type_dly(void){
    delay(10);
}
/*******************************************************/
void cancle_cmd(bool line_break){
    if(line_break)  Serial.println();
    spacebar_option(true,COMMAND_CANCLE,get_progmem(word_cancle));
    Serial.println();
};
/*******************************************************/
void display_cmd(void){
    Serial.print(get_progmem(interface_cmd));
};
/*******************************************************/
void display_model_err(void){
    Serial.println(get_progmem(interface_model_err));
};
/*******************************************************/
void display_boot(void){
    paging();
    Serial.println(get_progmem(scene_boot));
    spacebar_option(true,COMMAND_YES,get_progmem(word_yes));
    spacebar_option(true,COMMAND_NO,get_progmem(word_no));
    spacebar_option(false,COMMAND_CANCLE,get_progmem(scene_boot_cmd));
    Serial.println();
};
/*******************************************************/
void display_hash_check(void){
    paging();
    Serial.println(get_progmem(scene_hash_check));
};
/*******************************************************/
void display_help_cmd(void){
    paging();
    Serial.println(get_progmem(scene_help_cmd));
};
/*******************************************************/
void display_make_assist(void){
    paging();
    Serial.println(get_progmem(scene_make_assist));
};
/*******************************************************/
void display_make_user(void){
    paging();
    Serial.println(get_progmem(scene_make_user));
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
    if(gender)  response = get_progmem(word_male);
    else        response = get_progmem(word_female);
    Serial.print(response);
    
    for(uint16_t index=0; index<strlen_P(scene_prologue3); index++){
        Serial.print(char(pgm_read_byte_near(scene_prologue3+index)));
        type_dly();
    }
    Serial.println();
};
/*******************************************************/
void display_continue(void){
    paging();
    Serial.println(get_progmem(scene_continue));
};
/*******************************************************/
void display_game_help(void){
    paging();
    Serial.println(get_progmem(scene_help_game));
    spacebar_option(true,COMMAND_YES,get_progmem(word_yes));
    spacebar_option(true,COMMAND_NO,get_progmem(word_no));
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
void display_cmd_main(void){
    paging();
    Serial.println(get_progmem(scene_main_cmd));
    spacebar_option(true,COMMAND_DUNGEON,get_progmem(scene_main_opt1));
    spacebar_option(true,COMMAND_INFOMATION,get_progmem(scene_main_opt2));
    spacebar_option(true,COMMAND_STORE,get_progmem(scene_main_opt3));
    Serial.println();
    spacebar_option(true,COMMAND_INVASION,get_progmem(scene_main_opt4));
    spacebar_option(true,COMMAND_TRAINING,get_progmem(scene_main_opt5));
    spacebar_option(true,COMMAND_REST,get_progmem(scene_main_opt6));
    Serial.println();
};
/*******************************************************/
void display_dungeon(void){
    paging();
    Serial.println(get_progmem(scene_main_cmd));
    spacebar_option(true,COMMAND_OBSTRUCT,get_progmem(scene_dungeon_opt1));
    spacebar_option(true,COMMAND_WAYLAY,get_progmem(scene_dungeon_opt2));
    spacebar_option(true,COMMAND_TRAP,get_progmem(scene_dungeon_opt3));
    Serial.println();
    spacebar_option(true,COMMAND_AMENITY,get_progmem(scene_dungeon_opt4));
    spacebar_option(true,COMMAND_TROOP,get_progmem(scene_dungeon_opt5));
    cancle_cmd(false);
};
/*******************************************************/
void display_cmd_invasion(void){
    paging();
    Serial.println(get_progmem(scene_main_cmd));
    spacebar_option(true,COMMAND_SEARCH,get_progmem(scene_invasion_opt1));
    spacebar_option(true,COMMAND_KIDNAP,get_progmem(scene_invasion_opt2));
    spacebar_option(true,COMMAND_ATTACK,get_progmem(scene_invasion_opt3));
    cancle_cmd(true);
};
/*******************************************************/
void display_info(void){
    paging();
    Serial.println(get_progmem(scene_info));
};
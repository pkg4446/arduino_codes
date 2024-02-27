#include "display.h"

void type_dly(void){
    delay(10);
}
/*******************************************************/
void cancle_cmd(bool line_break){
    if(line_break)  Serial.println();
    space_option(true,COMMAND_CANCLE,get_progmem(word_cancle));
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
/*************** Display ***************/
void display_rest(void){
    Serial.println(get_progmem(scene_rest));
};
/*******************************************************/
void display_boot(void){
    paging();
    Serial.println(get_progmem(scene_boot));
    space_option(true,COMMAND_YES,get_progmem(word_yes));
    space_option(true,COMMAND_NO,get_progmem(word_no));
    space_option(false,COMMAND_CANCLE,get_progmem(scene_boot_cmd));
    Serial.println();
};
/*******************************************************/
void display_troop(void){
    paging();
    Serial.print(get_progmem(scene_troop));
    Serial.print(get_progmem(gramma_ul_2));
    spacebar();
    Serial.println(get_progmem(scene_check));
};
/*******************************************************/
void display_captive(void){
    paging();
    Serial.print(get_progmem(scene_captive));
    Serial.print(get_progmem(gramma_ul));
    spacebar();
    Serial.println(get_progmem(scene_check));
};
/*******************************************************/
void display_hash_check(void){
    paging();
    Serial.print(get_progmem(scene_hash));
    Serial.print(get_progmem(gramma_ul));
    spacebar();
    Serial.println(get_progmem(scene_check));
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
    Serial.print(get_progmem(gramma_ip));
    Serial.print(get_progmem(gramma_la));
    spacebar();
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
    space_option(true,COMMAND_YES,get_progmem(word_yes));
    space_option(true,COMMAND_NO,get_progmem(word_no));
    Serial.println();
};
/*******************************************************/
void display_time(uint16_t *time_year, uint8_t *time_month, uint8_t *time_day, uint8_t *time_hour){
    Serial.print(*time_year);
    Serial.print(get_progmem(scene_time_y));
    Serial.print(*time_month);
    Serial.print(get_progmem(scene_time_m));
    Serial.print(*time_day);
    Serial.print(get_progmem(scene_time_d));
    Serial.print(*time_hour);
    Serial.println(get_progmem(scene_time_h));
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
    space_option(true,COMMAND_DUNGEON,get_progmem(scene_main_opt1));
    space_option(true,COMMAND_INFOMATION,get_progmem(scene_main_opt2));
    space_option(true,COMMAND_STORE,get_progmem(scene_main_opt3));
    Serial.println();
    space_option(true,COMMAND_INVASION,get_progmem(scene_main_opt4));
    space_option(true,COMMAND_TRAINING,get_progmem(scene_main_opt5));
    space_option(true,COMMAND_REST,get_progmem(scene_main_opt6));
    Serial.println();
};
/*******************************************************/
void display_dungeon(void){
    paging();
    Serial.println(get_progmem(scene_main_cmd));
    space_option(true,COMMAND_COORDINATE,get_progmem(scene_dungeon_opt1));
    space_option(true,COMMAND_OBSTRUCT,get_progmem(scene_dungeon_opt2));
    space_option(true,COMMAND_WAYLAY,get_progmem(scene_dungeon_opt3));
    Serial.println();
    space_option(true,COMMAND_AMENITY,get_progmem(scene_dungeon_opt4));
    space_option(true,COMMAND_MANAGEMENT,get_progmem(scene_dungeon_opt5));
    cancle_cmd(false);
};
/*******************************************************/
void display_coordinate(uint16_t *scene_number){
    *scene_number = COMMAND_COORDINATE;
    paging();
    Serial.println(get_progmem(scene_position));
    space_option(true,COMMAND_MOVE4,get_progmem(scene_pos_opt4));
    space_option(true,COMMAND_MOVE2,get_progmem(scene_pos_opt2));
    space_option(true,COMMAND_MOVE6,get_progmem(scene_pos_opt6));
    Serial.println();
    space_option(true,COMMAND_MOVE8,get_progmem(scene_pos_opt8));
    cancle_cmd(false);
};
/*******************************************************/
void display_amenity(uint16_t *scene_number){
    *scene_number = COMMAND_AMENITY;
    paging();
    Serial.println(get_progmem(scene_amenity));
    space_option(true,COMMAND_MENU1,get_progmem(word_trap));
    space_option(true,COMMAND_MENU2,get_progmem(word_prison));
    space_option(true,COMMAND_MENU3,get_progmem(word_spa));
    Serial.println();
    space_option(true,COMMAND_MENU4,get_progmem(word_inn));
    space_option(true,COMMAND_MENU5,get_progmem(word_farm));
    space_option(true,COMMAND_MENU6,get_progmem(word_cage));
    Serial.println();
    space_option(true,COMMAND_MENU7,get_progmem(word_enter));
    space_option(true,COMMAND_MENU8,get_progmem(word_core));
    cancle_cmd(false);
};
/*******************************************************/
void display_management(uint16_t *scene_number, String name){
    *scene_number = COMMAND_MANAGEMENT;
    paging();
    Serial.print(get_progmem(word_captive));
    spacebar();
    Serial.print(name);
    Serial.print(get_progmem(gramma_ul_1));
    Serial.print(get_progmem(gramma_ul_2));
    spacebar();
    Serial.println(get_progmem(scene_training));
    space_option(true,COMMAND_VICTIM,get_progmem(scene_train_opt1));
    space_option(true,COMMAND_EDUCATION,get_progmem(word_edu));
    space_option(true,COMMAND_TRANSFER,get_progmem(scene_train_opt2));
    Serial.println();
    space_option(true,COMMAND_MENU1,get_progmem(scene_captive));
    space_option(true,COMMAND_MENU2,get_progmem(scene_train_opt4));
    cancle_cmd(false);
};
/*******************************************************/
void display_transfer(uint16_t *scene_number){
    *scene_number = COMMAND_TRANSFER;
    paging();
    Serial.println(get_progmem(scene_transfer));
    space_option(true,COMMAND_MENU1,get_progmem(word_player));
    space_option(true,COMMAND_MENU2,get_progmem(word_assist));
    cancle_cmd(false);
};
/*******************************************************/
void display_victim(uint16_t *scene_number){
    *scene_number = COMMAND_VICTIM;
    paging();
    Serial.println(get_progmem(scene_victim));
    cancle_cmd(false);
};
void display_no_victim(void){
    paging();
    Serial.println(get_progmem(scene_no_victim));
};
/*******************************************************/
void display_release(void){
    Serial.print(get_progmem(gramma_ul_1));
    Serial.print(get_progmem(gramma_ul_2));
    spacebar();
    Serial.println(get_progmem(scene_release));
};
/*******************************************************/
void display_execute(void){
    Serial.print(get_progmem(gramma_ul_1));
    Serial.print(get_progmem(gramma_ul_2));
    spacebar();
    Serial.println(get_progmem(scene_execute));
};
/*******************************************************/
/*******************************************************/
void display_invasion(void){
    paging();
    Serial.println(get_progmem(scene_main_cmd));
    space_option(true,COMMAND_MENU1,get_progmem(scene_invasion_opt1));
    space_option(true,COMMAND_MENU2,get_progmem(word_kidnap));
    space_option(true,COMMAND_MENU3,get_progmem(word_attack));
    cancle_cmd(true);
};
/*******************************************************/
void display_invasion_empt(void){
    Serial.println(get_progmem(scene_invasion_empt));
};
/*******************************************************/
void display_villager_kidnap(void){
    Serial.print(get_progmem(gramma_ul_1));
    spacebar();
    Serial.print(get_progmem(word_kidnap));
    Serial.println(get_progmem(gramma_da));
};
/*******************************************************/
void display_villager_attack(void){
    Serial.print(get_progmem(gramma_ul_1));
    spacebar();
    Serial.print(get_progmem(word_attack));
    Serial.println(get_progmem(gramma_da));
};
/*******************************************************/
void display_info(void){
    paging();
    Serial.println(get_progmem(scene_info));
    space_option(true,COMMAND_MENU1,get_progmem(word_dungeon));
    space_option(true,COMMAND_MENU2,get_progmem(word_player));
    space_option(true,COMMAND_MENU3,get_progmem(word_assist));
    Serial.println();
    space_option(true,COMMAND_MENU4,get_progmem(word_captive));
    space_option(true,COMMAND_MENU5,get_progmem(word_troop));
    cancle_cmd(false);
};
/*******************************************************/
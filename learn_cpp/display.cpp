#include "display.h"
#include "utility.h"
#include "model_hard.h"
#include "model_soft.h"

/*******************************************************/
void display_boot(){
    paging();
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_boot); index++){
        response += char(pgm_read_byte_near(scene_boot+index));
    }
    Serial.println(response);

    response = "";
    for(uint16_t index=0; index<strlen_P(scene_boot_opt1); index++){
        response += char(pgm_read_byte_near(scene_boot_opt1+index));
    }
    spacebar_option(true,2,response);

    response = "";
    for(uint16_t index=0; index<strlen_P(scene_boot_opt2); index++){
        response += char(pgm_read_byte_near(scene_boot_opt2+index));
    }
    spacebar_option(true,1,response);
    Serial.println();
};
/*******************************************************/
void display_prologue(){
    paging();
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_prologue); index++){
        response += char(pgm_read_byte_near(scene_prologue+index));
    }
    Serial.println(response);
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

void display_newday(uint32_t *calendar, INFO *class_info, STAT *class_stat, MENS *class_mens, CURRENT *class_current){
    paging();
    spacebar(false,"Day");Serial.println(String(*calendar));
    paging();
    String genders = "♀";
    if(class_info->get_gender()) genders = "♂";
    spacebar(true,class_info->get_family() + class_info->get_name() + genders);
    if(!class_info->get_gender() && class_mens->get() == 2){
        Serial.println("mens");
    }else{
        Serial.println("");
    }
    uint8_t get_stamina = class_current->get(4);
    uint8_t get_mental  = class_current->get(5);
    spacebar(true,"체력");   Serial.println(get_stamina);
    spacebar(true,"정신력"); Serial.println(get_mental);
}
/*******************************************************/
void cancle_cmd(bool line_break){
    if(line_break)  Serial.println();
    spacebar_option(true,COMMAND_CANCLE,"취소");
    Serial.println();
};
/*******************************************************/
void display_shelter(){
    paging();
    String response = "";

    for(uint16_t index=0; index<strlen_P(scene_main_opt1); index++)
    {   response += char(pgm_read_byte_near(scene_main_opt1+index)); }
    spacebar_option(true,COMMAND_EXPLORE,response);
    response       = "";

    for(uint16_t index=0; index<strlen_P(scene_main_opt2); index++)
    {   response += char(pgm_read_byte_near(scene_main_opt2+index)); }
    spacebar_option(true,COMMAND_EDUCATION,response);
    response       = "";
    
    for(uint16_t index=0; index<strlen_P(scene_main_opt3); index++)
    {   response += char(pgm_read_byte_near(scene_main_opt3+index)); }
    spacebar_option(true,COMMAND_INFOMATION,response);
    Serial.println();
};
/*******************************************************/
void display_explore(bool sweet_home){
    paging();
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_explore); index++){
        response += char(pgm_read_byte_near(scene_explore+index));
    }
    Serial.println(response);

    response = "";
    for(uint16_t index=0; index<strlen_P(scene_explore_opt1); index++)
    {   response += char(pgm_read_byte_near(scene_explore_opt1+index)); }
    spacebar_option(true,EXPLORE_AROUND,response);

    response       = "";
    for(uint16_t index=0; index<strlen_P(scene_explore_opt2); index++)
    {   response += char(pgm_read_byte_near(scene_explore_opt2+index)); }
    spacebar_option(true,EXPLORE_MOVE,response);

    if(!sweet_home){
        response       = "";
        for(uint16_t index=0; index<strlen_P(scene_explore_opt3); index++)
        {   response += char(pgm_read_byte_near(scene_explore_opt3+index)); }
        spacebar_option(true,EXPLORE_SHELTER,response);
    }else{
        response       = "";
        for(uint16_t index=0; index<strlen_P(scene_explore_opt4); index++)
        {   response += char(pgm_read_byte_near(scene_explore_opt4+index)); }
        spacebar_option(true,EXPLORE_SHELTER,response);
    }
    Serial.println();
};
/*******************************************************/
void display_explore_move(){
    paging();
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_explore_move); index++){
        response += char(pgm_read_byte_near(scene_explore_move+index));
    }
    Serial.println(response);

    spacebar_option(true,DIRECTION_EAST,"동");
    spacebar_option(true,DIRECTION_WAST,"서");
    spacebar_option(true,DIRECTION_SOUTH,"남");
    spacebar_option(true,DIRECTION_NORTH,"북");
    Serial.println();
    cancle_cmd(true);
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
#include "display.h"
#include "utility.h"


void display_hour(uint8_t *clock_hours){
    paging();
    int8_t index_start=0;
    int8_t index_end=161;
    if(*clock_hours<3){index_start=0;index_end=20;}
    else if(*clock_hours<6){index_start=20;index_end=43;}
    else if(*clock_hours<9){index_start=43;index_end=63;}
    else if(*clock_hours<12){index_start=63;index_end=83;}
    else if(*clock_hours<15){index_start=83;index_end=107;}
    else if(*clock_hours<18){index_start=107;index_end=124;}

    else if(*clock_hours<21){index_start=124;index_end=141;}
    else{index_start=141;index_end=161;}
    String response = "";
    for(uint16_t index=index_start; index<index_end; index++){
        response += char(pgm_read_byte_near(scene_sun_rise+index));
    }
    
    Serial.println(strlen_P(scene_sun_rise));
    Serial.println(response);
    paging();
}

void display_newday(uint32_t *calendar, INFO *class_info, STAT *class_stat, MENS *class_mens, CURRENT *class_current){
    paging();
    spacebar(false,"Day");Serial.println(String(*calendar));
    paging();
    spacebar(true,class_info->get_family() + class_info->get_name());
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
void display_boot(){
    paging();
    String progmem_number="";
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_boot); index++){
        response += char(pgm_read_byte_near(scene_boot+index));
    }
    Serial.println(response);

    response = "";
    progmem_number = char(pgm_read_byte_near(scene_boot_opt1));
    for(uint16_t index=1; index<strlen_P(scene_boot_opt1); index++){
        response += char(pgm_read_byte_near(scene_boot_opt1+index));
    }
    spacebar_option(true,progmem_number,response);

    response = "";
    progmem_number = char(pgm_read_byte_near(scene_boot_opt2));
    for(uint16_t index=1; index<strlen_P(scene_boot_opt2); index++){
        response += char(pgm_read_byte_near(scene_boot_opt2+index));
    }
    spacebar_option(true,progmem_number,response);
    Serial.println();
};
/*******************************************************/
void display_main(){
    paging();
    String progmem_number="";
    String response = "";

    for(uint16_t index=0; index<3; index++)
    {   progmem_number += char(pgm_read_byte_near(scene_main_opt1+index)); }
    for(uint16_t index=3; index<strlen_P(scene_main_opt1); index++)
    {   response += char(pgm_read_byte_near(scene_main_opt1+index)); }
    spacebar_option(true,progmem_number,response);
    response       = "";
    progmem_number = "";
    for(uint16_t index=0; index<3; index++)
    {   progmem_number += char(pgm_read_byte_near(scene_main_opt2+index)); }
    for(uint16_t index=3; index<strlen_P(scene_main_opt2); index++)
    {   response += char(pgm_read_byte_near(scene_main_opt2+index)); }
    spacebar_option(true,progmem_number,response);
    response       = "";
    progmem_number = "";
    for(uint16_t index=0; index<3; index++)
    {   progmem_number += char(pgm_read_byte_near(scene_main_opt3+index)); }
    for(uint16_t index=3; index<strlen_P(scene_main_opt3); index++)
    {   response += char(pgm_read_byte_near(scene_main_opt3+index)); }
    spacebar_option(true,progmem_number,response);
    response       = "";
    progmem_number = "";
    for(uint16_t index=0; index<3; index++)
    {   progmem_number += char(pgm_read_byte_near(scene_main_opt4+index)); }
    for(uint16_t index=3; index<strlen_P(scene_main_opt4); index++)
    {   response += char(pgm_read_byte_near(scene_main_opt4+index)); }
    spacebar_option(true,progmem_number,response);
    Serial.println();
};
/*******************************************************/
void display_explore(){
    paging();
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_explore); index++){
        response += char(pgm_read_byte_near(scene_explore+index));
    }
    Serial.println(response);
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
/*******************************************************/
void display_shop(){
    paging();
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_shop); index++){
        response += char(pgm_read_byte_near(scene_shop+index));
    }
    Serial.println(response);
};
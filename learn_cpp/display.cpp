#include "display.h"
#include "utility.h"

void display_newday(uint32_t calendar, INFO *class_info, STAT *class_stat, MENS *class_mens, CURRENT *class_current){
    paging();
    spacebar(false,"Day");Serial.println(String(calendar));
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
    String progmem_number="";
    String response = "";
    for(uint16_t index=0; index<strlen_P(scene_boot); index++){
        response += char(pgm_read_byte_near(scene_boot+index));
    }
    paging();
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
    String progmem_number="";
    String response = "";
    paging();

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
    String response = "";
/*
    for(uint16_t index=0; index<strlen_P(scene_main); index++){
        response += char(pgm_read_byte_near(scene_main+index));
    }
*/
    paging();
    Serial.println(response);
};
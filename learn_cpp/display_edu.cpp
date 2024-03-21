#include "display_edu.h"

void type_dly(void){
    delay(10);
}
/*******************************************************/
void edu_chk_body(String model_path){
    paging();
    Serial.print(get_progmem(word_captive));
    Serial.print(get_progmem(gramma_ui));
    spacebar();
    Serial.print(get_progmem(word_body));
    Serial.print(get_progmem(word_check));
    Serial.print(get_progmem(gramma_ul_2));
    spacebar();
    Serial.print(get_progmem(word_doing));
    Serial.print(get_progmem(gramma_da));
    //포로의 신체검사를 실시했다.
    INFO      *info_class   = new INFO();
    HEAD      *head_class   = new HEAD();
    BODY      *body_class   = new BODY();
    EROGENOUS *parts_class  = new EROGENOUS();
    read_model_hard(model_path, info_class, head_class, body_class, parts_class);
    //이름, 머리색, 컬, 눈동자 색, 쌍커플, 보조개, 대머리,
    //혈액형, 키, 가슴둘래&컵, 허리, 엉덩이, 다리비율,
    delete info_class;
    delete head_class;
    delete body_class;
    delete parts_class;
    /*
    
    */
    for(uint16_t index=0; index<strlen_P(scene_prologue1); index++){
        Serial.print(char(pgm_read_byte_near(scene_prologue1+index)));
        type_dly();
    }
    Serial.print(get_progmem(gramma_ip));
    Serial.println();
};
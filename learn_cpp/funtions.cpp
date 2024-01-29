#include "funtions.h"
#include "utility.h"

#define DEBUG
void view_status(String title,INFO *class_info,HEAD *class_head,BODY *class_body,EROGENOUS *class_parts,STAT *class_stat,HOLE *class_hole,SENSE *class_sense,NATURE *class_nature,EROS *class_eros){
    #ifdef DEBUG
    perforation(title);
    class_info-> status();
    class_head-> status();
    class_body-> status();
    class_body-> get_weight();
    class_parts->status();
    class_stat-> status();
    class_hole-> status(class_info->get_gender());
    class_sense->status(class_info->get_gender());
    class_nature->status();
    class_eros-> status();
    #endif
}

void routines_day(bool gender, MENS *class_mens, CURRENT *class_current){
    class_mens->daily(gender);
    class_current->daily();
    #ifdef DEBUG
        perforation("daily");
        if(!gender){
            class_mens->status();
            Serial.println(class_mens->get());
        }
        class_current->status();
    #endif
}
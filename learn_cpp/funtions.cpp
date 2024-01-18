#include "funtions.h"
#include "utility.h"

#define DEBUG

void new_model(bool gender,INFO *class_info,HEAD *class_head,BODY *class_body,EROGENOUS *class_parts,STAT *class_stat,HOLE *class_hole,SENSE *class_sense,NATURE *class_nature,EROS *class_eros){
    class_info-> generate(gender, false);
    class_head-> generate(class_info->get_gender());
    class_body-> generate(class_info->get_gender());
    class_parts->generate(class_info->get_gender());
    class_stat-> generate();
    class_hole-> generate();
    class_sense->generate();
    class_nature->generate();
    class_eros-> generate();
}

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
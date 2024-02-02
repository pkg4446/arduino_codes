#include "coresys.h"
#include "funtions.h"

void new_model(bool gender, String model_path){
    /***** HARDWARE *****/
    INFO        *info_class[model_gen];
    HEAD        *head_class[model_gen];
    BODY        *body_class[model_gen];
    EROGENOUS   *parts_class[model_gen];
    STAT        *stat_class[model_gen];
    HOLE        *hole_class[model_gen];
    SENSE       *sense_class[model_gen];
    NATURE      *nature_class[model_gen];
    EROS        *eros_class[model_gen];
    /***** SOFTWARE *****/
    EXP         *exp_class      = new EXP();
    CURRENT     *current_class  = new CURRENT();
    MENS        *mens_class     = new MENS();
    BREED       *breed_class    = new BREED();
    /***** MODELS *****/
    for(uint8_t index=0; index<model_gen; index++){
        info_class[index]   = new INFO();
        head_class[index]   = new HEAD();
        body_class[index]   = new BODY();
        parts_class[index]  = new EROGENOUS();
        stat_class[index]   = new STAT();
        hole_class[index]   = new HOLE();
        sense_class[index]  = new SENSE();
        nature_class[index] = new NATURE();
        eros_class[index]   = new EROS();
    }
    /***** MOTHER *****/
    info_class[0]-> generate(false, false);
    
    head_class[0]-> set_gender(info_class[0]->get_gender());
    body_class[0]-> set_gender(info_class[0]->get_gender());
    parts_class[0]->set_gender(info_class[0]->get_gender());

    head_class[0]-> generate();
    body_class[0]-> generate();
    parts_class[0]->generate();
    stat_class[0]-> generate();
    hole_class[0]-> generate();
    sense_class[0]->generate();
    nature_class[0]->generate();
    eros_class[0]-> generate();

    String models = info_class[0]->get_csv();
    make_csv_text(&models, head_class[0]->get_csv());
    make_csv_text(&models, body_class[0]->get_csv());
    make_csv_text(&models, parts_class[0]->get_csv());
    make_csv_text(&models, stat_class[0]->get_csv());
    make_csv_text(&models, hole_class[0]->get_csv());
    make_csv_text(&models, sense_class[0]->get_csv());
    make_csv_text(&models, nature_class[0]->get_csv());
    make_csv_text(&models, eros_class[0]->get_csv());
    file_write(model_path+"mother.csv", models);
    /***** FATHER *****/
    info_class[1]-> generate(true, false);
    
    head_class[1]-> set_gender(info_class[1]->get_gender());
    body_class[1]-> set_gender(info_class[1]->get_gender());
    parts_class[1]->set_gender(info_class[1]->get_gender());

    head_class[1]-> generate();
    body_class[1]-> generate();
    parts_class[1]->generate();
    stat_class[1]-> generate();
    hole_class[1]-> generate();
    sense_class[1]->generate();
    nature_class[1]->generate();
    eros_class[1]-> generate();

    models = info_class[1]->get_csv();
    make_csv_text(&models, head_class[1]->get_csv());
    make_csv_text(&models, body_class[1]->get_csv());
    make_csv_text(&models, parts_class[1]->get_csv());
    make_csv_text(&models, stat_class[1]->get_csv());
    make_csv_text(&models, hole_class[1]->get_csv());
    make_csv_text(&models, sense_class[1]->get_csv());
    make_csv_text(&models, nature_class[1]->get_csv());
    make_csv_text(&models, eros_class[1]->get_csv());
    file_write(model_path+"father.csv", models);
    /***** BABY *****/
    info_class[2]-> generate(gender, false);
    info_class[2]-> set_family(info_class[0]->get_family());
    
    Serial.print("gender:");
    Serial.println(info_class[2]->get_gender());

    head_class[2]-> set_gender(info_class[2]->get_gender());
    body_class[2]-> set_gender(info_class[2]->get_gender());
    parts_class[2]->set_gender(info_class[2]->get_gender());

    head_class[2]-> blend(head_class[0],head_class[1]);
    body_class[2]-> blend(body_class[0],body_class[1]);
    parts_class[2]->blend(parts_class[0],parts_class[1]);
    stat_class[2]-> blend(stat_class[0],stat_class[1]);
    hole_class[2]-> blend(hole_class[0],hole_class[1]);
    sense_class[2]->blend(sense_class[0],sense_class[1]);
    nature_class[2]->blend(nature_class[0],nature_class[1]);
    eros_class[2]-> blend(eros_class[0],eros_class[1]);

    models = info_class[2]->get_csv();
    make_csv_text(&models, head_class[2]->get_csv());
    make_csv_text(&models, body_class[2]->get_csv());
    make_csv_text(&models, parts_class[2]->get_csv());
    make_csv_text(&models, stat_class[2]->get_csv());
    make_csv_text(&models, hole_class[2]->get_csv());
    make_csv_text(&models, sense_class[2]->get_csv());
    make_csv_text(&models, nature_class[2]->get_csv());
    make_csv_text(&models, eros_class[2]->get_csv());
    file_write(model_path+"model.csv", models);
}

void read_model(String model_path,INFO *class_info,HEAD *class_head,BODY *class_body,EROGENOUS *class_parts,STAT *class_stat,HOLE *class_hole,SENSE *class_sense,NATURE *class_nature,EROS *class_eros){
    uint8_t file_number = dir_list(model_path,false,false);
    if(file_number == 3){
        String csv_file_str = file_read(model_path+"model.csv").c_str();
        char *csv_file  = const_cast<char*>(csv_file_str.c_str());
        char *class_text[9];
        class_text[0] = strtok(csv_file, "\n");
        for(uint8_t index=1; index<9; index++){
            class_text[index] = strtok(0x00, "\n");
            Serial.println(class_text[index]);
        }
        class_info  ->set_csv(class_text[0]);
        class_head  ->set_csv(class_text[1]);
        class_body  ->set_csv(class_text[2]);
        class_parts ->set_csv(class_text[3]);
        class_stat  ->set_csv(class_text[4]);
        class_hole  ->set_csv(class_text[5]);
        class_sense ->set_csv(class_text[6]);
        class_nature->set_csv(class_text[7]);
        class_eros  ->set_csv(class_text[8]);
    }
    /*
    Serial.println(class_info->get_csv());
    Serial.println(class_head->get_csv());
    Serial.println(class_body->get_csv());
    Serial.println(class_parts->get_csv());
    Serial.println(class_stat->get_csv());
    Serial.println(class_hole->get_csv());
    Serial.println(class_sense->get_csv());
    Serial.println(class_nature->get_csv());
    Serial.println(class_eros->get_csv());
    */
}
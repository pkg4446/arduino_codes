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
    dir_make(model_path);
    String file_name = model_path;
    /***** MOTHER *****/
    info_class[0]-> generate(false, false);
    head_class[0]-> generate(info_class[0]->get_gender());
    body_class[0]-> generate(info_class[0]->get_gender());
    parts_class[0]->generate(info_class[0]->get_gender());
    stat_class[0]-> generate();
    hole_class[0]-> generate();
    sense_class[0]->generate();
    nature_class[0]->generate();
    eros_class[0]-> generate();

    String models = info_class[2]->get_csv();
    make_csv_text(&models, head_class[2]->get_csv());
    make_csv_text(&models, body_class[2]->get_csv());
    make_csv_text(&models, parts_class[2]->get_csv());
    make_csv_text(&models, stat_class[2]->get_csv());
    make_csv_text(&models, hole_class[2]->get_csv());
    make_csv_text(&models, sense_class[2]->get_csv());
    make_csv_text(&models, nature_class[2]->get_csv());
    make_csv_text(&models, eros_class[2]->get_csv());
    file_name = model_path+"mother.csv";
    Serial.println(file_name);
    file_write(file_name, models);
    /***** FATHER *****/
    info_class[1]-> generate(true, false);
    head_class[1]-> generate(info_class[1]->get_gender());
    body_class[1]-> generate(info_class[1]->get_gender());
    parts_class[1]->generate(info_class[1]->get_gender());
    stat_class[1]-> generate();
    hole_class[1]-> generate();
    sense_class[1]->generate();
    nature_class[1]->generate();
    eros_class[1]-> generate();

    models = info_class[2]->get_csv();
    make_csv_text(&models, head_class[2]->get_csv());
    make_csv_text(&models, body_class[2]->get_csv());
    make_csv_text(&models, parts_class[2]->get_csv());
    make_csv_text(&models, stat_class[2]->get_csv());
    make_csv_text(&models, hole_class[2]->get_csv());
    make_csv_text(&models, sense_class[2]->get_csv());
    make_csv_text(&models, nature_class[2]->get_csv());
    make_csv_text(&models, eros_class[2]->get_csv());
    file_name = model_path+"father.csv";
    Serial.println(file_name);
    file_write(file_name, models);
    /***** BABY *****/
    info_class[2]-> generate(gender, false);
    info_class[2]-> set_family(info_class[0]->get_family());
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
    file_name = model_path+"avatar.csv";
    Serial.println(file_name);
    file_write(file_name, models);
}
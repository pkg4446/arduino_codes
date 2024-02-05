#include "coresys.h"
#include "funtions.h"

void new_model_body(String model_path, bool gender){
    /***** GENE *****/
    /***** HARDWARE *****/
    INFO        *info_class[model_gen];
    HEAD        *head_class[model_gen];
    BODY        *body_class[model_gen];
    EROGENOUS   *parts_class[model_gen];
    /***** SOFTWARE *****/
    STAT        *stat_class[model_gen];
    HOLE        *hole_class[model_gen];
    SENSE       *sense_class[model_gen];
    NATURE      *nature_class[model_gen];
    EROS        *eros_class[model_gen];
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
    Sha1.init();
    Sha1.print(models);
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
    Sha1.print(models);
    String hashs = Sha1.result();
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
    Sha1.init();
    Sha1.print(models);
    make_csv(&hashs, Sha1.result());
    file_write(model_path+"hash.csv", hashs);
    make_csv_text(&models, stat_class[2]->get_csv());
    make_csv_text(&models, hole_class[2]->get_csv());
    make_csv_text(&models, sense_class[2]->get_csv());
    make_csv_text(&models, nature_class[2]->get_csv());
    make_csv_text(&models, eros_class[2]->get_csv());
    file_write(model_path+"model.csv", models);

    for(uint8_t index=0; index<model_gen; index++){
        delete info_class[index];
        delete head_class[index];
        delete body_class[index];
        delete parts_class[index];
        delete stat_class[index];
        delete hole_class[index];
        delete sense_class[index];
        delete nature_class[index];
        delete eros_class[index];
    }
}

void new_model_status(String model_path, bool gender){
    /***** SOFTWARE *****/
    MENS    *mens_class     = new MENS();
    CURRENT *current_class  = new CURRENT();
    EXP     *exp_class      = new EXP();
    BREED   *breed_class    = new BREED();
    mens_class  ->generate(gender);
    current_class->generate();
    exp_class   ->generate();
    breed_class ->generate();

    String models = mens_class->get_csv();
    file_write(model_path+"mens.csv", models);
    models = current_class->get_csv();
    file_write(model_path+"current.csv", models);
    models = exp_class->get_csv();
    make_csv_text(&models, breed_class->get_csv());
    file_write(model_path+"exps.csv", models);

    delete exp_class;
    delete current_class;
    delete mens_class;
    delete breed_class;
}

bool check_model_hash(String model_path, bool type){
    bool response = false;
    if(exisits_check(model_path+"hash.csv") && exisits_check(model_path+"model.csv") && exisits_check(model_path+"mother.csv") && exisits_check(model_path+"father.csv")){
        String csv_file_str = file_read(model_path+"hash.csv").c_str();
        String hash_value[2];
        char *csv_file  = const_cast<char*>(csv_file_str.c_str());
        hash_value[0] = strtok(csv_file, ",");
        hash_value[1] = strtok(0x00, ",");
        Sha1.init();
        if(!type){
            csv_file_str = file_read(model_path+"mother.csv") + file_read(model_path+"father.csv");
            
        }else{
            csv_file_str = file_read(model_path+"model.csv").c_str();
            char *model_csv = const_cast<char*>(csv_file_str.c_str());
            String class_text[4];
            class_text[0] = strtok(model_csv, "\n");
            for(uint8_t index=1; index<4; index++){
                class_text[index] = strtok(0x00, "\n");
            }
            csv_file_str = class_text[0];
            make_csv_text(&csv_file_str, class_text[1]);
            make_csv_text(&csv_file_str, class_text[2]);
            make_csv_text(&csv_file_str, class_text[3]);
        }
        Sha1.print(csv_file_str);
        String hashs = Sha1.result();
        Serial.println(hashs);
        Serial.println(hash_value[type]);
    }
    return response;
}

void read_model_body(String model_path,INFO *class_info,HEAD *class_head,BODY *class_body,EROGENOUS *class_parts,STAT *class_stat,HOLE *class_hole,SENSE *class_sense,NATURE *class_nature,EROS *class_eros){
    uint8_t file_number = dir_list(model_path,false,false);
    if(exisits_check(model_path+"model.csv") && exisits_check(model_path+"mother.csv") && exisits_check(model_path+"father.csv")){
        String csv_file_str = file_read(model_path+"model.csv").c_str();
        char *csv_file  = const_cast<char*>(csv_file_str.c_str());
        char *class_text[9];
        class_text[0] = strtok(csv_file, "\n");
        for(uint8_t index=1; index<9; index++){
            class_text[index] = strtok(0x00, "\n");
            //Serial.println(class_text[index]);
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
        /*
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
        */
    }else{
        Serial.println("nofile");
    }
}


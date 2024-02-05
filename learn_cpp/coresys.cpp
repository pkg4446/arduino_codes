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
    file_write(model_path+file_mother(), models);
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
    file_write(model_path+file_father(), models);
    Sha1.print(models);
    String hashs = Sha1.result();
    /***** BABY *****/
    info_class[2]-> generate(gender, false);
    info_class[2]-> set_family(info_class[0]->get_family());
    
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
    file_write(model_path+file_hard(), models);
    models = stat_class[2]->get_csv();
    make_csv_text(&models, hole_class[2]->get_csv());
    make_csv_text(&models, sense_class[2]->get_csv());
    make_csv_text(&models, nature_class[2]->get_csv());
    make_csv_text(&models, eros_class[2]->get_csv());
    Sha1.init();
    Sha1.print(models);
    make_csv(&hashs, Sha1.result());
    file_write(model_path+file_hash(), hashs);
    file_write(model_path+file_soft(), models);
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
    file_write(model_path+file_mens(), models);
    models = current_class->get_csv();
    file_write(model_path+file_current(), models);
    models = exp_class->get_csv();
    make_csv_text(&models, breed_class->get_csv());
    file_write(model_path+file_exp(), models);

    delete exp_class;
    delete current_class;
    delete mens_class;
    delete breed_class;
}

void model_kill(String model_path){
    file_remove(model_path+file_mother());
    file_remove(model_path+file_father());
    file_remove(model_path+file_hard());
    file_remove(model_path+file_soft());
    file_remove(model_path+file_mens());
    file_remove(model_path+file_current());
    file_remove(model_path+file_exp());
    file_remove(model_path+file_hash());
    dir_remove(model_path);
}

bool check_model_hash(String model_path, uint8_t hash_num){
    bool response = false;
    if(exisits_check(model_path+file_hash()) && exisits_check(model_path+file_hard()) && exisits_check(model_path+file_soft()) && exisits_check(model_path+file_mother()) && exisits_check(model_path+file_father())){
        String csv_file_str = file_read(model_path+file_hash());
        String hash_value[3];
        char *csv_file  = const_cast<char*>(csv_file_str.c_str());
        hash_value[0] = strtok(csv_file, ",");
        hash_value[1] = strtok(0x00, ",");
        hash_value[2] = strtok(0x00, ",");
        Sha1.init();
        if(hash_num == 1){
            csv_file_str = file_read(model_path+file_hard());
        }else if(hash_num == 2){
            csv_file_str = file_read(model_path+file_soft());
        }else{
            csv_file_str = file_read(model_path+file_mother()) + file_read(model_path+file_father());
        }
        Sha1.print(csv_file_str);
        String hashs = Sha1.result();
        response = (hashs == hash_value[hash_num]);
    }else{
        model_kill(model_path);
        display_model_err();
    }
    return response;
}

void read_model_hard(String model_path,INFO *class_info,HEAD *class_head,BODY *class_body,EROGENOUS *class_parts){
    if(check_model_hash(model_path,1)){
        String csv_file_str = file_read(model_path+file_hard());
        char *csv_file  = const_cast<char*>(csv_file_str.c_str());
        char *class_text[9];
        class_text[0] = strtok(csv_file, "\n");
        for(uint8_t index=1; index<4; index++){
            class_text[index] = strtok(0x00, "\n");
        }
        class_info  ->set_csv(class_text[0]);
        class_head  ->set_csv(class_text[1]);
        class_body  ->set_csv(class_text[2]);
        class_parts ->set_csv(class_text[3]);
    }else{
        Serial.println("nofile");
    }
}

void read_model_soft(String model_path,STAT *class_stat,HOLE *class_hole,SENSE *class_sense,NATURE *class_nature,EROS *class_eros){
    if(check_model_hash(model_path,2)){
        String csv_file_str = file_read(model_path+file_soft());
        char *csv_file  = const_cast<char*>(csv_file_str.c_str());
        char *class_text[9];
        class_text[0] = strtok(csv_file, "\n");
        for(uint8_t index=1; index<9; index++){
            class_text[index] = strtok(0x00, "\n");
        }
        class_stat  ->set_csv(class_text[0]);
        class_hole  ->set_csv(class_text[1]);
        class_sense ->set_csv(class_text[2]);
        class_nature->set_csv(class_text[3]);
        class_eros  ->set_csv(class_text[4]);
    }else{
        Serial.println("nofile");
    }
}

void read_model_mens(String model_path, MENS *mens_class){
    String csv_file_str = file_read(model_path+file_mens());
    char *csv_file  = const_cast<char*>(csv_file_str.c_str());
    mens_class  ->set_csv(csv_file);
}
void read_model_feel(String model_path, CURRENT *feel_class){
    String csv_file_str = file_read(model_path+file_mens());
    char *csv_file  = const_cast<char*>(csv_file_str.c_str());
    feel_class  ->set_csv(csv_file);
}
void read_model_breed(String model_path, BREED *breed_class){
    String csv_file_str = file_read(model_path+file_mens());
    char *csv_file  = const_cast<char*>(csv_file_str.c_str());
    breed_class ->set_csv(csv_file);
}

/*
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
*/
#include "coresys.h"
/***** Inner funtion *****/
String merge_parent_csv(INFO *class_info,HEAD *class_head,BODY *class_body,EROGENOUS *class_parts,STAT *class_stat,HOLE *class_hole,SENSE *class_sense,NATURE *class_nature,EROS *class_eros){
    String response = "";
    make_csv_text(&response, class_info->get_csv());
    make_csv_text(&response, class_head->get_csv());
    make_csv_text(&response, class_body->get_csv());
    make_csv_text(&response, class_parts->get_csv());
    make_csv_text(&response, class_stat->get_csv());
    make_csv_text(&response, class_hole->get_csv());
    make_csv_text(&response, class_sense->get_csv());
    make_csv_text(&response, class_nature->get_csv());
    make_csv_text(&response, class_eros->get_csv());
    return response;
}
String merge_hard_csv(INFO *class_info,HEAD *class_head,BODY *class_body,EROGENOUS *class_parts){
    String response = "";
    make_csv_text(&response, class_info->get_csv());
    make_csv_text(&response, class_head->get_csv());
    make_csv_text(&response, class_body->get_csv());
    make_csv_text(&response, class_parts->get_csv());
    return response;
}
String merge_soft_csv(STAT *class_stat,HOLE *class_hole,SENSE *class_sense,NATURE *class_nature,EROS *class_eros){
    String response = "";
    make_csv_text(&response, class_stat->get_csv());
    make_csv_text(&response, class_hole->get_csv());
    make_csv_text(&response, class_sense->get_csv());
    make_csv_text(&response, class_nature->get_csv());
    make_csv_text(&response, class_eros->get_csv());
    return response;
}
/***** Inner funtion *****/
void read_model_gene(String model_path,bool parent,HEAD *class_head,BODY *class_body,EROGENOUS *class_parts,STAT *class_stat,HOLE *class_hole,SENSE *class_sense,NATURE *class_nature,EROS *class_eros){
    if(check_model_hash(model_path,0)){
        String csv_file_str = "";
        if(parent) csv_file_str = file_read(model_path+file_father());
        else       csv_file_str = file_read(model_path+file_mother());
        char *csv_file  = const_cast<char*>(csv_file_str.c_str());
        char *class_text[9];
        class_text[0]   = strtok(csv_file, "\n");
        for(uint8_t index=1; index<9; index++){
            class_text[index] = strtok(0x00, "\n");
        }
        class_head  ->set_csv(class_text[1]);
        class_body  ->set_csv(class_text[2]);
        class_parts ->set_csv(class_text[3]);
        class_stat  ->set_csv(class_text[4]);
        class_hole  ->set_csv(class_text[5]);
        class_sense ->set_csv(class_text[6]);
        class_nature->set_csv(class_text[7]);
        class_eros  ->set_csv(class_text[8]);
    }
}
void gene_meiosis(String model_path,INFO *class_info,HEAD *class_head,BODY *class_body,EROGENOUS *class_parts,STAT *class_stat,HOLE *class_hole,SENSE *class_sense,NATURE *class_nature,EROS *class_eros){
    /***** HARDWARE *****/
    HEAD        *head_class[parents_gen];
    BODY        *body_class[parents_gen];
    EROGENOUS   *parts_class[parents_gen];
    /***** SOFTWARE *****/
    STAT        *stat_class[parents_gen];
    HOLE        *hole_class[parents_gen];
    SENSE       *sense_class[parents_gen];
    NATURE      *nature_class[parents_gen];
    EROS        *eros_class[parents_gen];

    for(uint8_t index=0; index<parents_gen; index++){
        head_class[index]   = new HEAD();
        body_class[index]   = new BODY();
        parts_class[index]  = new EROGENOUS();
        stat_class[index]   = new STAT();
        hole_class[index]   = new HOLE();
        sense_class[index]  = new SENSE();
        nature_class[index] = new NATURE();
        eros_class[index]   = new EROS();
    }

    read_model_gene(model_path,false,head_class[0],body_class[0],parts_class[0],stat_class[0],hole_class[0],sense_class[0],nature_class[0],eros_class[0]);
    read_model_gene(model_path,true,head_class[1],body_class[1],parts_class[1],stat_class[1],hole_class[1],sense_class[1],nature_class[1],eros_class[1]);
    if(check_model_hash(model_path,0)){
        String csv_file_str = file_read(model_path+file_hard());
        char *csv_file      = const_cast<char*>(csv_file_str.c_str());
        char *class_info_csv = strtok(csv_file, "\n");
        class_info->set_csv(class_info_csv);
    }
    class_head->   meiosis(head_class[0],  head_class[1]);
    class_body->   meiosis(body_class[0],  body_class[1]);
    class_parts->  meiosis(parts_class[0], parts_class[1]);
    class_stat->   meiosis(stat_class[0],  stat_class[1]);
    class_hole->   meiosis(hole_class[0],  hole_class[1]);
    class_sense->  meiosis(sense_class[0], sense_class[1]);
    class_nature-> meiosis(nature_class[0],nature_class[1]);
    class_eros->   meiosis(eros_class[0],  eros_class[1]);

    for(uint8_t index=0; index<parents_gen; index++){
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
/***** Inner funtion *****/

void save_time_csv(uint16_t *time_year, uint8_t *time_month, uint8_t *time_day, uint8_t *time_hour){
    String time_csv = "";
    make_csv(&time_csv,String(*time_year));
    make_csv(&time_csv,String(*time_month));
    make_csv(&time_csv,String(*time_day));
    make_csv(&time_csv,String(*time_hour));
    file_write(path_config()+file_time(), time_csv);

};
void load_time_csv(uint16_t *time_year, uint8_t *time_month, uint8_t *time_day, uint8_t *time_hour){
    if(exisits_check(path_config()+file_time())){
        String time_csv = file_read(path_config()+file_time());
        char *csv_file  = const_cast<char*>(time_csv.c_str());
        *time_year  = atoi(strtok(csv_file, ","));
        *time_month = atoi(strtok(0x00, ","));
        *time_day   = atoi(strtok(0x00, ","));
        *time_hour  = atoi(strtok(0x00, ","));
    }
};

void new_model(String model_path, bool gender){
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
    info_class[0]-> generate(false, false);
    info_class[1]-> generate(true, false);
    info_class[2]-> generate(gender, false);
    info_class[2]-> set_family(info_class[0]->get_family());

    for(uint8_t index=0; index<model_gen; index++){
        head_class[index]-> set_gender(info_class[index]->get_gender());
        body_class[index]-> set_gender(info_class[index]->get_gender());
        parts_class[index]->set_gender(info_class[index]->get_gender());

        if(index < model_gen-1){
            head_class[index]-> generate();
            body_class[index]-> generate();
            parts_class[index]->generate();
            stat_class[index]-> generate();
            hole_class[index]-> generate();
            sense_class[index]->generate();
            nature_class[index]->generate();
            eros_class[index]-> generate();
        }
    }
    head_class[2]-> blend(head_class[0],head_class[1]);
    body_class[2]-> blend(body_class[0],body_class[1]);
    parts_class[2]->blend(parts_class[0],parts_class[1]);
    stat_class[2]-> blend(stat_class[0],stat_class[1]);
    hole_class[2]-> blend(hole_class[0],hole_class[1]);
    sense_class[2]->blend(sense_class[0],sense_class[1]);
    nature_class[2]->blend(nature_class[0],nature_class[1]);
    eros_class[2]-> blend(eros_class[0],eros_class[1]);

    String hashs  = "";
    String models = merge_parent_csv(info_class[0],head_class[0],body_class[0],parts_class[0],stat_class[0],hole_class[0],sense_class[0],nature_class[0],eros_class[0]);
    file_write(model_path+file_mother(), models);
    hash.init();
    hash.print(models);
    
    models = merge_parent_csv(info_class[1],head_class[1],body_class[1],parts_class[1],stat_class[1],hole_class[1],sense_class[1],nature_class[1],eros_class[1]);
    file_write(model_path+file_father(), models);
    hash.print(models);
    make_csv(&hashs, hash.result());

    models = merge_hard_csv(info_class[2],head_class[2],body_class[2],parts_class[2]);
    hash.init();
    hash.print(models);
    make_csv(&hashs, hash.result());
    file_write(model_path+file_hard(), models);

    models = merge_soft_csv(stat_class[2],hole_class[2],sense_class[2],nature_class[2],eros_class[2]);
    hash.init();
    hash.print(models);
    make_csv(&hashs, hash.result());
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
    /***** STATUS *****/
    MENS    *mens_class     = new MENS();
    CURRENT *current_class  = new CURRENT();
    EXP     *exp_class      = new EXP();
    BREED   *breed_class    = new BREED();
    mens_class  ->generate(gender);
    current_class->generate();
    exp_class   ->generate();
    breed_class ->generate();

    file_write(model_path+file_mens(), mens_class->get_csv());
    file_write(model_path+file_current(), current_class->get_csv());
    file_write(model_path+file_exp(), exp_class->get_csv());
    file_write(model_path+file_breed(), breed_class->get_csv());

    delete exp_class;
    delete current_class;
    delete mens_class;
    delete breed_class;
}

String read_hash_text(String model_path){
    String response = "error";
    if(exisits_check(model_path+file_hash())){
        String csv_file_str = file_read(model_path+file_hash());
        char *csv_file  = const_cast<char*>(csv_file_str.c_str());
        strtok(csv_file, ",");
        strtok(0x00, ",");
        response = strtok(0x00, ",");
    }
    return response;
}

bool check_model_hash(String model_path, uint8_t hash_num){
    return true;
}
/*
bool check_model_hash(String model_path, uint8_t hash_num){
    bool response = false;
    if(exisits_check(model_path+file_hash()) && exisits_check(model_path+file_hard()) && exisits_check(model_path+file_soft()) && exisits_check(model_path+file_mother()) && exisits_check(model_path+file_father())){
        String csv_file_str = file_read(model_path+file_hash());
        String hash_value[3];
        char *csv_file  = const_cast<char*>(csv_file_str.c_str());
        hash_value[0] = strtok(csv_file, ",");
        hash_value[1] = strtok(0x00, ",");
        hash_value[2] = strtok(0x00, ",");
        hash.init();
        if(hash_num == 1){
            csv_file_str = file_read(model_path+file_hard());
        }else if(hash_num == 2){
            csv_file_str = file_read(model_path+file_soft());
        }else{
            csv_file_str = file_read(model_path+file_mother()) + file_read(model_path+file_father());
        }
        hash.print(csv_file_str);
        String hashs = hash.result();
        response = (hashs == hash_value[hash_num]);
    }
    if(!response){
        dir_remove(model_path);
        display_model_err();
    }
    return response;
}
*/
void read_model_hard(String model_path,INFO *class_info,HEAD *class_head,BODY *class_body,EROGENOUS *class_parts){
    String csv_file_str = file_read(model_path+file_hard());
    char *csv_file  = const_cast<char*>(csv_file_str.c_str());
    char *class_text[4];
    class_text[0] = strtok(csv_file, "\n");
    for(uint8_t index=1; index<4; index++){
        class_text[index] = strtok(0x00, "\n");
    }
    class_info  ->set_csv(class_text[0]);
    class_head  ->set_csv(class_text[1]);
    class_body  ->set_csv(class_text[2]);
    class_parts ->set_csv(class_text[3]);
}
void read_model_hard_info(String model_path,INFO *class_info){
    String csv_file_str = file_read(model_path+file_hard());
    char *csv_file  = const_cast<char*>(csv_file_str.c_str());
    char *class_text= strtok(csv_file, "\n");
    class_info->set_csv(class_text);
}
void read_model_hard_head(String model_path,HEAD *class_head){
    String csv_file_str = file_read(model_path+file_hard());
    char *csv_file  = const_cast<char*>(csv_file_str.c_str());
    strtok(csv_file, "\n");
    char *class_text= strtok(0x00, "\n");
    class_head  ->set_csv(class_text);
}
void read_model_hard_body(String model_path,BODY *class_body){
    String csv_file_str = file_read(model_path+file_hard());
    char *csv_file  = const_cast<char*>(csv_file_str.c_str());
    strtok(csv_file, "\n");
    strtok(0x00, "\n");
    char *class_text= strtok(0x00, "\n");
    class_body  ->set_csv(class_text);
}
void read_model_hard_parts(String model_path,EROGENOUS *class_parts){
    String csv_file_str = file_read(model_path+file_hard());
    char *csv_file  = const_cast<char*>(csv_file_str.c_str());
    strtok(csv_file, "\n");
    strtok(0x00, "\n");
    strtok(0x00, "\n");
    char *class_text= strtok(0x00, "\n");
    class_parts ->set_csv(class_text);
}

void read_model_soft(String model_path,STAT *class_stat,HOLE *class_hole,SENSE *class_sense,NATURE *class_nature,EROS *class_eros){
    String csv_file_str = file_read(model_path+file_soft());
    char *csv_file  = const_cast<char*>(csv_file_str.c_str());
    char *class_text[5];
    class_text[0] = strtok(csv_file, "\n");
    for(uint8_t index=1; index<5; index++){
        class_text[index] = strtok(0x00, "\n");
    }
    class_stat  ->set_csv(class_text[0]);
    class_hole  ->set_csv(class_text[1]);
    class_sense ->set_csv(class_text[2]);
    class_nature->set_csv(class_text[3]);
    class_eros  ->set_csv(class_text[4]);
}
void read_model_soft_stat(String model_path, STAT *class_stat){
    String csv_file_str = file_read(model_path+file_soft());
    char *csv_file  = const_cast<char*>(csv_file_str.c_str());
    char *class_text= strtok(csv_file, "\n");
    class_stat  ->set_csv(class_text);
}

void read_model_mens(String model_path, MENS *mens_class){
    String csv_file_str = file_read(model_path+file_mens());
    char *csv_file  = const_cast<char*>(csv_file_str.c_str());
    mens_class  ->set_csv(csv_file);
}
void read_model_feel(String model_path, CURRENT *feel_class){
    String csv_file_str = file_read(model_path+file_current());
    char *csv_file  = const_cast<char*>(csv_file_str.c_str());
    feel_class  ->set_csv(csv_file);
}
void read_model_exp(String model_path, EXP *exp_class){
    String csv_file_str = file_read(model_path+file_exp());
    char *csv_file  = const_cast<char*>(csv_file_str.c_str());
    exp_class->set_csv(csv_file);
}
void read_model_breed(String model_path, BREED *breed_class){
    String csv_file_str = file_read(model_path+file_mens());
    char *csv_file  = const_cast<char*>(csv_file_str.c_str());
    breed_class->set_csv(csv_file);
}

//////////여기 바꾸기///shot 하고 mother에 gene으로.
void pregnant_baby(String mother_path, String father_path, bool gender){
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
    
    gene_meiosis(mother_path,info_class[0],head_class[0],body_class[0],parts_class[0],stat_class[0],hole_class[0],sense_class[0],nature_class[0],eros_class[0]);
    gene_meiosis(father_path,info_class[1],head_class[1],body_class[1],parts_class[1],stat_class[1],hole_class[1],sense_class[1],nature_class[1],eros_class[1]);

    info_class[2]-> generate(gender, true);
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

    String model_path = mother_path + path_womb();
    dir_make(model_path);
    String hashs  = "";
    String models = merge_parent_csv(info_class[0],head_class[0],body_class[0],parts_class[0],stat_class[0],hole_class[0],sense_class[0],nature_class[0],eros_class[0]);
    file_write(model_path+file_mother(), models);
    hash.init();
    hash.print(models);
    
    models = merge_parent_csv(info_class[1],head_class[1],body_class[1],parts_class[1],stat_class[1],hole_class[1],sense_class[1],nature_class[1],eros_class[1]);
    file_write(model_path+file_father(), models);
    hash.print(models);
    make_csv(&hashs, hash.result());

    models = merge_hard_csv(info_class[2],head_class[2],body_class[2],parts_class[2]);
    hash.init();
    hash.print(models);
    make_csv(&hashs, hash.result());
    file_write(model_path+file_hard(), models);

    models = merge_soft_csv(stat_class[2],hole_class[2],sense_class[2],nature_class[2],eros_class[2]);
    hash.init();
    hash.print(models);
    make_csv(&hashs, hash.result());
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
    /***** STATUS *****/
    MENS    *mens_class     = new MENS();
    CURRENT *current_class  = new CURRENT();
    EXP     *exp_class      = new EXP();
    BREED   *breed_class    = new BREED();
    mens_class  ->generate(gender);
    current_class->generate();
    exp_class   ->generate();
    breed_class ->generate();

    file_write(model_path+file_mens(), mens_class->get_csv());
    file_write(model_path+file_current(), current_class->get_csv());
    file_write(model_path+file_exp(), exp_class->get_csv());
    file_write(model_path+file_breed(), breed_class->get_csv());

    delete exp_class;
    delete current_class;
    delete mens_class;
    delete breed_class;
}

uint8_t mens_check(String model_path, bool daily){
    MENS    *mens_class = new MENS();
    String csv_file_str = file_read(model_path+file_mens());
    mens_class->set_csv(const_cast<char*>(csv_file_str.c_str()));
    if(daily && mens_class->daily()){
        file_write(model_path+file_mens(), mens_class->get_csv());
    }
    return mens_class->get();
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
/***** funtion gene *******/
/*
void pregnant(String family_name){
  info_class[e_baby]->generate(random(2), true);
  info_class[e_baby]->set_family(family_name);
}
*/
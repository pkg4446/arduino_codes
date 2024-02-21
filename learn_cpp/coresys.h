#pragma once
#include <arduino.h>
#include "model_hard.h"
#include "model_soft.h"
#include "model_status.h"

#include "filesys.h"
#include "hash.h"
#include "display.h"
#include "path_string.h"
#include "funtions.h"

#define parents_gen 2
#define model_gen   3

void save_time_csv(uint16_t *time_year, uint8_t *time_month, uint8_t *time_day, uint8_t *time_hour);
void load_time_csv(uint16_t *time_year, uint8_t *time_month, uint8_t *time_day, uint8_t *time_hour);

void new_model(String model_path, bool gender);

String read_hash_text(String model_path);
bool check_model_hash(String model_path, uint8_t hash_num);
void read_model_hard(String model_path, INFO *class_info, HEAD *class_head, BODY *class_body, EROGENOUS *class_parts);
void read_model_hard_info(String model_path, INFO *class_info);
void read_model_hard_head(String model_path, HEAD *class_head);
void read_model_hard_body(String model_path, BODY *class_body);
void read_model_hard_parts(String model_path,EROGENOUS *class_parts);
void read_model_soft(String model_path, STAT *class_stat, HOLE *class_hole, SENSE *class_sense, NATURE *class_nature, EROS *class_eros);
void read_model_soft_stat(String model_path, STAT *class_stat);

void read_model_mens(String model_path, MENS *mens_class);
void read_model_feel(String model_path, CURRENT *feel_class);
void read_model_exp(String model_path, EXP *exp_class);
void read_model_breed(String model_path, BREED *breed_class);

void pregnant_baby(String father_path, String mother_path, bool gender);
uint8_t mens_check(String model_path, bool daily);

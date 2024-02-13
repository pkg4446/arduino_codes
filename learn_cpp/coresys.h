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

void new_model(String model_path, bool gender);

bool check_model_hash(String model_path, uint8_t hash_num);
void read_model_hard(String model_path,INFO *class_info,HEAD *class_head,BODY *class_body,EROGENOUS *class_parts);
void read_model_soft(String model_path,STAT *class_stat,HOLE *class_hole,SENSE *class_sense,NATURE *class_nature,EROS *class_eros);
void read_model_mens(String model_path,MENS *mens_class);
void read_model_feel(String model_path,CURRENT *feel_class);
void read_model_breed(String model_path,BREED *breed_class);

void pregnant_baby(String father_path, String mother_path, bool gender);
uint8_t mens_check(String model_path, bool daily);
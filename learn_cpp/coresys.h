#pragma once
#include "filesys.h"

#include "model_hard.h"
#include "model_soft.h"

#define model_gen 3

void new_model_hardware(String model_path, bool gender);
void new_model_software(String model_path, bool gender);

void read_model_hardware(String model_path,INFO *class_info,HEAD *class_head,BODY *class_body,EROGENOUS *class_parts,STAT *class_stat,HOLE *class_hole,SENSE *class_sense,NATURE *class_nature,EROS *class_eros);
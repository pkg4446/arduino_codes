#pragma once
#include "arduino.h"
#include <SPI.h>
#include <SD.h>

const PROGMEM char check_sdcard1[] = "SD check...";
const PROGMEM char check_sdcard2[] = "SD Card Mount Failed. retrying...";
const PROGMEM char check_sdcard3[] = "SD Card is OK";

void    sd_init();
bool    exisits_check(String path);

String   dir_index(String path, bool type, uint16_t dir_index);
uint16_t dir_list(String path, bool type);
void     dir_make(String path);
void     dir_remove(String path);
void     dir_move(String path, String target);

String   file_read(String path);
void     file_write(String path, String contents);
void     file_append(String path, String contents);
void     file_remove(String path);
void     file_move(String dir, String file, String target);
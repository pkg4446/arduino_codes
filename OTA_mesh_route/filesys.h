#pragma once
#include <arduino.h>
#include <SPI.h>
#include <SD.h>

const PROGMEM char check_sdcard1[]  = "SD check...";
const PROGMEM char check_sdcard2[]  = "SD Card 가 없네요! 카드를 삽입 해주세요!";
const PROGMEM char check_sdcard3[]  = "시스템을 시작합니다.";

void     sd_init();
bool     exisits_check(String path);

String   dir_index(String path, bool type, uint8_t dir_index);
uint8_t  dir_list(String path, bool type, bool show);
void     dir_make(String path);
void     dir_remove(String path);
void     dir_move(String path, String target);

String   file_read(String path);
void     file_write(String path, String contents);
void     file_writest(String path, uint8_t contents[], uint16_t f_index);
void     file_append(String path, String contents);
void     file_remove(String path);
void     file_move(String dir, String file, String target);
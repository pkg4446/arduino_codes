#pragma once
#include <arduino.h>
#include <SPI.h>
#include <SD.h>

const PROGMEM char check_sdcard1[] = "SD check...";
const PROGMEM char check_sdcard2[] = "SD Card 가 없네요! 카드 삽입 후 재부팅 해주세요!";
const PROGMEM char check_sdcard3[] = "시스템을 시작합니다.";

void    sd_init();
bool    exisits_check(String path);

String   dir_index(String path, bool type, uint16_t dir_index);
uint16_t dir_list(String path, bool type);
void     dir_make(String path);
void     dir_remove(String path);

String   file_read(String path);
void     file_write(String path, String contents);
void     file_append(String path, String contents);
void     file_remove(String path);
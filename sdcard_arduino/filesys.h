#pragma once
#include "arduino.h"
#include <SPI.h>
#include <SD.h>

const PROGMEM char check_sdcard1[] = "SD check...";
const PROGMEM char check_sdcard2[] = "SD Card Mount Failed";
const PROGMEM char check_sdcard3[] = "SD Card is OK";

void sd_init();
bool exisits_check(String path);

void dir_make(String path);
void dir_remove(String path);

uint16_t dir_list(String path, bool type);
String   dir_index(String path, bool type, uint16_t dir_index);
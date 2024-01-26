/*
  Listfiles

  This example shows how print out the files in a
  directory on a SD card

  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  created   Nov 2010
  by David A. Mellis
  modified 9 Apr 2012
  by Tom Igoe
  modified 2 Feb 2014
  by Scott Fitzgerald

  This example code is in the public domain.

*/
#include <SPI.h>
#include <SD.h>
#include "filesys.h"

String path_current = "/";

#define COMMAND_LENGTH 128
/***** funtion command ****/
void command_helf() {
  Serial.println("***** help *****");
  Serial.println("cmd   help");
  Serial.println("dir   show items");
  Serial.println("cd    move path");
  Serial.println("cd/   move root");
  Serial.println("mk    make ");
  Serial.println("rm    remove");
  Serial.println("***** help *****");
}
char command_buf[COMMAND_LENGTH] = {0x00,};
uint8_t command_num = 0;
void get_command(char ch) {
  if(ch=='\n'){
    command_buf[ command_num ] = 0x00;
    command_num = 0;
    command_progress();
  }else if(ch!='\r'){
    command_buf[ command_num++ ] = ch;
    command_num %= COMMAND_LENGTH;
  }
}
void command_progress(){
  if(command_buf[0]=='c' && command_buf[1]=='m' && command_buf[2]=='d'){
    command_helf();
  }else if(command_buf[0]=='d' && command_buf[1]=='i' && command_buf[2]=='r'){
    dir_list(path_current,true);
  }else if(command_buf[0]=='c' && command_buf[1]=='d' && command_buf[2]==0x20){
    uint8_t command_index_check = 3;
    String temp_text = "";
    for(uint8_t index_check=3; index_check<COMMAND_LENGTH; index_check++){
      temp_text += command_buf[index_check];
      if(command_buf[index_check] == 0x00) break;
    }
    if(exisits_check(path_current+temp_text+"/")){
      path_current += temp_text+"/";
      Serial.println("폴더 이동!");
    }else{
      Serial.println("폴더가 없자녀!");
    }
  }else if(command_buf[0]=='c' && command_buf[1]=='d' && command_buf[2]=='/'){
    path_current = "/";
  }else if(command_buf[0]=='m' && command_buf[1]=='k'){

  }else if(command_buf[0]=='r' && command_buf[1]=='m'){

  }
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  sd_init();

  dir_list("/",false); //true = dir, false = file
  dir_index("/",false,4);
  dir_index("/",true,4);

  File root = SD.open(path_current);
  Serial.print("root: ");
  Serial.println(root);

  dir_make("/ardu1/");
  dir_make("/ardu2/");
  dir_remove("/ARDUINO/");

  command_helf();
}

void loop() {
  if (Serial.available()) get_command(Serial.read());
}
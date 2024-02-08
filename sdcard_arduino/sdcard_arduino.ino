#include <SPI.h>
#include <SD.h>
#include "filesys.h"

String path_current = "/";

#define COMMAND_LENGTH 128
/***** funtion command ****/
void command_helf() {
  Serial.println("***** help *****");
  Serial.println("help  this text");
  Serial.println("ls    show list");
  Serial.println("cd    move path");
  Serial.println("cd/   move root");
  Serial.println("md    make dir");
  Serial.println("rd    remove dir");
  Serial.println("op    open file");
  Serial.println("rf    remove file");
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
  String temp_text = "";
  for(uint8_t index_check=3; index_check<COMMAND_LENGTH; index_check++){
    if(command_buf[index_check] == 0x00) break;
    temp_text += command_buf[index_check];
  }
  if(command_buf[0]=='h' && command_buf[1]=='e' && command_buf[2]=='l' && command_buf[3]=='p'){
    command_helf();
  }else if(command_buf[0]=='c' && command_buf[1]=='d' && command_buf[2]==0x20){
    uint8_t command_index_check = 3;
    if(exisits_check(path_current+temp_text+"/")){
      path_current += temp_text;
      path_current += "/";
    }
    Serial.println(path_current);
  }else if(command_buf[0]=='c' && command_buf[1]=='d' && command_buf[2]=='/'){
    path_current = "/";
    Serial.println(path_current);
  }else if(command_buf[0]=='l' && command_buf[1]=='s'){
    dir_list(path_current,true,true);
  }else if(command_buf[0]=='m' && command_buf[1]=='d'){
    dir_make(path_current+temp_text);
  }else if(command_buf[0]=='r' && command_buf[1]=='d'){
    dir_remove(path_current+temp_text);
  }else if(command_buf[0]=='r' && command_buf[1]=='f'){
    file_remove(path_current+temp_text);
  }else if(command_buf[0]=='o' && command_buf[1]=='p'){
    Serial.println(file_read(path_current+temp_text));
  }else if(command_buf[0]=='r' && command_buf[1]=='f'){
    file_remove(path_current+temp_text);
  }else{
    Serial.println("wrong command!");
  }
}

void setup() {
  Serial.begin(115200);

  sd_init();

  dir_list("/",false,true); //true = dir, false = file
  dir_index("/",false,4);
  dir_index("/",true,4);

/*
  file_write("/test.csv","test1 file2 write!!!");
  Serial.print("read : ");
  Serial.println(file_read("/test.csv"));

  file_append("/test.csv","test1 file2 write!!!");
  Serial.print("read : ");
  Serial.println(file_read("/test.csv"));
*/
  
  command_helf();
}

void loop() {
  if (Serial.available()) get_command(Serial.read());
}
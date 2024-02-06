#include "filesys.h"
#if defined(ESP32)
  #include "FS.h"
  void createDir(fs::FS &fs, String path){
    fs.mkdir(path);
  }
  void removeDir(fs::FS &fs, String path){
    fs.rmdir(path);
  }
  void removeFile(fs::FS &fs, String path){
    fs.remove(path);
  }
#endif

void sd_init() {
  // Open serial communications and wait for port to open:
  uint8_t chipSelect = 53;
  #if defined(ESP32)
    chipSelect = 5;
  #endif
  bool sd_flage = SD.begin(chipSelect);
  
  String check_sd = "";
  for(uint16_t index=0; index<strlen_P(check_sdcard1); index++){
      check_sd += char(pgm_read_byte_near(check_sdcard1+index));
  }
  Serial.println(check_sd);

  check_sd = "";
  if (!sd_flage) {
    for(uint16_t index=0; index<strlen_P(check_sdcard2); index++){
        check_sd += char(pgm_read_byte_near(check_sdcard2+index));
    }
  }else{
    for(uint16_t index=0; index<strlen_P(check_sdcard3); index++){
        check_sd += char(pgm_read_byte_near(check_sdcard3+index));
    }
  }
  Serial.println(check_sd);
}

bool exisits_check(String path){
  return SD.exists(path);
}

void dir_make(String path){
  if(!exisits_check(path)){
    #if defined(ESP32)
      createDir(SD,path);
    #else
      SD.mkdir(path);
    #endif
  }
}

void dir_remove(String path){
  if(exisits_check(path)){
    #if defined(ESP32)
      removeDir(SD,path);
    #else
      uint16_t dir_last   = dir_list(path,true,false);
      String   now_path   = "";
      uint16_t file_last  = 0;
      for(uint16_t index_d=dir_last; index_d>0; index_d--){
        now_path  = path+"/"+ dir_index(path,true,index_d);
        file_last = dir_list(now_path,false,false)
        for(uint16_t index_f=file_last; index_f>0; index_f--){
          file_remove(now_path+"/"+ dir_index(path,false,index_f));
        }
        SD.rmdir(now_path);
      }
      file_last = dir_list(path,false,false);
      for(uint16_t index=file_last; index>0; index--){
        file_remove(path +"/"+ dir_index(path,false,index));
      }
      SD.rmdir(path);
    #endif
    if(exisits_check(path)){
      String response = "";
      for(uint16_t index=0; index<strlen_P(sdcard_option1); index++){
          response += char(pgm_read_byte_near(sdcard_option1+index));
      }
      Serial.println(response);
    }
  }
}

uint16_t dir_list(String path, bool type, bool show) {
  uint16_t type_index = 0;
  File root = SD.open(path);
  if(!root){return 0;}
  if(!root.isDirectory()){return 0;}

  File file = root.openNextFile();
  if(show) Serial.println(path);
  while(file){
    if(show) Serial.print("\t");
    if(file.isDirectory()){
      if(type)type_index++;
      if(show){
        Serial.print(file.name());
        Serial.println("/");
      }
    }else{
      if(!type)type_index++;
      if(show){
        Serial.print(file.name());
        Serial.print("\t");
        Serial.println(file.size());
      }
    }
    file = root.openNextFile();
  }
  return type_index;
}

String dir_index(String path, bool type, uint16_t dir_index) {
  String response = "";
  if(dir_index != 0){
    File root = SD.open(path);
    File file = root.openNextFile();
    while(file){
      if(file.isDirectory()){
        if(type)dir_index--;
      }else{
        if(!type)dir_index--;
      }
      if(dir_index <1 ){
        response = file.name();
        break;
      }
      file = root.openNextFile();
    }
  }
  return response;
}

String file_read(String path){
  File file;
  String response = "";
  #if defined(ESP32)
    fs::FS &fs = SD;
    file = fs.open(path);
  #else
    file = SD.open(path);
  #endif
  while (file.available()) {
    response += char(file.read());
  }
  file.close();
  return response;
}

void file_write(String path, String contents){
  File file;
  #if defined(ESP32)
    fs::FS &fs = SD;
    file = fs.open(path, FILE_WRITE);
  #else
    /*
    file = SD.open(path, O_CREAT|O_RDWR);
    file.seek (0);
    */
    SD.remove(path);
    file = SD.open(path, FILE_WRITE);
  #endif
  file.print(contents);
  file.close();
}

void file_append(String path, String contents){
  File file;
  #if defined(ESP32)
    fs::FS &fs = SD;
    file = fs.open(path, FILE_APPEND);
  #else
    file = SD.open(path, FILE_WRITE);
  #endif
  file.print(contents);
  file.close();
}

void file_remove(String path){
  if(exisits_check(path)){
    #if defined(ESP32)
      removeFile(SD,path);
    #else
      SD.remove(path);
    #endif
  }
}
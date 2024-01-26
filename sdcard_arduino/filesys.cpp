#include "filesys.h"
#if defined(ESP32)
  #include "FS.h"
  void createDir(fs::FS &fs, String path){
    fs.mkdir(path);
  }
  void removeDir(fs::FS &fs, String path){
    fs.rmdir(path);
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
  exisits_check(path);
  #if defined(ESP32)
    createDir(SD,path);
  #else
    SD.mkdir(path);
  #endif
}

void dir_remove(String path){
  exisits_check(path);
  #if defined(ESP32)
    removeDir(SD,path);
  #else
    SD.remove(path);
  #endif
}

uint16_t dir_list(String path, bool type) {
  uint16_t type_index = 0;
  File root = SD.open(path);
  if(!root){return 0;}
  if(!root.isDirectory()){return 0;}

  File file = root.openNextFile();
  if(path != "/") Serial.println("/");
  while(file){
    Serial.print("\t");
    if(file.isDirectory()){
      if(type)type_index++;
      Serial.print(file.name());
      Serial.println("/");
    }else{
      if(!type)type_index++;
      Serial.print("\t");
      Serial.print(file.name());
      Serial.print("\t");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
  Serial.print("Listing type:");
  Serial.println(type_index);
  return type_index;
}

String dir_index(String path, bool type, uint16_t dir_index) {
  File root = SD.open(path);
  File file = root.openNextFile();
  String response = "null";

  while(file){
    if(dir_index <=1 ){
      response = file.name();
      break;
    }
    if(file.isDirectory()){
      if(type)dir_index--;
    }else{
      if(!type)dir_index--;
    }
    file = root.openNextFile();
  }

  Serial.println(response);
  return response;
}

/*

uint32_t listDir(fs::FS &fs, const char * dirname){  
  uint32_t file_index = 0;
  File root = fs.open(dirname);
  if(!root){return 0;}
  if(!root.isDirectory()){return 0;}
  File file = root.openNextFile();
  while(file){
      if(!file.isDirectory()){
        file_index++;
        Serial.print("  FILE: ");
        Serial.print(file.name());
        Serial.print("  SIZE: ");
        Serial.println(file.size());
        } 
      file = root.openNextFile();
  }
  Serial.print("Listing files:");
  Serial.println(file_index);
  return file_index;
}

uint8_t list(const char * dirname) {
  File root = fs.open(dirname);
  while (true) {
    File entry =  root.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
*/
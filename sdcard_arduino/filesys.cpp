#include "filesys.h"

void init(uint8_t chipSelect){
  uint8_t index = 0;
  if (!SD.begin(chipSelect)) {
    Serial.println("SD-X");
  }else{
    Serial.println("SD-O");
  }
}

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

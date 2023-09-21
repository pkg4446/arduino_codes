/*
 * Connect the SD card to the following pins:
 *
 * SD Card | ESP32
 *    D2       -
 *    D3       SS
 *    CMD      MOSI
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      SCK
 *    VSS      GND
 *    D0       MISO
 *    D1       -
 */
#include "FS.h"
#include "SD.h"
#include "SPI.h"

uint32_t log_index  = 0;

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

boolean readFile(fs::FS &fs, String path){
    boolean response = true;
    
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        response = false;
    }
    if(file.size()>1048576){response = false;}    
    file.close();
    
    return response;
}

void writeFile(fs::FS &fs, String path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, String path, String message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void setup(){
    Serial.begin(115200);
    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return;
    }
  log2SD();
    
}

void log2SD(){
  uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    uint32_t file_index  = listDir(SD, "/");  

    String filename  = "/log_" + String(file_index) + ".csv"; 
    boolean file_max = readFile(SD, filename);
    if(!file_max){
      file_index++;
      filename  = "/log_" + String(file_index) + ".csv";
      writeFile(SD, filename, "index, temp, humi, interval, timestamp\n");
      log_index = 0;
    }
    log_index++;
    appendFile(SD, filename, String(log_index) + ", log1, log2, 3, tt\n");
    
}

void loop(){
}

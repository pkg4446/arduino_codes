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
char command_buf[COMMAND_LENGTH] = {0x00,};
uint8_t command_num = 0;
void get_command(char ch) {
  if(ch=='\n'){
    command_buf[ command_num ] = 0x00;
    command_num = 0;
    command_progress(command_buf);
  }else if(ch!='\r'){
    command_buf[ command_num++ ] = ch;
    command_num %= COMMAND_LENGTH;
  }
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  sd_init();

  dir_list("/",false);
  dir_list("/",true);

  dir_index("/",false,1);
  dir_index("/",false,2);
  dir_index("/",false,3);
  dir_index("/",false,4);
  dir_index("/",false,5);

  File root = SD.open(path_current);
  Serial.print("root: ");
  Serial.println(root);

  dir_make("/ardu1/");
  dir_make("/ardu2/");
  dir_remove("/ARDUINO/");
  all_list(root,0);

}

void loop() {
  if (Serial.available()) get_command(Serial.read());
}
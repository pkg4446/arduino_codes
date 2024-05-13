#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <painlessMesh.h>

#include "filesys_esp.h"

#define   MESH_PREFIX     "smartHiveMesh"
#define   MESH_PASSWORD   "smarthive123"
#define   MESH_PORT       3333

painlessMesh   mesh;
String routeID = "";
SimpleList<uint32_t> nodes;

uint8_t path_depth   = 0;
String  path_current = "/";

#define COMMAND_LENGTH 128

/****** base64_encode******/
const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
String base64_encode(const uint8_t* data, size_t length) {
  
  String encoded = "";
  size_t i = 0;
  
  while (i < length) {
    uint32_t octet_a = i < length ? (unsigned char)data[i++] : 0;
    uint32_t octet_b = i < length ? (unsigned char)data[i++] : 0;
    uint32_t octet_c = i < length ? (unsigned char)data[i++] : 0;

    uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

    encoded += base64_chars[(triple >> 3 * 6) & 0x3F];
    encoded += base64_chars[(triple >> 2 * 6) & 0x3F];
    encoded += base64_chars[(triple >> 1 * 6) & 0x3F];
    encoded += base64_chars[(triple >> 0 * 6) & 0x3F];
  }

  switch (length % 3) {
    case 1:
      encoded[encoded.length() - 2] = '=';
      encoded[encoded.length() - 1] = '=';
      break;
    case 2:
      encoded[encoded.length() - 1] = '=';
      break;
  }

  return encoded;
}

String base64_encode(const String& str, size_t length) {
  return base64_encode((const uint8_t*)str.c_str(), length);
}
/****** base64_encode******/
/***** funtion command ****/
void command_helf() {
  Serial.println("******* help *******");
  Serial.println("help  this text");
  Serial.println("ls    show list");
  Serial.println("cd    move path");
  Serial.println("md    make dir");
  Serial.println("rd    remove dir");
  Serial.println("rf    remove file");
  Serial.println("op    open file");
  Serial.println("st    open file");
  Serial.println("sw    file slice");
  Serial.println("up    upload file");
  Serial.println("fu    firmware update");
  Serial.println("sd    send command");
  Serial.println("******* help *******");
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
    if(temp_text == "/"){
      path_depth   = 0;
      path_current = "/";
    }else if(temp_text == ".." && path_depth != 0){
      String temp_path = path_current;
      char *upper_path = const_cast<char*>(temp_path.c_str());
      String dir_path  = strtok(upper_path, "/");
      if(path_depth == 1) path_current = "/";
      else path_current = "";
      for(uint8_t index=1; index<path_depth; index++){
        path_current += "/" + dir_path;
        dir_path = strtok(0x00, "/");
      }
      path_depth -= 1;
    }else if(exisits_check(path_current+"/"+temp_text)){
      path_depth += 1;
      if(path_current == "/") path_current += temp_text;
      else path_current += "/"+temp_text;
    }
    Serial.println(path_current);
  }else if(command_buf[0]=='l' && command_buf[1]=='s'){
    dir_list(path_current,true,true);
  }else if(command_buf[0]=='m' && command_buf[1]=='d'){
    dir_make(path_current+"/"+temp_text);
  }else if(command_buf[0]=='r' && command_buf[1]=='d'){
    dir_remove(path_current+"/"+temp_text);
  }else if(command_buf[0]=='r' && command_buf[1]=='f'){
    if(temp_text == "*") files_all_remove(path_current);
    else file_remove(path_current+"/"+temp_text);
  }else if(command_buf[0]=='o' && command_buf[1]=='p'){
    Serial.println(file_read(path_current+"/"+temp_text));
  }else if(command_buf[0]=='s' && command_buf[1]=='t'){
    file_stream(path_current+"/"+temp_text);
  }else if(command_buf[0]=='s' && command_buf[1]=='w'){
    firmware_slice(path_current+"/"+temp_text);
  }else if(command_buf[0]=='u' && command_buf[1]=='p'){

    File file;
    fs::FS &fs = SD;
    mesh.update();
    file = fs.open(path_current+temp_text);
    uint16_t  cmd_size = file.size();
    char      cmd_buf[cmd_size];
    uint16_t  cmd_currentSize = 0;
    mesh.update();

    Serial.print("file_name : ");
    Serial.print(temp_text);
    Serial.print(" , size : ");
    Serial.println(file.size());

    mesh.sendBroadcast("FIRMWARE");
    mesh.update();
    mesh.sendBroadcast(temp_text);
    mesh.update();
    mesh.sendBroadcast(String(file.size()));
    mesh.update();

    while (file.available()) {
      cmd_buf[cmd_currentSize++] = file.read();
      mesh.update();
    }

    file.close();
    auto base64Data = base64_encode(cmd_buf, cmd_size);
    Serial.println(base64Data);
    mesh.sendBroadcast(base64Data);
    file.close();
    Serial.println("done.");

  }else if(command_buf[0]=='f' && command_buf[1]=='u'){
    #include <Update.h>
    #define  CMD_UNIT_SIZE 1436
    uint8_t  cmd_buf[CMD_UNIT_SIZE];
    uint16_t cmd_currentSize = 0;
    uint32_t cmd_totalSize   = 0;

    if(!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
    if(temp_text == "auto"){

      String path = "/firmware/sw";
      uint16_t total_file = file_read(path+"_file.num").toInt();

      for(uint8_t index=0; index<total_file; index++){
        String slice_file = path + String(index) + ".bin";
        cmd_currentSize = file_size(slice_file);
        String file_buf = file_read(slice_file);
        cmd_totalSize  += cmd_currentSize;
        for(uint file_index=0; file_index<cmd_currentSize; file_index++){
          cmd_buf[file_index] = file_buf[file_index];
        }
        if (Update.write(cmd_buf, cmd_currentSize) != cmd_currentSize) Update.printError(Serial); //flashing firmware to ESP
        else Serial.println(total_file-index);
      }

    }else{
      File file;
      fs::FS &fs = SD;
      file = fs.open(path_current+temp_text);

      Serial.println("file_open");

      while (file.available()) {
      cmd_buf[cmd_currentSize++] = file.read();
      if(cmd_currentSize >= CMD_UNIT_SIZE){
        cmd_totalSize += cmd_currentSize;
        if (Update.write(cmd_buf, cmd_currentSize) != cmd_currentSize) Update.printError(Serial); //flashing firmware to ESP
        else Serial.println(cmd_totalSize);
        cmd_currentSize = 0;
        }
      }
      file.close();
      cmd_totalSize += cmd_currentSize;
      if (Update.write(cmd_buf, cmd_currentSize) != cmd_currentSize) Update.printError(Serial); //flashing firmware to ESP
    }

    if (Update.end(true)) { //true to set the size to the current progress
      Serial.printf("Update Success: %u\nRebooting...\n", cmd_totalSize);
      delay(1000);
      ESP.restart();
    } else {
      Update.printError(Serial);
    }
  }else if(command_buf[0]=='s' && command_buf[1]=='d'){
    mesh.sendBroadcast(temp_text);
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

  mesh.setDebugMsgTypes( ERROR | STARTUP );
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive( &receivedCallback );
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.setRoot( true );
  mesh.setContainsRoot(true);
  routeID = mesh.getNodeId();
  
  command_helf();
}

void loop() {
  unsigned long millisec = millis();
  mesh.update();
  if (Serial.available()) get_command(Serial.read());
  mesh.update();
  mesh_restart(millisec);
}

void mesh_node_list(){
  nodes = mesh.getNodeList();
  SimpleList<uint32_t>::iterator node = nodes.begin();
  Serial.print("[");
  while (node != nodes.end()) 
  {
    mesh.update();
    Serial.printf("%u,", *node);
    node++;
  }
  Serial.println("]");
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.println( String(from) + "=" + msg.c_str());
  mesh.sendBroadcast("ROOTRECV");  
}
// Needed for painless library end

void changedConnectionCallback() {
  mesh_node_list();
}

unsigned long timer_restart = 0;
uint8_t restart_count       = 0;
void mesh_restart(unsigned long millisec){
  if(millisec - timer_restart > 1000*60){
    timer_restart = millisec;
    if(restart_count++ > 240) ESP.restart();
  }
}
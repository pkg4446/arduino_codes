#include <SPI.h>
#include <SD.h>
#include "filesys.h"
#include "FS.h"
#include "painlessMesh.h"

#define   MESH_PREFIX     "smartHiveMesh"
#define   MESH_PASSWORD   "smarthive123"
#define   MESH_PORT       3333

painlessMesh   mesh;
String routeID = "";
SimpleList<uint32_t> nodes;

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
  Serial.println("up    upload file");
  Serial.println("sd    send command");
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
  }else if(command_buf[0]=='u' && command_buf[1]=='p'){

    #define   CMD_UNIT_SIZE   1436
    char      cmd_buf[CMD_UNIT_SIZE];
    uint16_t  cmd_currentSize = 0;

    File file;
    fs::FS &fs = SD;
    mesh.update();
    file = fs.open(path_current+temp_text);
    Serial.print("file_name : ");
    Serial.print(temp_text);
    Serial.print(" , size : ");
    Serial.println(file.size());
    mesh.sendBroadcast("FIRMWARE");
    mesh.sendBroadcast(String(file.size()%CMD_UNIT_SIZE));

    while (file.available()) {
      uint8_t file_contents = file.read();
      mesh.update();
      if(file_contents == 0){
        mesh.sendBroadcast(cmd_buf);
        cmd_currentSize = 0;
        Serial.println("0x00");
        mesh.sendBroadcast("0X00");
      }else if(cmd_currentSize >= CMD_UNIT_SIZE){
        cmd_currentSize = 0;
        mesh.sendBroadcast(cmd_buf);
      }else{
        cmd_buf[cmd_currentSize++] = file_contents;
      }
    }

    file.close();
    Serial.println("done.");

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
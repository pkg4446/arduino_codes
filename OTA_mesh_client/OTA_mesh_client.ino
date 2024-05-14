#include <Update.h>
#include <painlessMesh.h>

#include "base64.h"

#define   MESH_PREFIX     "smartHiveMesh"
#define   MESH_PASSWORD   "smarthive123"
#define   MESH_PORT       3333

Scheduler     taskScheduler; // to control upload task
painlessMesh  mesh;
String        nodeID    = "";

#define   CMC_MAX      16
bool      cmd_mode     = true;
bool      check_name   = true;
bool      check_len    = true;
String    file_name;
uint16_t  file_size    = 0;
uint16_t  file_index   = 0;
uint16_t  cmd_currentSize = 0;

//taskSendMessage funtion
void sendMessage() ; // Prototype so PlatformIO doesn't complain
Task firmware_check( TASK_SECOND*60*1, TASK_FOREVER, &sendMessage );
void sendMessage() {
  //mesh.sendBroadcast("VER_0001");
}

void receivedCallback( uint32_t from, String &msg ) {
  if(cmd_mode){

    Serial.print("receivedCallback ");
    if(msg == "FIRMWARE"){
      Serial.println("firmware update");
      cmd_mode   = false;
      check_name = true;
      check_len  = true; 
      //firmware_check.disable();
    }else if(msg == "ROOTRECV"){
      Serial.println("route ack");
    }else{
      Serial.println("cmd_null");
      mesh.sendBroadcast("cmd_null");
    }

  }else{
    if(check_len){
      if(check_name){
        Serial.print("file_name :");
        file_name = msg;
        Serial.println(file_name);
        check_name = false;
      }else{
        Serial.print("file_size :");
        file_size = msg.toInt();
        Serial.println(file_size);
        cmd_currentSize = 0;
        check_len       = false;
      }
    }else{
      mesh.update();
      String base64Data = base64_decode(msg);
      if(file_size != base64Data.length()) Serial.print("file error!");
      else{
        Serial.println("file_receive");
      }
      cmd_mode = true;
      //firmware_check.enable();
    }

  }
  mesh.update();
}

void setup(void) {
  Serial.begin(115200);

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &taskScheduler, MESH_PORT );
  mesh.setContainsRoot( true );

  mesh.onReceive(&receivedCallback);
  nodeID = mesh.getNodeId();

  taskScheduler.addTask(firmware_check);
  firmware_check.enable();

  Serial.print("System Online, nodeID:");
  Serial.println(nodeID);
}

void loop(void) {
  mesh.update();
}
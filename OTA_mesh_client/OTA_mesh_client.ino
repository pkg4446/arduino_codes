#include <Update.h>
#include "painlessMesh.h"

#define   MESH_PREFIX     "smartHiveMesh"
#define   MESH_PASSWORD   "smarthive123"
#define   MESH_PORT       3333

#define   CMD_UNIT_SIZE   1436

Scheduler     taskScheduler; // to control upload task
painlessMesh  mesh;
String        nodeID   = "";
bool          cmd_mode = true;

#define   CMC_MAX         16
uint32_t  cmd_totalSize   = 0;
  
//taskSendMessage funtion
void sendMessage() ; // Prototype so PlatformIO doesn't complain
Task firmware_check( TASK_SECOND*60*1, TASK_FOREVER, &sendMessage );
void sendMessage() {
  mesh.sendBroadcast("VER_0001");
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.print("receivedCallback ");
  if(cmd_mode){
    String msg_buf;
    for (int index = 0; index < msg.length(); index++) {
      msg_buf += msg[index];
    }
    if(msg_buf == "FIRMWARE"){
      Serial.println("firmware update");
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)){
        Update.printError(Serial);//start with max available size
      }else{
        cmd_mode = false;
      }
    }
  }else{
    if(msg.length() == 8){
      String msg_buf;
      for (int index = 0; index < msg.length(); index++) {
        msg_buf += msg[index];
      }
      Serial.println(msg_buf);
      if(msg_buf == "FIRMWARE"){
        cmd_mode = true;
        Serial.println("firmware update finish");
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.print("Update Success:");Serial.println(cmd_totalSize);
          mesh.sendBroadcast("FIRM_TRU;");
          delay(100);
          ESP.restart();
        } else {
          mesh.sendBroadcast("FIRM_ERR;");
        }
      }
    }
    uint8_t cmd_buf[msg.length()];
    Serial.println("cmd_currentSize:");
    Serial.println(msg.length());
    uint16_t  cmd_currentSize = 0; // size of data currently in buf
    for (int index = 0; index < msg.length(); index++) {
      cmd_buf[cmd_currentSize++] = msg[index];
    }
    cmd_totalSize += cmd_currentSize;
    if (Update.write(cmd_buf, cmd_currentSize) != cmd_currentSize) Update.printError(Serial);
  }
}

void setup(void) {
  Serial.begin(115200);
  

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &taskScheduler, MESH_PORT );
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
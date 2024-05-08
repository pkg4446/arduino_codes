#include <Update.h>
#include "painlessMesh.h"

#define   MESH_PREFIX     "smartHiveMesh"
#define   MESH_PASSWORD   "smarthive123"
#define   MESH_PORT       3333

#define   CMD_UNIT_SIZE   1436

Scheduler     taskScheduler; // to control upload task
painlessMesh  mesh;
String        nodeID    = "";

#define   CMC_MAX   16
bool      cmd_mode  = true;
bool      check_len = true;
uint32_t  file_size = 0;
uint8_t   cmd_buf[CMD_UNIT_SIZE];
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
    }else if(msg_buf == "ROOTRECV"){
      Serial.println("route ack");
    }else{
      Serial.println("cmd_null");
      mesh.sendBroadcast("cmd_null");
    }

  }else{

    if(check_len){

      String totlaSize = "";
      for (int index = 0; index < msg.length(); index++) {
        totlaSize += msg[index];
      }
      file_size = totlaSize.toInt();
      check_len = false;

    }else{

      uint16_t current_len = msg.length();
      mesh.update();
      if(msg.length() == 4 && msg[0] == 30 && msg[1] == 88 && msg[2] == 30 && msg[3] == 30)
      {
        Serial.println("0x00");
        cmd_buf[cmd_currentSize++] += 0;
      }
      for (int index = 0; index < current_len; index++) {
        cmd_buf[cmd_currentSize++] += msg[index];
      }
      
      
      if(cmd_currentSize >= CMD_UNIT_SIZE){
        if (Update.write(cmd_buf, cmd_currentSize) != cmd_currentSize) Update.printError(Serial); //flashing firmware to ESP
        cmd_currentSize = 0;
      }
      
      if(--file_size == 0){
        if (Update.write(cmd_buf, cmd_currentSize) != cmd_currentSize) Update.printError(Serial); //flashing firmware to ESP
        Serial.println("firmware update finish");
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.print("Update Success");
          mesh.sendBroadcast("FIRM_TRU;");
          delay(100);
          ESP.restart();
        } else {
          mesh.sendBroadcast("FIRM_ERR;");
        }
        cmd_mode = true;
      }

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
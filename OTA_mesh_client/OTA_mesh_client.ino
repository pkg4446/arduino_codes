#include <Update.h>
#include <painlessMesh.h>

#define   MESH_PREFIX     "smartHiveMesh"
#define   MESH_PASSWORD   "smarthive123"
#define   MESH_PORT       3333
const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

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

/****** base64_decode******/
String base64_decode(const String& encoded) {
  const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  String decoded = "";

  size_t in_len = encoded.length();
  size_t i = 0;
  size_t j = 0;
  int32_t sextet_a = 0;
  int32_t sextet_b = 0;
  int32_t sextet_c = 0;
  int32_t sextet_d = 0;
  while (in_len-- && (encoded[j] != '=') && is_base64(encoded[j])) {
    sextet_a = encoded[j++];
    sextet_b = encoded[j++];
    sextet_c = encoded[j++];
    sextet_d = encoded[j++];

    uint32_t triple = (sextet_a << 3 * 6)
                    + (sextet_b << 2 * 6)
                    + (sextet_c << 1 * 6)
                    + (sextet_d << 0 * 6);

    decoded += (triple >> 2 * 8) & 0xFF;
    decoded += (triple >> 1 * 8) & 0xFF;
    decoded += (triple >> 0 * 8) & 0xFF;
  }

  return decoded;
}

bool is_base64(char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}
/****** base64_decode******/

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
      cmd_mode   = false;
      check_name = true;
      check_len  = true; 
      /*
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)){
        Update.printError(Serial);//start with max available size
      }else{
        cmd_mode   = false;
        check_name = true;
        check_len  = true; 
      }
      */
    }else if(msg_buf == "ROOTRECV"){
      Serial.println("route ack");
    }else{
      Serial.println("cmd_null");
      mesh.sendBroadcast("cmd_null");
    }

  }else{
    if(check_len){
      if(check_name){
        Serial.print("file_name :");
        file_name = "";
        for (int index = 0; index < msg.length(); index++) {
          file_name += msg[index];
        }
        Serial.println(file_name);
        check_name = false;
      }else{
        Serial.print("file_size :");
        String totlaSize = "";
        for (int index = 0; index < msg.length(); index++) {
          totlaSize += msg[index];
        }
        file_size = totlaSize.toInt();
        Serial.println(file_size);
        cmd_currentSize = 0;
        check_len       = false;
      }
    }else{

      uint16_t current_len = msg.length();
      mesh.update();
      String firmware_file = "";
      for (int index = 0; index < current_len; index++) {
        mesh.update();
        firmware_file += msg[index];
      }
      mesh.update();
      String base64Data = base64_decode(firmware_file);
      Serial.println(base64Data);
      Serial.println(base64Data.length());
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
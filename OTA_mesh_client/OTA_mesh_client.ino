#include <Update.h>
#include <painlessMesh.h>

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

/****** base64_decode******/
static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";
   
bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

String base64_decode(const String& encoded_string) {
    size_t in_len = encoded_string.length();
    size_t i = 0;
    size_t j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    String ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
      mesh.update();
      char_array_4[i++] = encoded_string[in_];
      in_++;
      if (i == 4) {
        for (i = 0; i < 4; i++) {
          mesh.update();
          char_array_4[i] = static_cast<unsigned char>(strchr(base64_chars, char_array_4[i]) - base64_chars);
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (i = 0; i < 3; i++) {
          mesh.update();
          ret += char(char_array_3[i]);
        }
        i = 0;
      }
    }

    if (i) {
        for (j = i; j < 4; j++) {
            char_array_4[j] = 0;
        }

        for (j = 0; j < 4; j++) {
            char_array_4[j] = static_cast<unsigned char>(strchr(base64_chars, char_array_4[j]) - base64_chars);
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (j = 0; j < i - 1; j++) {
            ret += char(char_array_3[j]);
        }
    }

    return ret;
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
      if(file_size != base64Data.length()) Serial.print("file error!");
      else{
        Serial.print("file_receive");
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
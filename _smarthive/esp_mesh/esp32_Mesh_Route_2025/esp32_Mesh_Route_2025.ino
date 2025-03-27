#include  "painlessMesh.h"
#define   SERIAL_MAX  128

#define   SEGMENT 8
#define   QUE_MAX 9

#define   MESH_PREFIX     "HiveMesh"
#define   MESH_PASSWORD   "smarthive123"
#define   MESH_PORT       3333

HardwareSerial rootDvice(2);
painlessMesh   mesh;

const uint8_t led_pin[8] = {12,13,14,27,17,25,26,16}; //A,B,C,D,E,F,G,DP
const uint8_t led_sw[2]  = {18,19};
const uint8_t relay[2]   = {4,22};
uint8_t segment_number   = 0;
bool segment_state[SEGMENT]    = {true,true,true,true,true,true,true,true};
bool segment_change      = false;
bool segment_cal         = false;
uint8_t segment_interval = 9;

unsigned long segment_update  = 0UL;

String routeID = "";
SimpleList<uint32_t> nodes;

void segment_display(unsigned long millisec){
  if(millisec - segment_update> segment_interval){
    segment_update = millisec;
    if(segment_cal){
      const bool segment[10][SEGMENT] = {
        {false,false,false,false,false,false,true,true},
        {true,false,false,true,true,true,true,true},
        {false,false,true,false,false,true,false,true},
        {false,false,false,false,true,true,false,true},
        {true,false,false,true,true,false,false,true},
        {false,true,false,false,true,false,false,true},
        {false,true,false,false,false,false,false,true},
        {false,false,false,true,true,false,true,true},
        {false,false,false,false,false,false,false,true},
        {false,false,false,true,true,false,false,true},
      };
      uint8_t one_ten = 0;
      mesh.update();
      if(segment_change){
        one_ten = segment_number/10;
      }else{
        one_ten = segment_number%10;
      }
      for(uint8_t index=0; index<SEGMENT; index++){        
        if(segment_state[index] != segment[one_ten][index]){
          segment_state[index]   = segment[one_ten][index];
          digitalWrite(led_pin[index], segment_state[index]);
          mesh.update();
        }
      }
      segment_change = !segment_change;
    }
    mesh.update();
    if(segment_cal){
      digitalWrite(led_sw[segment_change], true);
      segment_interval = 9;
    }else{
      digitalWrite(led_sw[segment_change], false);
      segment_interval = 1;
    }
    segment_cal = !segment_cal;
  }
}////segment_display end

//// ----------- Command  -----------
char    command_Buf[SERIAL_MAX];
int16_t command_Num;
////--------------------- String_slice ----------------////
String String_slice(uint8_t *check_index, String text, char check_char){
  String response = "";
  for(uint8_t index_check=*check_index; index_check<text.length(); index_check++){
    if(text[index_check] == check_char || text[index_check] == 0x00){
      *check_index = index_check+1;
      break;
    }
    response += text[index_check];
  }
  return response;
}
////--------------------- String_slice ----------------////
void command_Service() {
  if(command_Buf[0] != '\n'){
    mesh.sendBroadcast(command_Buf);
  }
}//Command_service() END

//replace by pointer
void command_Process(char ch) {
  Serial.print(ch);
  mesh.update();
  if (ch == '\n') {
    command_Buf[command_Num] = 0x00;
    command_Service();
    command_Num = 0;
  }else if (ch != '\r'){
    command_Buf[command_Num++] = ch;
    command_Num %= SERIAL_MAX;
  }
}

void mesh_node_list(){
  nodes = mesh.getNodeList();
  SimpleList<uint32_t>::iterator node = nodes.begin();
  Serial.print("[");
  segment_number = 0;
  while (node != nodes.end()) 
  {
    mesh.update();
    Serial.printf("%u,", *node);
    node++;
    segment_number++;
  }
  Serial.println("]");
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  char msg_buf[SERIAL_MAX];
  for (int index = 0; index < msg.length(); index++) {
    msg_buf[index] = msg[index];
  }
  Serial.println(msg.c_str());

  uint8_t cmd_index = 0;
  String devicd_id  = String_slice(&cmd_index, msg_buf, 0x20);
  
  if(devicd_id == String(from)){
    String post = msg.c_str();
    post += "\n";
    rootDvice.print(post);
  }else{
    Serial.print(devicd_id);
    Serial.print(" != ");
    Serial.println(from);
  }
}
// Needed for painless library end

void changedConnectionCallback() {
  mesh_node_list();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);  
  rootDvice.begin(115200, SERIAL_8N1, 33, 32);
  //mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );
  mesh.setDebugMsgTypes( ERROR | STARTUP );
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.setRoot( true );
  mesh.setContainsRoot( true );

  mesh.onReceive( &receivedCallback );
  mesh.onChangedConnections(&changedConnectionCallback);

  routeID = mesh.getNodeId();

  for(uint8_t index=0; index<8; index++){
    pinMode(led_pin[index], OUTPUT);
    digitalWrite(led_pin[index], true);
  }
  for(uint8_t index=0; index<2; index++){
    pinMode(led_sw[index], OUTPUT);
    pinMode(relay[index], OUTPUT);
    digitalWrite(led_sw[index], true);
    digitalWrite(relay[index], false);
  }
  Serial.println("ver 0.0.1");
}

void loop() {
  unsigned long millisec = millis();
  if (rootDvice.available()) {
    command_Process(rootDvice.read());
  }
  if (Serial.available()) {
    command_Process(Serial.read());
  }
  mesh.update();
  mesh_restart(millisec);
  segment_display(millisec);
}

unsigned long timer_restart = 0;
uint8_t restart_count       = 0;
void mesh_restart(unsigned long millisec){
  if(millisec - timer_restart > 1000*60){
    timer_restart = millisec;
    if(restart_count++ > 240) ESP.restart();
  }
}
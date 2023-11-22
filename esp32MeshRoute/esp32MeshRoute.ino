#include  "painlessMesh.h"
#define   SERIAL_MAX  128

#define   SEGMENT 8
#define   QUE_MAX 9

#define   MESH_PREFIX     "smartHiveMesh"
#define   MESH_PASSWORD   "smarthive123"
#define   MESH_PORT       3333

HardwareSerial rootDvice(2);
painlessMesh   mesh;

const uint8_t led_pin[8] = {12,13,14,16,17,25,26,27};
const uint8_t led_sw[2]  = {32,33};
const uint8_t relay[2]   = {22,23};
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

void command_Service() {
  if(command_Buf[0] != ';'){
    Serial.println(command_Buf);
    mesh.sendBroadcast(command_Buf);
  }
}//Command_service() END

//replace by pointer
void command_Process() {
  char ch;
  ch = rootDvice.read();
  mesh.update();
  switch (ch) {
    case ';':
      command_Buf[command_Num] = ';';
      command_Buf[command_Num+1] = 0x00;
      command_Service();
      command_Num = 0;
      break;
    default:
      command_Buf[command_Num++] = ch;
      command_Num %= SERIAL_MAX;
      break;
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
  rootDvice.print( String(from) + "=" + msg.c_str());
  Serial.println( String(from) + "=" + msg.c_str());
}
// Needed for painless library end

void changedConnectionCallback() {
  mesh_node_list();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);  
  rootDvice.begin(115200, SERIAL_8N1, 18, 19);
  //mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );
  mesh.setDebugMsgTypes( ERROR | STARTUP );
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive( &receivedCallback );
  mesh.onChangedConnections(&changedConnectionCallback);

  mesh.setRoot( true );
  mesh.setContainsRoot( true );
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
  Serial.println("ver 1.0.0");
}

//unsigned long retime = 0UL;

void loop() {
  unsigned long millisec = millis();
  if (rootDvice.available()) {
    command_Process();
  }
  if (Serial.available()) {
    Serial_process();
  }
  mesh.update();
  mesh_restart(millisec);
  segment_display(millisec);
}

void Serial_process() {
  char ch;
  ch = Serial.read();
  mesh.update();
  switch (ch) {
    case ';':
      command_Buf[command_Num] = ';';
      command_Buf[command_Num+1] = 0x00;
      command_Service();
      command_Num = 0;
      break;
    default:
      command_Buf[command_Num++] = ch;
      command_Num %= SERIAL_MAX;
      break;
  }
}

unsigned long timer_restart = 0;
uint8_t restart_count       = 0;
void mesh_restart(unsigned long millisec){
  if(millisec - timer_restart > 1000*60*60){
    timer_restart = millisec;
    if(restart_count++ > 240) ESP.restart();
  }
}
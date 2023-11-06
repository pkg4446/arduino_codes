#include  "painlessMesh.h"
#define   SERIAL_MAX  128

#define   MESH_PREFIX     "smartHiveMesh"
#define   MESH_PASSWORD   "smarthive123"
#define   MESH_PORT       3333

HardwareSerial rootDvice(2);
painlessMesh  mesh;

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

String routeID = "";
// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  rootDvice.print( String(from) + "=" + msg.c_str());
  Serial.println( String(from) + "=" + msg.c_str());
}
// Needed for painless library end

SimpleList<uint32_t> nodes;
void changedConnectionCallback() {
  nodes = mesh.getNodeList();
  SimpleList<uint32_t>::iterator node = nodes.begin();
  Serial.print("[");
  while (node != nodes.end()) 
  {
    Serial.printf("%u,", *node);
    node++;
  }
  Serial.println("]");
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
}

//unsigned long retime = 0UL;

void loop() {
  mesh.update();
  if (rootDvice.available()) {
    command_Process();
  }
  if (Serial.available()) {
    Serial_process();
  }
  mesh_restart(millis());
/*
  unsigned long update = millis();
  if(update - retime > 1000){
    retime = update;
    Serial.println("Test");
  }
*/
}

void Serial_process() {
  char ch;
  ch = Serial.read();
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
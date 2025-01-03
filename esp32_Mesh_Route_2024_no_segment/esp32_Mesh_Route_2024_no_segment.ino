#include  "painlessMesh.h"
#define   SERIAL_MAX  128

#define   SEGMENT 8
#define   QUE_MAX 9

#define   MESH_PREFIX     "HiveMesh"
#define   MESH_PASSWORD   "smarthive123"
#define   MESH_PORT       3333

HardwareSerial rootDvice(2);
painlessMesh   mesh;

String routeID = "";
SimpleList<uint32_t> nodes;

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
void command_Process(char ch) {
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
  char msg_buf[SERIAL_MAX];
  for (int index = 0; index < msg.length(); index++) {
    msg_buf[index] = msg[index];
  }
  String devicd_id = strtok(msg_buf, "=");
  if(devicd_id == String(from)){
    rootDvice.print(msg.c_str());
    Serial.println(msg.c_str());
    String res = "S=" + String(from) + "=AT+RES=1;";
    mesh.sendBroadcast(res);
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

  Serial.println("ver 1.1.0");
}

//unsigned long retime = 0UL;

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
}

unsigned long timer_restart = 0;
uint8_t restart_count       = 0;
void mesh_restart(unsigned long millisec){
  if(millisec - timer_restart > 1000*60){
    timer_restart = millisec;
    if(restart_count++ > 240) ESP.restart();
  }
}
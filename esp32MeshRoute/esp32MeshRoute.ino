#include  "painlessMesh.h"
#define   SERIAL_MAX  64

#define   MESH_PREFIX     "smartHiveMesh"
#define   MESH_PASSWORD   "smarthive123"
#define   MESH_PORT       3333

HardwareSerial rootDvice(2);
painlessMesh  mesh;

//// ----------- Command  -----------
char    command_Buf[SERIAL_MAX];
int16_t command_Num;

void command_Service() {
  Serial.printf(command_Buf);
  mesh.sendBroadcast( command_Buf );
}//Command_service() END

void command_Process() {
  char ch;
  ch = rootDvice.read();
  switch (ch) {
    case ';':
      command_Buf[command_Num] = ';';
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.printf("MeshRoot");
  
  rootDvice.begin(115200, SERIAL_8N1, 22, 21);
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive( &receivedCallback );

  mesh.setRoot( true );
  mesh.setContainsRoot( true );
  routeID = mesh.getNodeId();
}

void loop() {
  mesh.update();
  if (rootDvice.available()) {
    command_Process();
  }
  if (Serial.available()) {
    Serial_process();
  }
}

void Serial_process() {
  char ch;
  ch = Serial.read();
  switch (ch) {
    case ';':
      command_Buf[command_Num] = ';';
      command_Service();
      command_Num = 0;
      break;
    default:
      command_Buf[command_Num++] = ch;
      command_Num %= SERIAL_MAX;
      break;
  }
}

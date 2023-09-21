#include  "painlessMesh.h"
#include  "Adafruit_SHT31.h"
#include  "EEPROM.h"
#define   EEPROM_SIZE 2
#define   SERIAL_MAX  64

#define   MESH_PREFIX     "smartHiveMesh"
#define   MESH_PASSWORD   "smarthive123"
#define   MESH_PORT       3333

Scheduler taskScheduler; // to control upload task
painlessMesh  mesh;
Adafruit_SHT31 sht31 = Adafruit_SHT31();

String  nodeID = "";
//// ----------- Variable -----------
uint8_t TIME_CHECK = 0;
////for millis() func//
unsigned long time_Water  = 0;
//// ------------ EEPROM ------------
const uint8_t MAX_PUMP    = 1;
//// ------------- PIN --------------
const uint8_t RELAY_PUMP[3] = {25, 26, 27};
//// ----------- Variable -----------
uint8_t MAX_TIME = 30;

//// ----------- Command  -----------
void command_Service(String command, String value) {
  if (command == "AT+PUMP") {
    uint8_t number = value.toInt();
    pump_Water(number);
  } else if (command == "AT+TIME") {
    MAX_TIME = value.toInt();
    EEPROM.write(MAX_PUMP, MAX_TIME);
    EEPROM.commit();
    Serial.print("MAX_TIME : ");
    Serial.println(MAX_TIME);
    mesh.sendBroadcast("PUMP=SET=TIME;");
  }
}//Command_service() END

//// ----------- TEST  -----------
void AT_commandHelp() {
  Serial.println("---------------- AT command help ----------------");
  Serial.println(";AT+PUMP = int;      Pump Run.[int=0:OFF,1~3:PUMP]");
  Serial.println(";AT+TIME = int;      Pump Runtime Change.[PUMP OFF]");
}
char Serial_buf[SERIAL_MAX];
int8_t Serial_num;
void Serial_service() {
  String str1 = strtok(Serial_buf, "=");
  String str2 = strtok(NULL, " ");
  command_Service(str1,str2);
}
void Serial_process() {
  char ch;
  ch = Serial.read();
  switch ( ch ) {
    case ';':
      Serial_buf[Serial_num] = NULL;
      Serial.print("ehco : ");
      Serial.println(Serial_buf);
      Serial_service();
      Serial_num = 0;
      break;
    default :
      Serial_buf[ Serial_num ++ ] = ch;
      Serial_num %= SERIAL_MAX;
      break;
  }
}
//// ----------- TEST  -----------

//taskSendMessage funtion
void sendMessage() ; // Prototype so PlatformIO doesn't complain
Task pumpLog( TASK_SECOND * 60, TASK_FOREVER, &sensorValue );
void sensorValue() {
  mesh.sendBroadcast("PUMP=NET=" + String(MAX_TIME) + ";");
}
//taskSendMessage funtion end


// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  char msg_buf[SERIAL_MAX];
  for (int i = 0; i < msg.length(); i++) {
    msg_buf[i] = msg[i];
  }
  String types   = strtok(msg_buf, "=");
  if (types == "P") {
    String device  = strtok(NULL, "=");
    //if (device == nodeID) {}
    Serial.println(msg);
    String command = strtok(NULL, "=");
    String value   = strtok(NULL, ";");
    command_Service(command, value);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //// ------------ PIN OUT ------------
  pinMode(RELAY_PUMP[0], OUTPUT);
  pinMode(RELAY_PUMP[1], OUTPUT);
  pinMode(RELAY_PUMP[2], OUTPUT);
  //// ------------ PIN OUT ------------
  if (!EEPROM.begin(EEPROM_SIZE)) Serial.println("failed to initialise EEPROM");
  //// ------------ EEPROM ------------
  MAX_TIME = byte(EEPROM.read(MAX_PUMP));
  //// ------------ EEPROM ------------
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &taskScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);

  nodeID = mesh.getNodeId();

  taskScheduler.addTask( pumpLog );
  pumpLog.enable();

  Serial.print("MAX_TIME : ");
  Serial.println(MAX_TIME);
  Serial.println("System online.");
  AT_commandHelp();
}

void loop() {
  mesh.update();
  if (Serial.available()) {
    Serial_process();
  }
}

void pump_Water(uint8_t pumpNumber) {
  if (pumpNumber == 0) {
    digitalWrite(RELAY_PUMP[0], false);
    digitalWrite(RELAY_PUMP[1], false);
    digitalWrite(RELAY_PUMP[2], false);
    Serial.println("Pump cut off");
  } else if (pumpNumber < 4) {
    digitalWrite(RELAY_PUMP[pumpNumber - 1], true);
    TIME_CHECK = MAX_TIME;
    Serial.print(pumpNumber);
    Serial.println(" Pump run");
  } else {
    Serial.println("error");
  }
}//pump_Water() END

void max_times() {
  if ((millis() - time_Water) > 1000 * 60) {
    time_Water = millis();
    if (TIME_CHECK > 1) {
      TIME_CHECK--;
    } else if (TIME_CHECK == 1) {
      digitalWrite(RELAY_PUMP[0], false);
      digitalWrite(RELAY_PUMP[1], false);
      digitalWrite(RELAY_PUMP[2], false);
      TIME_CHECK--;
    }
  }//millis()
}//stable() END

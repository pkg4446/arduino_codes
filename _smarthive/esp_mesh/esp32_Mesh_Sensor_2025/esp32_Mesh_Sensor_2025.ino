#include  <Arduino.h>
#include  <painlessMesh.h>
#include  <EEPROM.h>
#include  <Wire.h>
#include  <FS.h>

#define EEPROM_SIZE 2
#define SERIAL_MAX  128

#define MESH_PREFIX   "HiveMesh"
#define MESH_PASSWORD "smarthive123"
#define MESH_PORT     3333

#define SDA2 33
#define SCL2 32
TwoWire Wire2 = TwoWire(1);

//#define SENSOR_SHT40
#ifdef SENSOR_SHT40
  #include "Adafruit_SHT4x.h"
  Adafruit_SHT4x sht40 = Adafruit_SHT4x();
#else
  #include  <Adafruit_SHT31.h>
  Adafruit_SHT31 sht31_1  = Adafruit_SHT31();
  Adafruit_SHT31 sht31_2  = Adafruit_SHT31(&Wire2);
#endif

const boolean pin_on  = true;
const boolean pin_off = false;

bool mesh_send = true;

uint8_t sht_port      = 0;

Scheduler taskScheduler; // to control upload task
painlessMesh  mesh;
String nodeID = "";

SimpleList<uint32_t> nodes;

//// ----------- Flage --------------
boolean use_stable  = false;
boolean run_heater  = false;
//// ----------- Variable -----------
//// ----------- Sensor -------------
float temperature   = 0.00f;
float humidity      = 0.00f;
//// ----------- heater work --------
uint16_t work_total = 0;
uint16_t work_heat  = 0;
////for millis() func//
unsigned long timer_SHT     = 0UL;
unsigned long time_stayble  = 0UL;
unsigned long time_send     = 0UL;
//// ------------ EEPROM ------------
const uint8_t EEP_temp      = 0;
const uint8_t EEP_stable    = 1;
//// ------------ EEPROM Variable ---
uint8_t control_temperature = 33;
//// ------------- PIN --------------
const uint8_t RELAY_HEATER  = 12;
//// ----------- Command  -----------
char    Serial_buf[SERIAL_MAX];
int8_t  Serial_num;
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
////--------------------- service help ----------------////
void help() {
  Serial.println("************** help **************");
  Serial.println("run     * homeothermy mode, on/off");
  Serial.println("set     * temperature goal setup, 0~4,5 is all . ex) set 0 25");
  Serial.println("config  * read temperature setup");
  Serial.println("show    * seonsor value show");
  Serial.println("reboot  * system reboot");
  Serial.println("**********************************");
}
////--------------------- service serial --------------////
void read_config(){
  Serial.print("Temperature goal: ");
  Serial.print(control_temperature);
  Serial.print(", Set Operation: ");
  Serial.println(use_stable);
}
void serial_monit(){
  Serial.print("TCA Port");
  Serial.print(sht_port);
  Serial.print(", T: ");
  Serial.print(temperature);
  Serial.print("°C ,H: ");
  Serial.print(humidity);
  Serial.println("%");
  mesh.update();
  Serial.print("USE_heater = ");
  Serial.println(use_stable);
  Serial.print(", heater");
  Serial.print(run_heater);
  Serial.println(";");
  mesh.update();
}
////--------------------- service serial --------------////
void command_Service(String command, String value) {
  if (command == "ACK") {
    mesh_send = false;
  } else if (command == "run") {
    if (value == "on"){use_stable = 1;}
    else{use_stable = 0;}
    EEPROM.write(EEP_stable, use_stable);
    EEPROM.commit();
    mesh.sendBroadcast(nodeID+" post hive run " + String(use_stable));
  } else if (command == "set") {
    control_temperature = value.toInt();
    EEPROM.write(EEP_temp, control_temperature);
    EEPROM.commit();
    mesh.sendBroadcast(nodeID+" post hive temp " + String(control_temperature));
  } else if (command == "config") {
    read_config();
  } else if (command == "show") {
    serial_monit();
  } else if (command == "reboot") {
    ESP.restart();
  } else if (command == "cnt") {
    mesh.sendBroadcast(nodeID+" post hive cnt 1");
  } else {
    help();
  }
  mesh.update();
}//Command_service() END
////--------------------- service serial --------------////
void command_parser(String cmd) {
  Serial.println(cmd);
  uint8_t cmd_index = 0;
  String command  = String_slice(&cmd_index, cmd, 0x20);
  String value    = String_slice(&cmd_index, cmd, 0x20);
  command_Service(command, value);
}
void Serial_process(char ch) {
  ch = Serial.read();
  Serial.print("input");
  Serial.println(ch);
  mesh.update();
  if (ch == '\n') {
    Serial_buf[Serial_num] = 0x00;
    command_parser(Serial_buf);
    Serial_num = 0;
  }else if (ch != '\r' && (Serial_num < SERIAL_MAX - 1)){
    Serial_buf[ Serial_num ++ ] = ch;
  }
}
////--------------------- service serial --------------////
////--------------------- wifi mesh -------------------////
// Needed for painless library
uint8_t mesh_node_list(){
  nodes = mesh.getNodeList();
  SimpleList<uint32_t>::iterator node = nodes.begin();
  uint8_t node_number = 0;
  while (node != nodes.end()) 
  {
    mesh.update();
    node++;
    node_number++;
  }
  return node_number;
}
//taskSendMessage funtion
Task sensorLog( TASK_SECOND*3*5, TASK_FOREVER, &sensorValue );
void sensorValue() {
  mesh_send = true;
  Serial.println("send check");
}
void data_post(unsigned long millisec){
  //매쉬 확인
  if(millisec - time_send > 10000 && mesh_send){
    time_send = millisec;
    if(mesh_node_list() > 0){
      Serial.println("send log");
      String msg = nodeID+" post hive log {\"temp\":\"" + (String)temperature;
      msg += "\",\"humi\":\"" + (String)humidity;
      msg += "\",\"work\":"   + (String)work_heat;
      msg += ",\"runt\":"     + (String)work_total + '}';
      mesh.sendBroadcast( msg );
    }
  }
}
void receivedCallback( uint32_t from, String &msg ) {
  char msg_buf[SERIAL_MAX];
  char device[12];
  uint8_t dvc_index = 0;
  bool device_check = false;
  for (int index = 0; index < msg.length(); index++) {
    if (device_check){
      msg_buf[index-dvc_index] = msg[index];
    }else if(msg[index] != 0x20){
      device[index] = msg[index];
    }else{
      device[index] = 0x00;
      device_check = true;
      dvc_index = index+1;
    }
  }
  msg_buf[msg.length()-dvc_index] = 0x00;
  if (String(device) == nodeID) {command_parser(msg_buf);}
}
////--------------------- wifi mesh -------------------////
////--------------------- set up ----------------------////
void setup() {
  // put your setup code here, to run once:
  //// ------------ PIN OUT ------------
  pinMode(RELAY_HEATER, OUTPUT);
  digitalWrite(RELAY_HEATER,  pin_off);
  //// ------------ PIN OUT ------------
  Serial.begin(115200);
  Wire.begin();
  Wire2.begin(SDA2,SCL2,400000);
  //// ------------ EEPROM ------------
  if(!EEPROM.begin(EEPROM_SIZE)) Serial.println("failed to initialise EEPROM");
  if(EEPROM.read(EEP_temp) == 255){EEPROM.write(EEP_temp, 3);EEPROM.commit();}
  if(EEPROM.read(EEP_stable) == 255){EEPROM.write(EEP_stable, 0);EEPROM.commit();}
  control_temperature = byte(EEPROM.read(EEP_temp));
  if(EEPROM.read(EEP_stable) != 0){use_stable = true;}
  //// ------------ EEPROM ------------
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &taskScheduler, MESH_PORT );
  mesh.setContainsRoot( true );
  mesh.onReceive(&receivedCallback);
  taskScheduler.addTask( sensorLog );
  sensorLog.enable();

  nodeID = mesh.getNodeId();

  read_config();
  Serial.print("Device nodeID = ");
  if(nodeID != String(mesh.getNodeId())){
    Serial.println("error. system reboot");
    ESP.restart();
  }
  Serial.println(nodeID);
  Serial.println( "System online. ver 0.0.1");
}

void loop() {
  unsigned long now = millis();
  if (Serial.available()) Serial_process(Serial.read());
  mesh.update();
  sensor_get(now);
  mesh.update();
  stable(now);
  mesh.update();
  mesh_restart(now);
  data_post(now);
}

void stable(unsigned long millisec) {
  if ((millisec - time_stayble) > 1000 * 1) {
    time_stayble = millisec;
    run_heater = isnan(temperature) && use_stable && (temperature < control_temperature);
    work_total += 1;
    mesh.update();
    if(run_heater) {
      digitalWrite(RELAY_HEATER, pin_on);
      work_heat += 1;
    }else{
      digitalWrite(RELAY_HEATER, pin_off);
    }
  }//millis()
}//stable() END

void sensor_get(unsigned long millisec) {
  if ((millisec - timer_SHT) > 500) {
    timer_SHT = millisec;
    bool sensor_conn = false;
    #ifdef SENSOR_SHT40
      if (sht40.begin()) {
        sht_port = 1;
        sensors_event_t humi, temp;
        sht40.getEvent(&humi, &temp);
        temperature  = temp.temperature;
        humidity     = humi.relative_humidity;
        sensor_conn  = true;
      }else if (sht40.begin($Wire2)) {
        sht_port = 2;
        sensors_event_t humi, temp;
        sht40.getEvent(&humi, &temp);
        temperature  = temp.temperature;
        humidity     = humi.relative_humidity;
        sensor_conn  = true;
      }
    #else
      if (sht31_1.begin(0x44)) {
        sensor_conn = true;
        sht_port = 1;
        temperature = sht31_1.readTemperature();
        humidity    = sht31_1.readHumidity();
        sht31_1.reset();
      }else if (sht31_2.begin(0x44)) {
        sensor_conn = true;
        sht_port = 2;
        temperature = sht31_2.readTemperature();
        humidity    = sht31_2.readHumidity();
        sht31_2.reset();
      }
    #endif
    if(!sensor_conn){
      temperature  = NAN;
      humidity     = NAN;
    }
  }//if
}

unsigned long timer_restart = 0;
uint8_t restart_count       = 0;
void mesh_restart(unsigned long millisec){
  if(millisec - timer_restart > 1000*60){
    timer_restart = millisec;
    if(restart_count++ > 120){
      data_post(millis()+1000*60*10);
      ESP.restart();
    }
  }
}
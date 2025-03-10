#include  <Arduino.h>
#include  <painlessMesh.h>
#include  <EEPROM.h>
#include  <Wire.h>
#include  <FS.h>
#include  <SD.h>
#include  <SPI.h>
#include  <DS3231.h>

#define EEPROM_SIZE 5
#define SERIAL_MAX  128

#define MESH_PREFIX   "HiveMesh"
#define MESH_PASSWORD "smarthive123"
#define MESH_PORT     3333

#define TCAADDR 0x70
#define SDA2 33
#define SCL2 32
TwoWire I2Ctwo = TwoWire(1);

#define SENSOR_SHT40
#ifdef SENSOR_SHT40
  #include "Adafruit_SHT4x.h"
  Adafruit_SHT4x sht40 = Adafruit_SHT4x();
#else
  #include  <Adafruit_SHT31.h>
  Adafruit_SHT31 sht31  = Adafruit_SHT31();
#endif
DS3231 RTC_DS3231(I2Ctwo);

const boolean pin_on  = false;
const boolean pin_off = true;

bool mesh_info = false;
bool mesh_send = false;

uint8_t sht_port = 0;

void tca_select(uint8_t index) {
  if (index > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << index);
  Wire.endTransmission();
}

Scheduler taskScheduler; // to control upload task
painlessMesh  mesh;
String ERR_Message = "SENSOR=COMMEND=VALUE1=VALUE2=VALUE3=VALUE4;";

String nodeID = "";
SimpleList<uint32_t> nodes;

const uint8_t tempGap = 1;
//// ----------- Flage --------------
boolean connect_check = false;

boolean use_stable_h = false;
boolean use_stable_f = false;

boolean run_heater = false;
boolean run_fan    = false;
boolean run_log    = false;
//// ----------- Variable -----------
//// ----------- Sensor -----------
int16_t temperature = 14040;
int16_t humidity    = 14040;
////for millis() func//
unsigned long timer_SHT     = 0UL;
unsigned long time_stayble  = 0UL;
unsigned long time_send     = 0UL;
//// ------------ EEPROM ------------
const uint8_t EEP_temperature = 1;
const uint8_t EEP_humidity    = 2;
const uint8_t EEP_Stable_h    = 3;
const uint8_t EEP_Stable_f    = 4;
//// ------------ EEPROM Variable ---
uint8_t control_temperature = 33;
uint8_t control_humidity    = 50;
//// ------------- PIN --------------
const uint8_t RELAY_HEATER  = 12;
const uint8_t RELAY_FAN     = 16;
const uint8_t SENSOR_H[2] = {35,34};
const uint8_t SENSOR_W[2] = {17,36};

//// ----------- Command  -----------
void command_Service(String command, String value) {
  if (command == "AT+RES") {
    if(mesh_info) mesh_send = false;
  } else if (command == "AT+HELP") {
    AT_commandHelp();
  } else if (command == "AT+TEMP") {
    control_temperature = value.toInt();
    EEPROM.write(EEP_temperature, control_temperature);
    mesh.sendBroadcast(nodeID+"=SENSOR=SET=TEMP="+ String(control_temperature) +"=0=0;");
  } else if (command == "AT+HUMI") {
    control_humidity = value.toInt();
    EEPROM.write(EEP_humidity, control_humidity);
    mesh.sendBroadcast(nodeID+"=SENSOR=SET=HUMI="+ String(control_humidity) +"=0=0;");
  } else if (command == "AT+USE") {
    if (value == "true" || value == "1"){
      use_stable_h = 1;
      use_stable_f = 1;
    }else{
      use_stable_h = 0;
      use_stable_f = 0; 
    }
    EEPROM.write(EEP_Stable_h, use_stable_h);
    EEPROM.write(EEP_Stable_f, use_stable_f);
    mesh.sendBroadcast(nodeID+"=SENSOR=SET=USE=1=0=0;");
  } else if (command == "AT+USEH") {
    if (value == "true" || value == "1"){
      use_stable_h = 1;
    }else{
      use_stable_h = 0;
    }
    EEPROM.write(EEP_Stable_h, use_stable_h);
    mesh.sendBroadcast(nodeID+"=SENSOR=SET=USE=0=1=0;");
  } else if (command == "AT+USEF") {
    if (value == "true" || value == "1"){
      use_stable_f = 1;
    }else{
      use_stable_f = 0; 
    }
    EEPROM.write(EEP_Stable_f, use_stable_f);
    mesh.sendBroadcast(nodeID+"=SENSOR=SET=USE=0=0=1;");
  }
  
  else if (command == "AT+RELAY") {
    if(value.toInt() == 1){
      digitalWrite(RELAY_HEATER,  pin_on);
    }else if(value.toInt() == 2){     
      digitalWrite(RELAY_FAN,     pin_on);
    }else{      
      digitalWrite(RELAY_HEATER,  pin_off);
      digitalWrite(RELAY_FAN,     pin_off);
    }
  } else if (command == "AT+LOG") {
    run_log  = (value.toInt() > 0) ? true : false;
  } else if (command == "AT+LIST") {
    Serial.println(mesh_node_list());
  } else if (command == "reboot") {
    ESP.restart();
  }
  mesh.update();
  Serial.print("AT command:");
  Serial.print(command);
  Serial.print("=");
  Serial.println(value);
  EEPROM.commit();
}//Command_service() END

//// ----------- TEST  -----------
void AT_commandHelp() {
  Serial.println("------------ AT command help ------------");
  Serial.println(";AT+HELP=   int;      Tish menual.");
  Serial.println(";AT+TEMP=   int;      Temperature Change.");
  Serial.println(";AT+HUMI=   int;      Humidity Change.");
  Serial.println(";AT+USE=    bool;     Useable Change.");
  Serial.println(";AT+USEH=   bool;     Useable Heat Change.");
  Serial.println(";AT+USEF=   bool;     Useable Fan  Change.");
  Serial.println(";AT+RELAY=  int;      relay(int) on, if int=0 all relay off.");
  Serial.println(";AT+LOG=    bool;     Serial log view");
  Serial.println(";AT+LIST=   bool;     mesh connected nodes");
  Serial.println("-----------------------------------------");
}
char Serial_buf[SERIAL_MAX];
int8_t Serial_num;
void Serial_service() {
  String str1 = strtok(Serial_buf, "=");
  String str2 = strtok(0x00, " ");
  command_Service(str1, str2);
}
void Serial_process() {
  char ch;
  ch = Serial.read();
  mesh.update();
  switch ( ch ) {
    case '\n':
      Serial_num = 0;
      break;
    case ';':
      Serial_buf[Serial_num] = 0x00;
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
void sendMessage() ; // Prototype so PlatformIO doesn't complain
Task sensorLog( TASK_SECOND*60*5, TASK_FOREVER, &sensorValue );
void sensorValue() {
  mesh_send = true;
}

void sensor_values(unsigned long millisec){
  //매쉬 확인
  if(mesh_send&& millisec - time_send > 1000*10){
    time_send = millisec;
    if(mesh_node_list() > 0){
      mesh_info = true;
      String msg = nodeID+"=SENSOR=LOG=" + (String)temperature + "=" + (String)humidity + ';';
      mesh.sendBroadcast( msg );
    }
  }
}
//taskSendMessage funtion end

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  char msg_buf[SERIAL_MAX];
  for (int index = 0; index < msg.length(); index++) {
    msg_buf[index] = msg[index];
  }
  String types   = strtok(msg_buf, "=");
  if (types == "S") {
    String device  = strtok(0x00, "=");
    if (device == nodeID) {
      String command = strtok(0x00, "=");
      String value   = strtok(0x00, ";");
      command_Service(command, value);
    } else if (device == "connecting"){
      mesh.sendBroadcast(nodeID+"=SENSOR=CNT=TRUE=0=0=0;");
    }//echo
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  I2Ctwo.begin(SDA2,SCL2,400000);
  //// ------------ PIN OUT ------------
  pinMode(RELAY_HEATER, OUTPUT);
  digitalWrite(RELAY_HEATER,  pin_off);
  pinMode(RELAY_FAN, OUTPUT);
  digitalWrite(RELAY_FAN,     pin_off);
  for (uint8_t index = 0; index < 2; index++){
    pinMode(SENSOR_H[index], INPUT);
    pinMode(SENSOR_W[index], INPUT);
  }
  //// ------------ PIN OUT ------------
  if (!EEPROM.begin(EEPROM_SIZE)) Serial.println("failed to initialise EEPROM");
  //// ------------ EEPROM ------------
  
  if(EEPROM.read(EEP_temperature) == 255){EEPROM.write(EEP_temperature, 3);EEPROM.commit();}
  if(EEPROM.read(EEP_humidity)    == 255){EEPROM.write(EEP_humidity, 50);EEPROM.commit();}
  if(EEPROM.read(EEP_Stable_h) == 255){EEPROM.write(EEP_Stable_h, 0);EEPROM.commit();}
  if(EEPROM.read(EEP_Stable_f) == 255){EEPROM.write(EEP_Stable_f, 0);EEPROM.commit();}

  control_temperature = byte(EEPROM.read(EEP_temperature));
  control_humidity    = byte(EEPROM.read(EEP_humidity));

  if(EEPROM.read(EEP_Stable_h) != 0){use_stable_h = true;}
  if(EEPROM.read(EEP_Stable_f) != 0){use_stable_f = true;}
  
  //// ------------ EEPROM ------------
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &taskScheduler, MESH_PORT );
  mesh.setContainsRoot( true );

  mesh.onReceive(&receivedCallback);
  
  nodeID = mesh.getNodeId();

  taskScheduler.addTask( sensorLog );
  sensorLog.enable();

  Serial.print("System online, Set temperature is ");
  Serial.print(control_temperature);
  Serial.print(", Set humidity is ");
  Serial.print(control_humidity);  
  Serial.print(", Set Operation Heat: ");
  Serial.print(use_stable_h);
  Serial.print(", Fan: ");
  Serial.println(use_stable_f);
  AT_commandHelp();
  Serial.print("Device nodeID = ");
  Serial.println(nodeID);
  Serial.println("ver 1.1.1");
}

void loop() {
  unsigned long now = millis();
  if (Serial.available()) Serial_process();
  mesh.update();
  get_sensor(now);
  mesh.update();
  stable(now);
  mesh.update();
  serial_monit(now);
  mesh.update();
  mesh_restart(now);
  sensor_values(now);
}

uint8_t err_stable = 0;
boolean temp_flage(boolean onoff_Heater, boolean onoff_Fan) {
  if (run_heater == onoff_Heater && run_fan == onoff_Fan) {
    return false;
  }
  mesh.update();
  if (run_heater != onoff_Heater) {
    run_heater = onoff_Heater;
    if (onoff_Heater) {
      Serial.println("Heater on");
      mesh.sendBroadcast(nodeID+"=SENSOR=RELAY=ON=HEAT=1=1;");
    }
    else {
      Serial.println("Heater off");
      mesh.sendBroadcast(nodeID+"=SENSOR=RELAY=OFF=HEAT=0=0;");
    }
  }
  if (run_fan != onoff_Fan) {
    run_fan = onoff_Fan;
    if (onoff_Fan) {
      Serial.println("Fan on");
      mesh.sendBroadcast(nodeID+"=SENSOR=RELAY=ON=FAN=1=1;");
    }
    else {
      Serial.println("Fan off");
      mesh.sendBroadcast(nodeID+"=SENSOR=RELAY=OFF=FAN=0=0;");
    }
  }
  return true;
}

void stable(unsigned long millisec) {
  if ((millisec - time_stayble) > 1000 * 1) {
    time_stayble = millisec;
    mesh.update();
    if(temperature != 14040 && temperature > 0) {
      ////온도 유지 팬
      if ((use_stable_h || use_stable_f)) {
        if (use_stable_h) {
          if (temperature/100 < control_temperature - tempGap) {
            if (temp_flage(true, false)) { //히터, 팬
              digitalWrite(RELAY_HEATER, pin_on);
            }
          }else if (temperature/100 > control_temperature) {
            if (temp_flage(false, false)) { //히터, 팬
              digitalWrite(RELAY_HEATER, pin_off);
            }
          }
        }else if (use_stable_f) {
          if (temperature/100 > control_temperature + tempGap) {
            if (temp_flage(false, true)) { //히터, 팬
              digitalWrite(RELAY_FAN,    pin_on);
            }
          }else if (temperature/100 < control_temperature) {
            if (temp_flage(false, false)) { //히터, 팬
              digitalWrite(RELAY_FAN,    pin_off);
            }
          }
        }
      }else{//온도 조절 종료
        digitalWrite(RELAY_HEATER, pin_off);
        digitalWrite(RELAY_FAN,    pin_off);
      }
    }
  }//millis()
}//stable() END


void get_sensor(unsigned long millisec) {
  if ((millisec - timer_SHT) > 300) {
    timer_SHT = millisec;
    bool sensor_conn = false;
    for (int index = 0; index < 8; index++){
      tca_select(index);
      Wire.beginTransmission(68); //0x44
      mesh.update();
      if(!Wire.endTransmission()){
        #ifdef SENSOR_SHT40
          if (sht40.begin()) {
            sensors_event_t humi, temp;
            sht40.getEvent(&humi, &temp);
            sht_port = index;
            temperature  = temp.temperature * 100;
            humidity     = humi.relative_humidity * 100;
            sensor_conn  = true;
          }
        #else
          if (sht31.begin(0x44)) {
            sht_port = index;
            temperature  = sht31.readTemperature() * 100;
            humidity    = sht31.readHumidity() * 100;
            sensor_conn  = true;
          } 
        #endif
      }
    }
    if(!sensor_conn){
      temperature  = 14040;
      humidity     = 14040;
    }
  }//if
}

unsigned long timer_serial_monit = 0;
void serial_monit(unsigned long millisec){
  if (run_log && ((millisec - timer_serial_monit) > 1000)) {
    timer_serial_monit = millisec;
    Serial.println(ERR_Message);
    Serial.print("TCA Port");
    Serial.print(sht_port);
    Serial.print(", T: ");
    Serial.print(temperature);
    Serial.print("°C ,H: ");
    Serial.print(humidity);
    Serial.println("%");
    mesh.update();
    Serial.print("USE_heater = ");
    Serial.print(use_stable_h);
    Serial.print(", USE_fan = ");
    Serial.println(use_stable_f);
    Serial.print(", heater");
    Serial.print(run_heater);
    Serial.print(", fan");
    Serial.print(run_fan);
    Serial.println(";");
    mesh.update();
  }
}

unsigned long timer_restart = 0;
uint8_t restart_count       = 0;
void mesh_restart(unsigned long millisec){
  if(millisec - timer_restart > 1000*60){
    timer_restart = millisec;
    if(restart_count++ > 60) ESP.restart();
  }
}
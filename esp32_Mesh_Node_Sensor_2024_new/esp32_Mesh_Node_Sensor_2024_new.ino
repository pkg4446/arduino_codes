#include  <Arduino.h>
#include  <painlessMesh.h>
#include  <EEPROM.h>
#include  <Wire.h>
#include  <FS.h>
#include  <SD.h>
#include  <SPI.h>
#include  <Adafruit_SHT31.h>
#include  <DS3231.h>

#define EEPROM_SIZE 7
#define SERIAL_MAX  128

#define MESH_PREFIX   "HiveMesh"
#define MESH_PASSWORD "smarthive123"
#define MESH_PORT     3333

#define TCAADDR 0x70
#define SDA2 33
#define SCL2 32
TwoWire I2Ctwo = TwoWire(1);

//#define SENSOR_SHT40
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

uint8_t liquid_sensor = 0;
uint8_t sht_port      = 0;

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
boolean use_water_f  = false;
boolean use_honey_f  = false;

boolean use_water    = false;
boolean use_honey    = false;

boolean sensor_state_w = false;//센서 고장여부 확인 플레그
boolean sensor_state_h = false;
boolean run_water  = false;
boolean run_honey  = false;
boolean run_heater = false;
boolean run_fan    = false;
boolean run_log    = false;
//// ----------- Variable -----------
//// ----------- Sensor -----------
int16_t temperature = 14040;
int16_t humidity    = 14040;
////for millis() func//
unsigned long timer_SHT   = 0UL;
unsigned long time_water    = 0UL;
unsigned long time_stalbe   = 0UL;
unsigned long time_send     = 0UL;
//// ------------ EEPROM ------------
const uint8_t EEP_temperature = 1;
const uint8_t EEP_humidity    = 2;
const uint8_t EEP_Stable_h    = 3;
const uint8_t EEP_Stable_f    = 4;
const uint8_t EEP_water       = 5;
const uint8_t EEP_honey       = 6;
//// ------------ EEPROM Variable ---
uint8_t control_temperature = 33;
uint8_t control_humidity    = 50;
//// ------------- PIN --------------
const uint8_t RELAY_HEATER  = 12;
const uint8_t RELAY_FAN     = 16;
const uint8_t RELAY_VALVE_H = 13;
const uint8_t RELAY_VALVE_W = 14;
const uint8_t SENSOR_H[2] = {35,34};
const uint8_t SENSOR_W[2] = {17,36};

//// ----------- Variable -----------
uint8_t count_sensor_err_w = 0;
uint8_t count_sensor_err_h = 0;

//// ----------- Command  -----------
void command_Service(String command, String value) {
  if (command == "AT+RES") {
    if(mesh_info) mesh_send = false;
  } else if (command == "AT+HELP") {
    AT_commandHelp();
  } else if (command == "AT+TEMP") {
    control_temperature = value.toInt();
    EEPROM.write(EEP_temperature, control_temperature);
    mesh.sendBroadcast("SENSOR=SET=TEMP="+ String(control_temperature) +"=0=0;");
  } else if (command == "AT+HUMI") {
    control_humidity = value.toInt();
    EEPROM.write(EEP_humidity, control_humidity);
    mesh.sendBroadcast("SENSOR=SET=HUMI="+ String(control_humidity) +"=0=0;");
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
    mesh.sendBroadcast("SENSOR=SET=USE=1=0=0;");
  } else if (command == "AT+USEH") {
    if (value == "true" || value == "1"){
      use_stable_h = 1;
    }else{
      use_stable_h = 0;
    }
    EEPROM.write(EEP_Stable_h, use_stable_h);
    mesh.sendBroadcast("SENSOR=SET=USE=0=1=0;");
  } else if (command == "AT+USEF") {
    if (value == "true" || value == "1"){
      use_stable_f = 1;
    }else{
      use_stable_f = 0; 
    }
    EEPROM.write(EEP_Stable_f, use_stable_f);
    mesh.sendBroadcast("SENSOR=SET=USE=0=0=1;");
  } else if (command == "AT+LIQUID") {
    sensor_state_w     = false;
    sensor_state_h     = false;
    count_sensor_err_h = 0;
    count_sensor_err_w = 0;
    mesh.sendBroadcast("SENSOR=SET=WATER=HONEY=0=0;");
  }else if (command == "AT+WATER") {
    if (value == "true" || value == "1"){
      use_water_f = 1;
    }else{
      use_water_f = 0;
    }
    EEPROM.write(EEP_water, use_water_f);
    mesh.sendBroadcast("SENSOR=SET=WATER=HONEY=1=1;");
  }else if (command == "AT+HONEY") {
    if (value == "true" || value == "1"){
      use_honey_f = 1;
    }else{
      use_honey_f = 0;
    }
    EEPROM.write(EEP_honey, use_honey_f);
    mesh.sendBroadcast("SENSOR=SET=WATER=HONEY=1=1;");
  }
  
  else if (command == "AT+RELAY") {
    if(value.toInt() == 1){
      digitalWrite(RELAY_HEATER,  pin_on);
    }else if(value.toInt() == 2){     
      digitalWrite(RELAY_FAN,     pin_on);
    }else if(value.toInt() == 3){     
      digitalWrite(RELAY_VALVE_H, pin_on);
    }else if(value.toInt() == 4){     
      digitalWrite(RELAY_VALVE_W, pin_on);
    }else{      
      digitalWrite(RELAY_HEATER,  pin_off);
      digitalWrite(RELAY_FAN,     pin_off);
      digitalWrite(RELAY_VALVE_H, pin_off);
      digitalWrite(RELAY_VALVE_W, pin_off);
    }
  } else if (command == "AT+LOG") {
    run_log  = (value.toInt() > 0) ? true : false;
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
  Serial.println(";AT+WATER=  bool;     Useable Water Change.");
  Serial.println(";AT+HONEY=  bool;     Useable Honey Change.");
  Serial.println(";AT+LIQUID= bool;     Water&Honey Alarm Reset.");
  Serial.println(";AT+RELAY=  int;      relay(int) on, if int=0 all relay off.");
  Serial.println(";AT+LOG=    bool;     Serial log view");
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
      String msg = "SENSOR=LOG=" + (String)temperature + "=" + (String)humidity + ';';
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
      mesh.sendBroadcast("SENSOR=CNT=TRUE=0=0=0;");
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
  pinMode(RELAY_VALVE_H, OUTPUT);
  digitalWrite(RELAY_VALVE_H, pin_off);
  pinMode(RELAY_VALVE_W, OUTPUT);
  digitalWrite(RELAY_VALVE_W, pin_off);
  for (uint8_t index = 0; index < 2; index++){
    pinMode(SENSOR_H[index], INPUT);
    pinMode(SENSOR_W[index], INPUT);
  }
  //// ------------ PIN OUT ------------
  if (!EEPROM.begin(EEPROM_SIZE)) Serial.println("failed to initialise EEPROM");
  //// ------------ EEPROM ------------
  control_temperature = byte(EEPROM.read(EEP_temperature));
  control_humidity    = byte(EEPROM.read(EEP_humidity));
  if(EEPROM.read(EEP_Stable_h) != 0){use_stable_h = true;}
  if(EEPROM.read(EEP_Stable_f) != 0){use_stable_f = true;}
  if(EEPROM.read(EEP_water) != 0)   {use_water_f = true;}
  if(EEPROM.read(EEP_honey) != 0)   {use_honey_f = true;}
  
  //// ------------ EEPROM ------------
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &taskScheduler, MESH_PORT );
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
  Serial.print(use_stable_f);
  Serial.print(", Set Water: ");
  Serial.println(use_water_f);
  Serial.print(", Honey: ");
  Serial.println(use_honey_f);
  AT_commandHelp();
  Serial.print("Device nodeID = ");
  Serial.println(nodeID);
  Serial.println("ver 1.1.1");
}

void loop() {
  unsigned long now = millis();
  if (Serial.available()) Serial_process();
  mesh.update();
  sensor_level(now);
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

uint8_t full_water = 0;
uint8_t full_honey = 0;
uint8_t err_sensor = 0;
void sensor_level(unsigned long millisec) {
  if ((millisec - time_water) > 1000 * 1) {
    time_water = millisec;
    if(!digitalRead(SENSOR_W[0])){
      if(digitalRead(SENSOR_W[1])){
        Serial.println("water_sensor_error");
      }else{
        use_water = true;
      }
    }else{
      digitalWrite(RELAY_VALVE_W, pin_off);
    }
    mesh.update();
    if(!digitalRead(SENSOR_H[0])){
      if(digitalRead(SENSOR_H[1])){
        Serial.println("water_sensor_error");
      }else{
        use_honey = true;
      }
    }else{
      digitalWrite(RELAY_VALVE_H, pin_off);
    }

    if(use_water_f && use_water){
      mesh.update();
      if(run_water && sensor_state_w){
        //솔레노이드 벨브 정지
        digitalWrite(RELAY_VALVE_W, pin_off);
        run_water = false;
        Serial.println("water_relay_stop_err");
      }else if(!sensor_state_w){
        if(!run_water && !digitalRead(SENSOR_W[1])){
          //mesh.sendBroadcast("P=ID=AT+PUMP=3;"); //펌프 켜기
          digitalWrite(RELAY_VALVE_W, pin_on);   //솔레노이드 밸브 켜기
          run_water = true;
          mesh.sendBroadcast("SENSOR=RELAY=ON=WATER=1=1;");
          Serial.println("water_relay_run");
        }else if(digitalRead(SENSOR_W[1])){
          if(run_water){
            //가득참
            //mesh.sendBroadcast("P=ID=AT+PUMP=0;"); //펌프 끄기
            digitalWrite(RELAY_VALVE_W, pin_off);
            run_water = false;
            mesh.sendBroadcast("SENSOR=RELAY=OFF=WATER=0=0;");
            Serial.println("water_relay_stop_full");
          }else{
            full_water++;
            if (full_water > 254){
              digitalWrite(RELAY_VALVE_W, pin_off);
              run_water      = false;
              sensor_state_w = true;
              ERR_Message = "SENSOR=ERR=EMPTY=WATER=0=0;";
              mesh.sendBroadcast(ERR_Message);
              mesh.sendBroadcast("SENSOR=RELAY=OFF=WATER=0=0;");
              Serial.println("water_relay_stop_timeout");
            }
          }//밸브가 열려있을때 수위가 올라가지 않을경우
        }
      }
    }//use_water
    if(use_honey_f && use_honey){
      mesh.update();
      if(run_honey && sensor_state_h){
        //솔레노이드 벨브 정지
        run_honey = false;
        digitalWrite(RELAY_VALVE_H, pin_off);
        Serial.println("honey_relay_stop_err");
      }else if(!sensor_state_h){
        if(!run_honey && !digitalRead(SENSOR_H[1])){
          //mesh.sendBroadcast("P=ID=AT+PUMP=3;"); //펌프 켜기
          digitalWrite(RELAY_VALVE_H, pin_on);     //솔레노이드 밸브 켜기
          run_honey = true;
          mesh.sendBroadcast("SENSOR=RELAY=ON=HONEY=1=1;");
          Serial.println("honey_relay_run");          
        }else if(digitalRead(SENSOR_H[1])){
          if(run_honey){
            //가득참
            //mesh.sendBroadcast("P=ID=AT+PUMP=0;"); //펌프 끄기
            digitalWrite(RELAY_VALVE_H, pin_off);
            run_honey      = false;
            mesh.sendBroadcast("SENSOR=RELAY=OFF=HONEY=0=0;");
            Serial.println("honey_relay_stop_full");
          }else{
            full_honey++;
            if (full_honey > 254){
              digitalWrite(RELAY_VALVE_H, pin_off);
              run_honey = false;
              sensor_state_h = true;
              ERR_Message = "SENSOR=ERR=EMPTY=HONEY=0=0;";
              mesh.sendBroadcast(ERR_Message);
              mesh.sendBroadcast("SENSOR=RELAY=OFF=HONEY=0=0;");
              Serial.println("honey_relay_stop_timeout");
            }
          }//밸브가 열려있을때 수위가 올라가지 않을경우
        }
      }
    }//use_honey
    mesh.update();
    if(((use_honey_f && use_honey) || (use_water_f && use_water)) && (sensor_state_w || sensor_state_h)){ //센서가 고장일 경우
      //알람 보내기
      if(liquid_sensor++<2){
        if (err_sensor > 254) {
          if(sensor_state_w && sensor_state_h){
            ERR_Message = "SENSOR=ERR=LEVEL=ALL=0=0;";
          }else if(sensor_state_w){
            ERR_Message = "SENSOR=ERR=LEVEL=WATER=0=0;";
          }else{
            ERR_Message = "SENSOR=ERR=LEVEL=HONEY=0=0;";
          }        
          mesh.sendBroadcast(ERR_Message);
          err_sensor  = 0;
        }
        else {
          err_sensor++;
        }
      }
    } else {
      
    }//sensor_check
  }//millis()
}//sensor_Water() END

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
      mesh.sendBroadcast("SENSOR=RELAY=ON=HEAT=1=1;");
    }
    else {
      Serial.println("Heater off");
      mesh.sendBroadcast("SENSOR=RELAY=OFF=HEAT=0=0;");
    }
  }
  if (run_fan != onoff_Fan) {
    run_fan = onoff_Fan;
    if (onoff_Fan) {
      Serial.println("Fan on");
      mesh.sendBroadcast("SENSOR=RELAY=ON=FAN=1=1;");
    }
    else {
      Serial.println("Fan off");
      mesh.sendBroadcast("SENSOR=RELAY=OFF=FAN=0=0;");
    }
  }
  return true;
}

void stable(unsigned long millisec) {
  if ((millisec - time_stalbe) > 1000 * 1) {
    time_stalbe = millisec;
    mesh.update();
    if(temperature != 14040 && temperature > 0) {
      ////온도 유지 팬
      if (use_stable_h || use_stable_f) {
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
      sht_port = index;
      if(!Wire.endTransmission()){
        #ifdef SENSOR_SHT40
          if (sht40.begin()) {
            sensors_event_t humi, temp;
            sht40.getEvent(&humi, &temp);
            temperature  = temp.temperature * 100;
            humidity     = humi.relative_humidity * 100;
            sensor_conn  = true;
          }
        #else
          if (sht31.begin(0x44)) {
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
    Serial.print("sensor_state_w: ");
    Serial.print(sensor_state_w);
    Serial.print(",sensor_state_h: ");
    Serial.print(sensor_state_h);
    Serial.println(";");
    Serial.print("USE_heater = ");
    Serial.print(use_stable_h);
    Serial.print(", USE_fan = ");
    Serial.println(use_stable_f);
    Serial.print("run: water");
    Serial.print(run_water);
    Serial.print(", honey");
    Serial.print(run_honey);
    Serial.print(", heater");
    Serial.print(run_heater);
    Serial.print(", fan");
    Serial.print(run_fan);
    Serial.print(", state");
    Serial.print(count_sensor_err_w);
    Serial.print(count_sensor_err_h);
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
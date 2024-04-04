#include  "painlessMesh.h"
#include  "EEPROM.h"
#define   EEPROM_SIZE 7
#define   SERIAL_MAX  128

#define   ERROR_VALUE   14040

#define   MESH_PREFIX   "smartHiveMesh"
#define   MESH_PASSWORD "smarthive123"
#define   MESH_PORT     3333

#include  "Wire.h"
#define   TCAADDR 0x70
#include  "FS.h"
#include  "SD.h"
#include  "SPI.h"

#define   PCA9539_H_
#include  "Arduino.h"
#define   DEBUG 1
#define   NXP_INPUT  0
#define   NXP_OUTPUT 2
#define   NXP_INVERT 4
#define   NXP_CONFIG 6

#define SDA1 21
#define SCL1 22

#define SDA2 33
#define SCL2 32

TwoWire I2Cone = TwoWire(0);
TwoWire I2Ctwo = TwoWire(1);

const boolean pin_on  = false;
const boolean pin_off = true;

bool mesh_info = false;
bool mesh_send = false;

uint8_t liquid_sensor = 0;

#include "Adafruit_SHT31.h"
Adafruit_SHT31 sht31 = Adafruit_SHT31(&I2Cone);
void tca_select(uint8_t index) {
  if (index > 7) return;
  I2Cone.beginTransmission(TCAADDR);
  I2Cone.write(1 << index);
  I2Cone.endTransmission();
}

Scheduler taskScheduler; // to control upload task
painlessMesh  mesh;
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
uint8_t led_show   = 60;
//// ----------- Variable -----------
//// ----------- Sensor -----------
int16_t temp_boad       = {ERROR_VALUE,};
int16_t temperature[4]  = {ERROR_VALUE,};
int16_t humidity[4]     = {ERROR_VALUE,};
int16_t temp_high   = ERROR_VALUE;
int16_t temp_avg    = ERROR_VALUE;
int8_t  send_water  = 0;
int8_t  send_honey  = 0;
boolean water_level[2]  = {false,};
boolean honey_level[2]  = {false,};
////for millis() func//
unsigned long time_led_show = 0UL;
unsigned long timer_board   = 0UL;
unsigned long timer_sht31   = 0UL;
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
const uint8_t RELAY_VALVE_W = 13;
const uint8_t RELAY_VALVE_H = 14;
const uint8_t SEN_WATER_L   = 35;
const uint8_t SEN_WATER_H   = 34;
const uint8_t SEN_HONEY_L   = 17;
const uint8_t SEN_HONEY_H   = 36;
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
    if (value == "true"){
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
    if (value == "true"){
      use_stable_h = 1;
    }else{
      use_stable_h = 0;
    }
    EEPROM.write(EEP_Stable_h, use_stable_h);
    mesh.sendBroadcast("SENSOR=SET=USE=0=1=0;");
  } else if (command == "AT+USEF") {
    if (value == "true"){
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
    if (value == "true"){
      use_water_f = 1;
    }else{
      use_water_f = 0;
    }
    EEPROM.write(EEP_water, use_water_f);
    mesh.sendBroadcast("SENSOR=SET=WATER=HONEY=1=1;");
  }else if (command == "AT+HONEY") {
    if (value == "true"){
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
      digitalWrite(RELAY_FAN,     !pin_on);
    }else if(value.toInt() == 3){     
      digitalWrite(RELAY_VALVE_H, pin_on);
    }else if(value.toInt() == 4){     
      digitalWrite(RELAY_VALVE_W, pin_on);
    }else{      
      digitalWrite(RELAY_HEATER,  pin_off);
      digitalWrite(RELAY_FAN,     !pin_off);
      digitalWrite(RELAY_VALVE_H, pin_off);
      digitalWrite(RELAY_VALVE_W, pin_off);
    }
  } else if (command == "AT+LOG") {
    run_log  = (value.toInt() > 0) ? true : false;
  } else if (command == "AT+RESTART") {
    ESP.restart();
  }
  mesh.update();
  Serial.print("AT command:");
  Serial.print(command);
  Serial.print("=");
  Serial.println(value);
  EEPROM.commit();
  config();
}//Command_service() END

//// ----------- TEST  -----------
void AT_commandHelp(void) {
  Serial.println("------------ AT command help ------------");
  Serial.println("AT+HELP=   int;      Tish menual");
  Serial.println("AT+TEMP=   int;      Temperature Change");
  Serial.println("AT+HUMI=   int;      Humidity Change");
  Serial.println("AT+USE=    bool;     Useable Change");
  Serial.println("AT+USEH=   bool;     Useable Heat Change");
  Serial.println("AT+USEF=   bool;     Useable Fan  Change");
  Serial.println("AT+WATER=  bool;     Useable Water Change");
  Serial.println("AT+HONEY=  bool;     Useable Honey Change");
  Serial.println("AT+LIQUID= bool;     Water&Honey Alarm Reset");
  Serial.println("AT+RELAY=  int;      relay(int) on, if int=0 all relay off.");
  Serial.println("AT+LOG=    bool;     Serial log view");
  Serial.println("AT+RESTART=bool;     System reboot");
  Serial.println("-----------------------------------------");
}
char Serial_buf[SERIAL_MAX];
int8_t Serial_num;
void Serial_service() {
  String str1 = strtok(Serial_buf, "=");
  String str2 = strtok(0x00, " ");
  command_Service(str1, str2);
}
void Serial_process(void) {
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
      if(ch!='\r' && ch!='\n'){
        Serial_buf[ Serial_num ++ ] = ch;
        Serial_num %= SERIAL_MAX;
      }
      break;
  }
}
//// ----------- TEST  -----------

uint8_t mesh_node_list(void){
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
void sendMessage(); // Prototype so PlatformIO doesn't complain
Task sensorLog( TASK_SECOND*60*5, TASK_FOREVER, &sensorValue );
void sensorValue(void) {
  mesh_send = true;
}

void sensor_values(unsigned long millisec){
  //매쉬 확인
  if(mesh_send&& millisec - time_send > 1000*10){
    time_send = millisec;
    if(mesh_node_list() > 0){
      mesh_info = true;
      String msg = "SENSOR=LOG=" + (String)temp_high + "=" + (String)humidity[0] + "=" + (String)send_water + "=" + (String)send_honey + ';';
      mesh.sendBroadcast( msg );
    }
  }
}
//taskSendMessage funtion end

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  char msg_buf[SERIAL_MAX];
  for (int i = 0; i < msg.length(); i++) {
    msg_buf[i] = msg[i];
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

void setup(void) {
  // put your setup code here, to run once:
  Serial.begin(115200);
  I2Cone.begin(SDA1,SCL1,400000);
  I2Ctwo.begin(SDA2,SCL2,400000);
  //// ------------ PIN OUT ------------
  pinMode(RELAY_HEATER, OUTPUT);
  digitalWrite(RELAY_HEATER,  pin_off);
  pinMode(RELAY_FAN, OUTPUT);
  digitalWrite(RELAY_FAN,     !pin_off);
  pinMode(RELAY_VALVE_H, OUTPUT);
  digitalWrite(RELAY_VALVE_H, pin_off);
  pinMode(RELAY_VALVE_W, OUTPUT);
  digitalWrite(RELAY_VALVE_W, pin_off);
  
  pinMode(SEN_WATER_L, INPUT);
  pinMode(SEN_WATER_H, INPUT);
  pinMode(SEN_HONEY_L, INPUT);
  pinMode(SEN_HONEY_H, INPUT);
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

  Serial.print("System online, ");
  config();
  AT_commandHelp();
  Serial.print("Device nodeID = ");
  Serial.println(nodeID);
  Serial.println("ver 1.1.1");
}

void config(void){
  Serial.print("Set temperature is ");
  Serial.print(control_temperature);
  Serial.print(", Set humidity is ");
  Serial.print(control_humidity);  
  Serial.print(", Set Operation Heat: ");
  Serial.print(use_stable_h);
  Serial.print(", Fan: ");
  Serial.print(use_stable_f);
  Serial.print(", Set Water: ");
  Serial.print(use_water_f);
  Serial.print(", Honey: ");
  Serial.println(use_honey_f);
}

void loop(void) {
  unsigned long now = millis();
  if (Serial.available()) Serial_process();
  mesh.update();
  sensor_level(now);
  mesh.update();
  get_bord_temp(now);
  mesh.update();
  get_sensor(now);
  mesh.update();
  stable(now);
  mesh.update();
  serial_monit(now);
  mesh.update();
  mesh_restart(now);
  mesh.update();
  sensor_values(now);
}

uint8_t full_water = 0;
uint8_t full_honey = 0;
uint8_t err_sensor = 0;
void sensor_level(unsigned long millisec) {
  if ((millisec - time_water) > 1000 * 1) {
    time_water = millisec;
    
    water_level[0] = digitalRead(SEN_WATER_L);
    water_level[1] = digitalRead(SEN_WATER_H);
    honey_level[0] = digitalRead(SEN_HONEY_L);
    honey_level[1] = digitalRead(SEN_HONEY_H);

    if(use_water_f){
      if(water_level[0] && water_level[1]){
        use_water = false; //full
      }else if(!water_level[0] && !water_level[1]){
        use_water = true;  //empty
      }else if(!water_level[0] && water_level[1]){
         if(count_sensor_err_w >= 30){
          sensor_state_w = true;
        }else{
          count_sensor_err_w += 15;
          Serial.println("water_sensor_error");
        }
      }
    }
    mesh.update();
    if(use_honey_f){
      if(honey_level[0] && honey_level[1]){
        use_honey = false; //full
      }else if(!honey_level[0] && !honey_level[1]){
        use_honey = true;  //empty
      }else if(!honey_level[0] && honey_level[1]){
        if(count_sensor_err_h >= 30){
          sensor_state_h = true;
        }else{
          count_sensor_err_h += 15;
          Serial.println("honey_sensor_error");
        }
      }
    }
    mesh.update();

    if(use_water_f && use_water){
      if(!run_water){
        digitalWrite(RELAY_VALVE_W, pin_on);   //솔레노이드 밸브 켜기
        run_water  = true;
        full_water = 0;
        mesh.sendBroadcast("SENSOR=RELAY=ON=WATER=1=1;");
        Serial.println("water_relay_run");
      }else if(sensor_state_w){
        //솔레노이드 벨브 정지(센서에러)
        digitalWrite(RELAY_VALVE_W, pin_off);
        run_water   = false;
        use_water_f = false;
        Serial.println("water_relay_stop_err");
      }else{
        if(run_water && !water_level[1]){
          full_water++;
          if(full_water > 30){
            //솔레노이드 벨브 정지(비었음)
            digitalWrite(RELAY_VALVE_W, pin_off);
            use_water_f = false;
            mesh.sendBroadcast("SENSOR=ERR=EMPTY=WATER=0=0;");
            Serial.println("water_relay_stop_timeout");
          }
        }
      }//밸브가 열려있을때 수위가 올라가지 않을경우
    }else{
      //솔레노이드 벨브 정지
      run_water = false;
      use_water = false;
      digitalWrite(RELAY_VALVE_W, pin_off);
    }//use_water
    mesh.update();

    if(use_honey_f && use_honey){
      mesh.update();
      if(!run_honey){
        digitalWrite(RELAY_VALVE_H, pin_on);     //솔레노이드 밸브 켜기
        run_honey  = true;
        full_honey = 0;
        mesh.sendBroadcast("SENSOR=RELAY=ON=HONEY=1=1;");
        Serial.println("honey_relay_run");  
      }else if(sensor_state_h){
        //솔레노이드 벨브 정지(센서에러)
        digitalWrite(RELAY_VALVE_H, pin_off);
        run_honey   = false;
        use_honey_f = false;
        Serial.println("honey_relay_stop_err");
      }else{
        if(!honey_level[1]){
          full_honey++;
          if (full_honey > 30){
          //솔레노이드 벨브 정지(비었음)
            digitalWrite(RELAY_VALVE_H, pin_off);
            use_honey_f = false;
            mesh.sendBroadcast("SENSOR=ERR=EMPTY=HONEY=0=0;");
            Serial.println("honey_relay_stop_timeout");
          }
        }
      }//밸브가 열려있을때 수위가 올라가지 않을경우
    }else{
      //솔레노이드 벨브 정지
      run_honey = false;
      use_honey = false;
      digitalWrite(RELAY_VALVE_H, pin_off);
    }//use_honey
    mesh.update();
  }//millis()
}//sensor_Water() END

uint8_t err_stable = 0;
void temp_flage(boolean onoff_Heater, boolean onoff_Fan) {
  digitalWrite(RELAY_HEATER, !onoff_Heater);
  digitalWrite(RELAY_FAN,    onoff_Fan);
  mesh.update();

  if (run_heater == onoff_Heater && run_fan == onoff_Fan) {
    return;
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
}

void stable(unsigned long millisec) {
  if ((millisec - time_stalbe) > 1000 * 1) {
    time_stalbe = millisec;
    mesh.update();
    if(temp_high != ERROR_VALUE && temp_high > 0) {
      ////온도 유지 팬
      if (use_stable_h || use_stable_f) {
        if (use_stable_h) {
          if (temp_high/100 < control_temperature - tempGap) {
            temp_flage(true, false);
          }else if (temp_high/100 > control_temperature) {
            temp_flage(false, false);
          }
        }else if (use_stable_f) {
          if (temp_high/100 > control_temperature + tempGap) {
            temp_flage(false, true);
          }else if (temp_high/100 < control_temperature) {
            temp_flage(false, false);
          }
        }
      }else{//온도 조절 종료
        digitalWrite(RELAY_HEATER, pin_off);
        digitalWrite(RELAY_FAN,    !pin_off);
      }
    }
  }//millis()
}//stable() END

void get_bord_temp(unsigned long millisec) {
  if ((millisec - timer_board) > 1000) {
    timer_board = millisec;
    I2Ctwo.requestFrom(0x48,1);
    temp_boad   = uint16_t(I2Ctwo.read() << 8)/256;
    //temp_boad  /= 256;
    I2Ctwo.endTransmission();
    mesh.update();
  }//if
}

void get_sensor(unsigned long millisec) {
  if ((millisec - timer_sht31) > 300) {
    timer_sht31 = millisec;
    uint16_t sensor_mount = 0;
    int16_t sensor_values = 0;
    temp_high = ERROR_VALUE;
    for(uint8_t index=0; index<4; index++){
      tca_select(index);
      I2Cone.beginTransmission(68); //0x44
      mesh.update();
      if (!I2Cone.endTransmission() && sht31.begin()) {
        temperature[index] = sht31.readTemperature()*100;
        humidity[index]    = sht31.readHumidity()*100;
        sensor_mount  += 1;
        sensor_values += temperature[index];
        if(temp_high == ERROR_VALUE || temp_high < temperature[index]) temp_high = temperature[index];
      } else {
        temperature[index] = ERROR_VALUE;
        humidity[index]    = ERROR_VALUE;
      }
      mesh.update();
    }
    if(sensor_mount>0) temp_avg = sensor_values/sensor_mount;
    else{
      temp_high = ERROR_VALUE;
      temp_avg  = ERROR_VALUE;
    }
    if(temp_high == ERROR_VALUE) temp_flage(true, false);
  }//if
}

unsigned long timer_serial_monit = 0;
void serial_monit(unsigned long millisec){
  if (run_log && ((millisec - timer_serial_monit) > 1000)) {
    timer_serial_monit = millisec;
    Serial.print("T high: ");
    Serial.print(temp_high);
    Serial.print("°C ,T avg: ");
    Serial.print(temp_avg);
    Serial.println("°C");
    for(uint8_t index=0; index<4; index++){
      Serial.print("TCA Port");
      Serial.print(index);
      Serial.print(", T: ");
      Serial.print(temperature[index]);
      Serial.print("°C ,H: ");
      Serial.print(humidity[index]);
      Serial.println("%");
      mesh.update();
    }
    Serial.print("Board T: ");
    Serial.print(temp_boad);
    Serial.println("°C");
    mesh.update();
    Serial.print("sensor_state_w: ");
    Serial.print(sensor_state_w);
    Serial.print(",sensor_state_h: ");
    Serial.print(sensor_state_h);
    Serial.println(";");
    mesh.update();
    Serial.print("USE_heater = ");
    Serial.print(use_stable_h);
    Serial.print(", USE_fan = ");
    Serial.println(use_stable_f);
    mesh.update();
    Serial.print("run: water");
    Serial.print(run_water);
    Serial.print(", honey");
    Serial.print(run_honey);
    Serial.print(", heater");
    Serial.print(run_heater);
    Serial.print(", fan");
    Serial.print(run_fan);
    Serial.println(";");
    mesh.update();
    Serial.print(count_sensor_err_w);
    Serial.print("-water_level: ");
    Serial.print(water_level[0]);
    Serial.print(water_level[1]);
    Serial.println(";");
    mesh.update();
    Serial.print(count_sensor_err_h);
    Serial.print("-honey_level: ");
    Serial.print(honey_level[0]);
    Serial.print(honey_level[1]);   
    Serial.println(";\n");
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


/* 루프로 1시간에 한번 리셋? mesh_restart에서 리셋하니 필요 없음. 고려해서 mesh_restart 시간 변경 필요.
  if(EEPROM.read(EEP_water) != 0)   {use_water_f = true;}
  if(EEPROM.read(EEP_honey) != 0)   {use_honey_f = true;}
*/
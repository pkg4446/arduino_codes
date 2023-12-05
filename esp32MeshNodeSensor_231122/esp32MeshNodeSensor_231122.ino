#include  "painlessMesh.h"
#include  "EEPROM.h"
#define   EEPROM_SIZE 5
#define   SERIAL_MAX  128

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

const boolean pin_on  = false;
const boolean pin_off = true;

bool mesh_send = false;

class PCA9539 {
  public:
    PCA9539(uint8_t address);                       // constructor
    void pinMode(uint8_t pin, uint8_t IOMode );     // pinMode
    uint8_t digitalRead(uint8_t pin);               // digitalRead
    void digitalWrite(uint8_t pin, uint8_t value ); // digitalWrite

  private:
    uint16_t I2CGetValue(uint8_t address, uint8_t reg);
    void I2CSetValue(uint8_t address, uint8_t reg, uint8_t value);

    union {
      struct {
        uint8_t _configurationRegister_low;   // low order byte
        uint8_t _configurationRegister_high;  // high order byte
      };
      uint16_t _configurationRegister;        // 16 bits presentation
    };
    union {
      struct {
        uint8_t _valueRegister_low;           // low order byte
        uint8_t _valueRegister_high;          // high order byte
      };
      uint16_t _valueRegister;
    };
    uint8_t _address;                         // address of port this class is supporting
    int _error;                               // error code from I2C
};

PCA9539::PCA9539(uint8_t address) {
  _address        = address;          // save the address id
  _valueRegister  = 0;
  Wire.begin();                       // start I2C communication
}

void PCA9539::pinMode(uint8_t pin, uint8_t IOMode) {
  if (pin <= 15) {
    if (IOMode == OUTPUT) {
      _configurationRegister = _configurationRegister & ~(1 << pin);
    } else {
      _configurationRegister = _configurationRegister | (1 << pin);
    }
    I2CSetValue(_address, NXP_CONFIG    , _configurationRegister_low);
    I2CSetValue(_address, NXP_CONFIG + 1, _configurationRegister_high);
  }
}

uint8_t PCA9539::digitalRead(uint8_t pin) {
  uint16_t _inputData = 0;
  if (pin > 15 ) return 255;
  _inputData  = I2CGetValue(_address, NXP_INPUT);
  _inputData |= I2CGetValue(_address, NXP_INPUT + 1) << 8;
  if ((_inputData & (1 << pin)) > 0) {
    return HIGH;
  } else {
    return LOW;
  }
}

void PCA9539::digitalWrite(uint8_t pin, uint8_t value) {
  if (pin > 15 ) {
    _error = 255;            // invalid pin
    return;                  // exit
  }
  if (value > 0) {
    _valueRegister = _valueRegister | (1 << pin);   // and OR bit in register
  } else {
    _valueRegister = _valueRegister & ~(1 << pin);  // AND all bits
  }
  I2CSetValue(_address, NXP_OUTPUT    , _valueRegister_low);
  I2CSetValue(_address, NXP_OUTPUT + 1, _valueRegister_high);
}

uint16_t PCA9539::I2CGetValue(uint8_t address, uint8_t reg) {
  uint16_t _inputData;
  Wire.beginTransmission(address);      // setup read registers
  Wire.write(reg);
  _error = Wire.endTransmission();
  if (Wire.requestFrom((int)address, 1) != 1)
  {
    return 256;                         // error code is above normal data range
  };
  _inputData = Wire.read();
  return _inputData;
}

void PCA9539::I2CSetValue(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);      // setup direction registers
  Wire.write(reg);                      // pointer to configuration register address 0
  Wire.write(value);                    // write config register low byte
  _error = Wire.endTransmission();
}

PCA9539 ioport(0x74); // Base address starts at 0x74 for A0 = L and A1 = L
//Address   A1    A0
//0x74      L     L
//0x75      L     H
//0x76      H     L
//0x77      H     H

#include "Adafruit_SHT4x.h"
Adafruit_SHT4x sht40 = Adafruit_SHT4x();
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

boolean SHT40        = false;
boolean use_stable_h = false;
boolean use_stable_f = false;
boolean use_water    = true;
boolean use_honey    = true;

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
int16_t temperature = 14040;
int16_t humidity    = 14040;
int8_t  send_water  = 0;
int8_t  send_honey  = 0;
boolean water_level[6] = {false,};
boolean honey_level[6] = {false,};
////for millis() func//
unsigned long time_led_show = 0UL;
unsigned long timer_SHT40   = 0UL;
unsigned long time_water    = 0UL;
unsigned long time_stalbe   = 0UL;
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
const uint8_t RELAY_VALVE_H = 13;
const uint8_t RELAY_VALVE_W = 14;
const uint8_t BUILTIN_LED_A = 17;
const uint8_t BUILTIN_LED_B = 25;
//// ----------- Variable -----------
uint8_t count_sensor_err_w = 0;
uint8_t count_sensor_err_h = 0;

//// ----------- Command  -----------
void command_Service(String command, String value) {
  if (command == "AT+TEMP") {
    control_temperature = value.toInt();
    EEPROM.write(EEP_temperature, control_temperature);
    mesh.sendBroadcast("SENSOR=SET=TEMP=0=0=0;");
  } else if (command == "AT+HUMI") {
    control_humidity = value.toInt();
    EEPROM.write(EEP_humidity, control_humidity);
    mesh.sendBroadcast("SENSOR=SET=HUMI=0=0=0;");
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
  }
  
  else if (command == "AT+RELAY") {
    if(value.toInt() > 0){
      digitalWrite(RELAY_HEATER,  pin_on);
      digitalWrite(RELAY_FAN,     pin_on);
      digitalWrite(RELAY_VALVE_H, pin_on);
      digitalWrite(RELAY_VALVE_W, pin_on);
    }else{      
      digitalWrite(RELAY_HEATER,  pin_off);
      digitalWrite(RELAY_FAN,     pin_off);
      digitalWrite(RELAY_VALVE_H, pin_off);
      digitalWrite(RELAY_VALVE_W, pin_off);
    }
  } else if (command == "AT+LOG") {
    run_log  = (value.toInt() > 0) ? true : false;
  } else if (command == "AT+SHOW") {
    led_show = (value.toInt() > 0) ? 0 : 60;
  }
  mesh.update();
  Serial.print("AT command:");
  Serial.print(command);
  Serial.print("=");
  Serial.println(value);
  EEPROM.commit();
}//Command_service() END

boolean led_status = false;
void builtin_led(unsigned long millisec){
  if ((millisec - time_led_show) > 1000 * 1) {
    if(led_show<60){
      time_led_show = millisec;
      Serial.print("led: 1");
      digitalWrite(BUILTIN_LED_A, led_status);
      digitalWrite(BUILTIN_LED_B, led_status);
      led_show ++;
      led_status = !led_status;
    }else{
      digitalWrite(BUILTIN_LED_A, pin_off);
      digitalWrite(BUILTIN_LED_B, pin_off);
    }
  }
}

//// ----------- TEST  -----------
void AT_commandHelp() {
  Serial.println("------------ AT command help ------------");
  Serial.println(";AT+TEMP=   int;      Temperature Change.");
  Serial.println(";AT+HUMI=   int;      Humidity Change.");
  Serial.println(";AT+USE=    bool;     Useable Change.");
  Serial.println(";AT+LIQUID= bool;     Water&Honey Alarm Reset.");
  Serial.println(";AT+RELAY=  bool;     All relay on or off test.");
  Serial.println(";AT+LOG=    bool;     Serial log view");
  Serial.println(";AT+SHOW=   bool;     Builtin led on off");
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
  if(mesh_send&& millisec - time_send > 3000){
    time_send = millisec;
    if(mesh_node_list() > 0){
      mesh_send = false;
      String msg = "SENSOR=LOG=" + (String)temperature + "=" + (String)humidity + "=" + (String)send_water + "=" + (String)send_honey + ';';
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  for (int8_t index = 0 ; index <= 15 ; index++) {
    ioport.pinMode(index, INPUT);
  }
  //// ------------ PIN OUT ------------
  pinMode(RELAY_HEATER, OUTPUT);
  digitalWrite(RELAY_HEATER,  pin_off);
  pinMode(RELAY_FAN, OUTPUT);
  digitalWrite(RELAY_FAN,     pin_off);
  pinMode(RELAY_VALVE_H, OUTPUT);
  digitalWrite(RELAY_VALVE_H, pin_off);
  pinMode(RELAY_VALVE_W, OUTPUT);
  digitalWrite(RELAY_VALVE_W, pin_off);
  pinMode(BUILTIN_LED_A, OUTPUT);
  digitalWrite(BUILTIN_LED_A, pin_off);
  pinMode(BUILTIN_LED_B, OUTPUT);
  digitalWrite(BUILTIN_LED_B, pin_off);
  //// ------------ PIN OUT ------------
  if (!EEPROM.begin(EEPROM_SIZE)) Serial.println("failed to initialise EEPROM");
  //// ------------ EEPROM ------------
  control_temperature = byte(EEPROM.read(EEP_temperature));
  control_humidity    = byte(EEPROM.read(EEP_humidity));
  if(EEPROM.read(EEP_Stable_h) != 0){use_stable_h = true;}
  if(EEPROM.read(EEP_Stable_f) != 0){use_stable_f = true;}
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
  Serial.println(use_stable_f);
  AT_commandHelp();
  Serial.print("Device nodeID = ");
  Serial.println(nodeID);
  Serial.println("ver 1.0.2");
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
  builtin_led(now);
  mesh.update();
  mesh_restart(now);
  sensor_values(now);
}

uint8_t full_water = 0;
uint8_t full_honey = 0;
uint8_t err_sensor = 0;
uint8_t level_gauge_w = 0;
uint8_t level_gauge_h = 0;
void sensor_level(unsigned long millisec) {
  if ((millisec - time_water) > 1000 * 1) {
    time_water = millisec;
    send_water = 0;
    send_honey = 0;
    for (int8_t index = 0 ; index < 6 ; index++) {
      water_level[index] = ioport.digitalRead(index);
      honey_level[index] = ioport.digitalRead(index+6);
      mesh.update();
      //수위(단위 %)
      if(water_level[index]) send_water = (index+1)*20;
      if(honey_level[index]) send_honey = (index+1)*20;
    }
    if(!water_level[1]) use_water = true; //센서 수위가 40% 이하일 경우 급수
    if(!honey_level[1]) use_honey = true;
    for (int8_t index = 1 ; index < 6 ; index++) { //위는 켜졌는데 아래는 안켜질 경우(고장) 검증
      if(water_level[index] && !water_level[index-1]){
        if(count_sensor_err_w >= 30){
          sensor_state_w = true;
        }else{
          count_sensor_err_w += 15;
          Serial.println("water_sensor_error");
        }
      }else if(count_sensor_err_w>0){
        count_sensor_err_w -= 1;
      }
      if(honey_level[index] && !honey_level[index-1]){
        if(count_sensor_err_h >= 30){
          sensor_state_h = true;
        }else{
          count_sensor_err_h += 15;
          Serial.println("honey_sensor_error");
        }
      }else if(count_sensor_err_h>0){
        count_sensor_err_h -= 1;
      }
      if(sensor_state_w && sensor_state_h) break;//센서들이 고장일 경우
      mesh.update();
    }
    if(use_water){
      mesh.update();
      if(run_water && sensor_state_w){
        //솔레노이드 벨브 정지
        digitalWrite(RELAY_VALVE_W, pin_off);
        run_water = false;
        Serial.println("water_relay_stop_err");
      }else if(!sensor_state_w){
        if(!run_water && !water_level[1]){
          mesh.sendBroadcast("P=ID=AT+PUMP=3;"); //펌프 켜기
          digitalWrite(RELAY_VALVE_W, pin_on);   //솔레노이드 밸브 켜기
          run_water = true;
          level_gauge_w = 1;
          mesh.sendBroadcast("SENSOR=RELAY=ON=WATER=0=0;");
          Serial.println("water_relay_run");
        }else{
          if(water_level[3]){
            if(run_water){
              //가득참
              //mesh.sendBroadcast("P=ID=AT+PUMP=0;"); //펌프 끄기
              digitalWrite(RELAY_VALVE_W, pin_off);
              run_water      = false;
              mesh.sendBroadcast("SENSOR=RELAY=OFF=WATER=0=0;");
              Serial.println("water_relay_stop_full");
            }            
          } else if(water_level[4]||water_level[5]){
            //넘침
            //mesh.sendBroadcast("P=ID=AT+PUMP=0;"); //펌프 끄기
            digitalWrite(RELAY_VALVE_W, pin_off);
            mesh.sendBroadcast("SENSOR=ERR=OVERFLOW=WATER=0=0;");
            Serial.println("water_relay_stop_overflow");
          }else if(run_water){ 
            if(!water_level[level_gauge_w]){
              full_water++;
              if (full_water > 240){
                digitalWrite(RELAY_VALVE_W, pin_off);
                run_water      = false;
                sensor_state_w = true;
                ERR_Message = "SENSOR=ERR=EMPTY=WATER=0=0;";
                mesh.sendBroadcast(ERR_Message);
                mesh.sendBroadcast("SENSOR=RELAY=OFF=WATER=0=0;");
                Serial.println("water_relay_stop_timeout");
              }
            } else if(level_gauge_w != 0){
              level_gauge_w += 1;
              if(level_gauge_w > 6){
                level_gauge_w = 0;
                digitalWrite(RELAY_VALVE_W, pin_off);
                run_water = false;
              }
            }
          }//밸브가 열려있을때 수위가 올라가지 않을경우
        }
      }
    }//use_water
    if(use_honey){
      mesh.update();
      if(run_honey && sensor_state_h){
        //솔레노이드 벨브 정지
        run_honey = false;
        digitalWrite(RELAY_VALVE_H, pin_off);
        Serial.println("honey_relay_stop_err");
      }else if(!sensor_state_h){
        if(!run_honey && !honey_level[1]){
          mesh.sendBroadcast("P=ID=AT+PUMP=3;"); //펌프 켜기
          digitalWrite(RELAY_VALVE_H, pin_on);     //솔레노이드 밸브 켜기
          run_honey = true;
          level_gauge_h = 1;
          mesh.sendBroadcast("SENSOR=RELAY=ON=HONEY=0=0;");
          Serial.println("honey_relay_run");          
        }else{
          if(honey_level[3]){
            if(run_honey){
              //가득참
              //mesh.sendBroadcast("P=ID=AT+PUMP=0;"); //펌프 끄기
              digitalWrite(RELAY_VALVE_H, pin_off);
              run_honey      = false;
              mesh.sendBroadcast("SENSOR=RELAY=OFF=HONEY=0=0;");
              Serial.println("honey_relay_stop_full");
            }
          } else if(honey_level[4]||honey_level[5]){
            //넘침
            //mesh.sendBroadcast("P=ID=AT+PUMP=0;"); //펌프 끄기
            digitalWrite(RELAY_VALVE_H, pin_off);
            mesh.sendBroadcast("SENSOR=ERR=OVERFLOW=HONEY=0=0;");
            Serial.println("honey_relay_stop_overflow");
          } else if(run_honey){ 
            if(!honey_level[level_gauge_h]){
              full_honey++;
              if (full_honey > 240){
                digitalWrite(RELAY_VALVE_H, pin_off);
                run_honey = false;
                sensor_state_h = true;
                ERR_Message = "SENSOR=ERR=EMPTY=HONEY=0=0;";
                mesh.sendBroadcast(ERR_Message);
                mesh.sendBroadcast("SENSOR=RELAY=OFF=HONEY=0=0;");
                Serial.println("honey_relay_stop_timeout");
              }
            } else if(level_gauge_h != 0){
              level_gauge_h += 1;
              if(level_gauge_h > 6){
                level_gauge_h = 0;
                digitalWrite(RELAY_VALVE_W, pin_off);
                run_honey = false;
              }
            }
          }//밸브가 열려있을때 수위가 올라가지 않을경우
        }
      }
    }//use_honey
    mesh.update();
    if(sensor_state_w || sensor_state_h){ //센서가 고장일 경우
      //알람 보내기
      if (err_sensor > 240) {
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
      mesh.sendBroadcast("SENSOR=RELAY=ON=HEAT=0=0;");
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
      mesh.sendBroadcast("SENSOR=RELAY=ON=FAN=0=0;");
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
    if(temperature != 14040 && temperature > 1) {
      ////온도 유지 팬
      if (use_stable_h || use_stable_f) {
        if (use_stable_h) {
          if (temperature/100 < control_temperature - tempGap) {
            if (temp_flage(true, false)) { //히터, 팬

              digitalWrite(RELAY_HEATER, pin_on);
            }
          }else if (temperature/100 >= control_temperature) {
            if (temp_flage(false, false)) { //히터, 팬
              digitalWrite(RELAY_HEATER, pin_off);
            }
          }
        }
        if (use_stable_f) {
          if (temperature/100 > control_temperature + tempGap) {
            if (temp_flage(false, true)) { //히터, 팬
              digitalWrite(RELAY_FAN,    pin_on);
            }
          }else if (temperature/100 <= control_temperature) {
            if (temp_flage(false, false)) { //히터, 팬
              digitalWrite(RELAY_FAN,    pin_off);
            }
          }

        }
      }else{//온도 조절 종료
        digitalWrite(RELAY_HEATER, pin_off);
        digitalWrite(RELAY_FAN,    pin_off);
      }
    } else {
      if (err_stable > 240) {
        ERR_Message = "SENSOR=ERR=TEMP=0=0=0;";
        mesh.sendBroadcast(ERR_Message);
        err_stable  = 0;
      }
      else {
        err_stable++;
      }
    }
  }//millis()
}//stable() END


void get_sensor(unsigned long millisec) {
  if ((millisec - timer_SHT40) > 300) {
    timer_SHT40 = millisec;
    tca_select(6);
    Wire.beginTransmission(68); //0x44
    mesh.update();
    if (!Wire.endTransmission() && sht40.begin()) {
      sensors_event_t humi, temp;
      sht40.getEvent(&humi, &temp);
      temperature  = temp.temperature * 100;
      humidity     = humi.relative_humidity * 100;

    } else {
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
    Serial.print("TCA Port 6");
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
    Serial.println(";");
    mesh.update();
    Serial.print(count_sensor_err_w);
    Serial.print("-water_level: ");
    for (int8_t index = 0 ; index < 6 ; index++) {
      Serial.print(water_level[index]);
      mesh.update();
    }
    Serial.println(";");
    Serial.print(count_sensor_err_h);
    Serial.print("-honey_level: ");
    for (int8_t index = 0 ; index < 6 ; index++) {
      Serial.print(honey_level[index]);
      mesh.update();
    }    
    Serial.println(";");
  }
}

unsigned long timer_restart = 0;
uint8_t restart_count       = 0;
void mesh_restart(unsigned long millisec){
  if(millisec - timer_restart > 1000*60*60){
    timer_restart = millisec;
    if(restart_count++ > 60) ESP.restart();
  }
}
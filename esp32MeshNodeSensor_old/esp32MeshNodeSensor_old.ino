#include  "painlessMesh.h"
#include  "Adafruit_SHT31.h"
#include  "EEPROM.h"
#define   EEPROM_SIZE 4
#define   SERIAL_MAX  64

#define   MESH_PREFIX     "smartHiveMesh"
#define   MESH_PASSWORD   "smarthive123"
#define   MESH_PORT       3333

Scheduler taskScheduler; // to control upload task
painlessMesh  mesh;
Adafruit_SHT31 sht31 = Adafruit_SHT31();

String nodeID = "";
const uint8_t tempGap = 1;
//// ----------- Flage --------------
boolean SHT31     = false;
boolean stableUse = false;
boolean flage_Water = true;
boolean flage_Relay_Water = false;
boolean flage_Heater  = false;
boolean flage_Fan     = false;
//// ----------- Variable -----------
uint8_t control_Temperature = 33;
uint8_t control_Humidity    = 50;
////for millis() func//
unsigned long time_Water  = 0;
unsigned long time_Stalbe = 0;
//// ------------ EEPROM ------------
const uint8_t EEP_Temperature = 1;
const uint8_t EEP_Humidity    = 2;
const uint8_t EEP_Stable      = 3;
//// ------------- PIN --------------
const uint8_t SENSOR_TOP    = 32; //INPUT_PULLDOWN
const uint8_t SENSOR_BOTTOM = 33; //INPUT_PULLDOWN
const uint8_t RELAY_HEATER  = 25;
const uint8_t RELAY_FAN     = 26;
const uint8_t RELAY_VALVE   = 27;
//// ----------- Variable -----------

//// ----------- Command  -----------
void command_Service(String command, String value) {
  if (command == "AT+TEMP") {
    control_Temperature = value.toInt();
    EEPROM.write(EEP_Temperature, control_Temperature);
    mesh.sendBroadcast("SENSOR=SET=TEMP;");
  } else if (command == "AT+HUMI") {
    control_Humidity = value.toInt();
    EEPROM.write(EEP_Humidity, control_Humidity);
    mesh.sendBroadcast("SENSOR=SET=HUMI;");
  } else if (command == "AT+USE") {
    stableUse = value.toInt();
    EEPROM.write(EEP_Stable, stableUse);
    mesh.sendBroadcast("SENSOR=SET=USE;");
  } else if (command == "AT+WATER") {
    flage_Water       = true;
    flage_Relay_Water = false;
    mesh.sendBroadcast("SENSOR=SET=WATER;");
  }
  EEPROM.commit();
}//Command_service() END

//// ----------- TEST  -----------
void AT_commandHelp() {
  Serial.println("------------ AT command help ------------");
  Serial.println(";AT+TEMP = int;      Temperature Change.");
  Serial.println(";AT+HUMI = int;      Humidity Change.");
  Serial.println(";AT+USE  = int;      Useable Change.");
  Serial.println(";AT+WATER= int;      Water Alarm Reset.");
}
char Serial_buf[SERIAL_MAX];
int8_t Serial_num;
void Serial_service() {
  String str1 = strtok(Serial_buf, "=");
  String str2 = strtok(NULL, " ");
  command_Service(str1, str2);
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
Task sensorLog( TASK_SECOND * 60, TASK_FOREVER, &sensorValue );
void sensorValue() {
  String msg = "SENSOR=" + (String)sht31.readTemperature() + "=" + (String)sht31.readHumidity() + ';' ;
  mesh.sendBroadcast( msg );
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
    String device  = strtok(NULL, "=");
    if (device == nodeID) {
      Serial.println(msg);
      String command = strtok(NULL, "=");
      String value   = strtok(NULL, ";");
      command_Service(command, value);
    } else if (device == "connecting"){
      mesh.sendBroadcast("SENSOR=CNT=TRUE;");
    }//echo
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //// ------------ PIN OUT ------------
  pinMode(SENSOR_TOP, INPUT_PULLDOWN);
  pinMode(SENSOR_BOTTOM, INPUT_PULLDOWN);
  pinMode(RELAY_HEATER, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_VALVE, OUTPUT);
  //// ------------ PIN OUT ------------
  if (!EEPROM.begin(EEPROM_SIZE)) Serial.println("failed to initialise EEPROM");
  //// ------------ EEPROM ------------
  control_Temperature = byte(EEPROM.read(EEP_Temperature));
  control_Humidity    = byte(EEPROM.read(EEP_Humidity));
  stableUse           = byte(EEPROM.read(EEP_Stable));
  //// ------------ EEPROM ------------

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &taskScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);

  nodeID = mesh.getNodeId();

  taskScheduler.addTask( sensorLog );
  sensorLog.enable();

  if (!sht31.begin(0x44))delay(1000);
  SHT31 = sht31.begin(0x44);

  Serial.print("System online, Set temperature is ");
  Serial.print(control_Temperature);
  Serial.print(", Set humidity is ");
  Serial.print(control_Humidity);

  if (!SHT31) {
    Serial.print("Sensor error");
    mesh.sendBroadcast("SENSOR=ERR=SHT31;");
  }
  
  Serial.print(", Set Operation : ");
  Serial.println(stableUse);

  Serial.print("Device nodeID = ");
  Serial.println(nodeID);
  AT_commandHelp();
}

void loop() {
  mesh.update();
  //sensor_Water();
  stable();
  if (Serial.available()) {
    Serial_process();
  }
}

void water_flage(boolean passfail) {
  if (flage_Water) flage_Relay_Water = passfail;
}

uint8_t full_Water  = 0;
void sensor_Water() {
  if ((millis() - time_Water) > 1000 * 1) {
    time_Water = millis();
    if ((!flage_Relay_Water) && (digitalRead(SENSOR_BOTTOM) == LOW) && (digitalRead(SENSOR_TOP) == LOW)) {
      digitalWrite(RELAY_VALVE, HIGH);
      mesh.sendBroadcast("P=ID=AT+PUMP=3;");
      water_flage(true);
      full_Water++;
      Serial.println("Water relay On");
    } else if (flage_Relay_Water && (digitalRead(SENSOR_BOTTOM) == HIGH) && (digitalRead(SENSOR_TOP) == LOW)) {
      if ((full_Water < 250) && flage_Relay_Water) full_Water++;
    } else if (flage_Relay_Water && (digitalRead(SENSOR_BOTTOM) == HIGH) && (digitalRead(SENSOR_TOP) == HIGH)) {
      //mesh.sendBroadcast("P=ID=AT+PUMP=0;"); //펌프 끄기
      digitalWrite(RELAY_VALVE, LOW);
      Serial.println("Water charging");
      water_flage(false);
      full_Water = 0;
      Serial.println("Water full");
    } else if ((full_Water > 240) || ((digitalRead(SENSOR_BOTTOM) == LOW) && (digitalRead(SENSOR_TOP) == HIGH))) {
      mesh.sendBroadcast("SENSOR=ERR=WATER;");
      digitalWrite(RELAY_VALVE, LOW);
      flage_Water = false;
      water_flage(false);
      full_Water = 0;
      Serial.println("Water ERR log");
    } else if (full_Water > 0) {
      full_Water++;
      Serial.print("WROT : ");
      Serial.println(full_Water);
    }
  }//millis()
}//sensor_Water() END

uint8_t err_Stable    = 0;
boolean temp_flage(boolean onoff_Heater, boolean onoff_Fan) {
  if (flage_Heater == onoff_Heater && flage_Fan == onoff_Fan) {
    return false;
  }
  if (flage_Heater != onoff_Heater) {
    flage_Heater  = onoff_Heater;
    if (onoff_Heater == true) {
      Serial.println("Heater on");
    }
    else {
      Serial.println("Heater off");
    }
  }
  if (flage_Fan != onoff_Fan) {
    flage_Fan     = onoff_Fan;
    if (onoff_Fan == true) {
      Serial.println("Fan on");
    }
    else {
      Serial.println("Fan off");
    }
  }
  return true;
}

void stable() {
  if ((millis() - time_Stalbe) > 1000 * 1) {
    String  ERR_Message = "SENSOR=ERR=SHT31;";
    time_Stalbe = millis();
    if (SHT31) {
      ////센서 계측
      uint8_t Temperature = sht31.readTemperature();
      uint8_t Humidity    = sht31.readHumidity();
      Serial.print("temperature:"); Serial.print(Temperature); Serial.print(" , Humidity:"); Serial.println(Humidity);
      ////온도 유지
      if (stableUse) {
        if (Temperature > control_Temperature + tempGap) {
          ERR_Message = "SENSOR=ERR=TEMP;";
          if (temp_flage(false, true)) { //히터, 팬
            digitalWrite(RELAY_HEATER, flage_Heater);
            digitalWrite(RELAY_FAN, flage_Fan);
            Serial.println("TEMP EMERGENCY");
          }
          Serial.println(ERR_Message);
        } else if (Temperature >= control_Temperature) {
          if (temp_flage(false, false)) { //히터, 팬
            digitalWrite(RELAY_HEATER, flage_Heater);
            digitalWrite(RELAY_FAN, flage_Fan);
          }
        } else if (Temperature < control_Temperature - tempGap) {
          if (temp_flage(true, false)) { //히터, 팬
            digitalWrite(RELAY_HEATER, flage_Heater);
            digitalWrite(RELAY_FAN, flage_Fan);
          }
        }//온도 조절 종료
      }
    } else {
      if (err_Stable > 240) {
        mesh.sendBroadcast(ERR_Message);
        err_Stable = 0;
      }
      else {
        err_Stable++;
      }
    }
  }//millis()
}//stable() END

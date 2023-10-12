#include <WiFi.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient
#include <HTTPClient.h>
#define SERIAL_MAX  64

const char* ssid      = "CNR_L580W_2CD944";
const char* password  = "#234567!";

const char* mqttServer    = "smarthive.kr";
const int   mqttPort      = 1883;
const char* mqttUser      = "hive";
const char* mqttPassword  = "hive";
const char* topic_pub     = "SHS";
char        deviceID[18];
char        sendID[21]    = "ID=";

WiFiClient mqtt_client;
PubSubClient mqttClient(mqtt_client);

#include "Wire.h"
#define  TCAADDR 0x70
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define PCA9539_H_
#include "Arduino.h"
#define DEBUG 1
#define NXP_INPUT      0
#define NXP_OUTPUT     2
#define NXP_INVERT     4
#define NXP_CONFIG     6

class PCA9539 {
  public:
    PCA9539(uint8_t address);                            // constructor
    void pinMode(uint8_t pin, uint8_t IOMode );          // pinMode
    uint8_t digitalRead(uint8_t pin);                    // digitalRead
    void digitalWrite(uint8_t pin, uint8_t value );      // digitalWrite

  private:
    uint16_t I2CGetValue(uint8_t address, uint8_t reg);
    void I2CSetValue(uint8_t address, uint8_t reg, uint8_t value);

    union {
      struct {
        uint8_t _configurationRegister_low;          // low order byte
        uint8_t _configurationRegister_high;         // high order byte
      };
      uint16_t _configurationRegister;                 // 16 bits presentation
    };
    union {
      struct {
        uint8_t _valueRegister_low;                  // low order byte
        uint8_t _valueRegister_high;                 // high order byte
      };
      uint16_t _valueRegister;
    };
    uint8_t _address;                                    // address of port this class is supporting
    int _error;                                          // error code from I2C
};

PCA9539::PCA9539(uint8_t address) {
  _address         = address;        // save the address id
  _valueRegister   = 0;
  Wire.begin();                      // start I2C communication
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
    _valueRegister = _valueRegister | (1 << pin);    // and OR bit in register
  } else {
    _valueRegister = _valueRegister & ~(1 << pin);    // AND all bits
  }
  I2CSetValue(_address, NXP_OUTPUT    , _valueRegister_low);
  I2CSetValue(_address, NXP_OUTPUT + 1, _valueRegister_high);
}

uint16_t PCA9539::I2CGetValue(uint8_t address, uint8_t reg) {
  uint16_t _inputData;
  Wire.beginTransmission(address);          // setup read registers
  Wire.write(reg);
  _error = Wire.endTransmission();
  if (Wire.requestFrom((int)address, 1) != 1)
  {
    return 256;                            // error code is above normal data range
  };
  _inputData = Wire.read();
  return _inputData;
}

void PCA9539::I2CSetValue(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);              // setup direction registers
  Wire.write(reg);                              // pointer to configuration register address 0
  Wire.write(value);                            // write config register low byte
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

//// ----------- Variable -----------
const boolean pin_on  = false;
const boolean pin_off = true;
boolean run_log       = false;
boolean led_show      = false;

struct dataSet {
  String TYPE;
  String MODULE;
  String COMMEND;
  String VALUE1;
  String VALUE2;
  String VALUE3;
  String VALUE4;
};

struct dataSet dataSend[3] = {
  {":01","SENSOR","LOG","1404","1404","0","0"},
  {":02","SENSOR","LOG","1404","1404","0","0"},
  {":03","SENSOR","LOG","1404","1404","0","0"}
};
uint8_t sensor_socket[3] = {2,4,6};

void get_sensor(uint_8t sensor_number) {
  tca_select(sensor_socket[sensor_number]);
  Wire.beginTransmission(68); //0x44
  if (!Wire.endTransmission() && sht40.begin()) {
    sensors_event_t humi, temp;
    sht40.getEvent(&humi, &temp);
    dataSend[sensor_number].VALUE1  = temp.temperature * 100;
    dataSend[sensor_number].VALUE2  = humi.relative_humidity * 100;
  } else {
    dataSend[sensor_number].VALUE1  = 14040;
    dataSend[sensor_number].VALUE2  = 14040;
  }
}

void save_sensor_value() {
    get_sensor(0);
    get_sensor(1);
    get_sensor(2);
}

void routine_sensor(unsigned long millisec) {
  if ((millisec - timer_SHT40) > 300) {
    timer_SHT40 = millisec;
    save_sensor_value();
  }//if
}

int16_t temperature[3] = {14040,};
int16_t humidity[3]    = {14040,};
//// ------------- PIN --------------
const uint8_t RELAY_HEATER  = 12;
const uint8_t RELAY_FAN     = 16;
const uint8_t RELAY_VALVE_H = 13;
const uint8_t RELAY_VALVE_W = 14;
const uint8_t BUILTIN_LED_A = 17;
const uint8_t BUILTIN_LED_B = 25;
////for millis() func//
unsigned long time_led_show = 0UL;
unsigned long timer_SHT40   = 0UL;
unsigned long timer_post    = 0UL;
//// ----------- Variable -----------

//// ------------ MQTT Callback ------------
void callback(char* topic, byte* payload, unsigned int length) {
  char mqtt_buf[SERIAL_MAX] = "";
  int8_t Serial_num = 0;
  for (int i = 0; i < length; i++) {
    switch ( payload[i] ) {
      case ';':
        Serial_buf[Serial_num] = 0x00;
        mqtt_service();
        Serial_num = 0;
        break;
      default :
        Serial_buf[ Serial_num ++ ] = ch;
        Serial_num %= SERIAL_MAX;
        break;
    }
  }
  Serial.println(mqtt_buf);
}

//// ----------- Command  -----------
void AT_commandHelp() {
  Serial.println("------------ AT command help ------------");
  Serial.println(";AT+RELAY=  bool;     All relay on or off test.");
  Serial.println(";AT+LOG=    bool;     Serial log view");
  Serial.println(";AT+SHOW=   bool;     Builtin led on off");
}

void mqtt_service() {
  String str1 = strtok(Serial_buf, "=");
  String str2 = strtok(0x00, " ");
  command_Service(str1, str2);
}

void command_Service(String command, String value) {
  if (command == "AT+RELAY") {
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
    led_show = (value.toInt() > 0) ? true : false;
  }
  Serial.print("AT command:");
  Serial.print(command);
  Serial.print("=");
  Serial.println(value);
}

void setup() {
  Serial.begin(115200);

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  for (int i = 0; i < 17; i++) {
    sendID[i + 3] = WiFi.macAddress()[i];
    deviceID[i]   = sendID[i + 3];
  }
  
  char* topic_sub = deviceID;
  char* sub_ID    = sendID;
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect(deviceID, mqttUser, mqttPassword )) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }

  mqttClient.subscribe(topic_sub);
  mqttClient.publish(topic_pub, sub_ID);

  Serial.print("subscribe: ");
  Serial.print(topic_sub);
  Serial.println(" - MQTT Connected");

  save_sensor_value();
  send_http_post();
}//End Of Setup()

void reconnect(){
  char* topic_sub = deviceID;
  char* sub_ID    = sendID;
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect(deviceID, mqttUser, mqttPassword )) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
}

void loop() {
  unsigned long millisec = millis();
  if (mqttClient.connected()){mqttClient.loop();}
  else{reconnect();}
  routine_sensor(millisec);
  routine_http_post(millisec);
}

void httpPOSTRequest(struct dataSet *ptr) {
  String serverUrl = "http://dev.smarthive.kr/reg/hive/";   //API adress
  HTTPClient http;
  WiFiClient http_client;
  http.begin(http_client, serverUrl);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String httpRequestData = (String)"FARM="  + deviceID     +
                           "&MODULE="       + deviceID     + ptr->MODULE  +
                           "&TYPE="         + ptr->TYPE    +
                           "&COMMEND="      + ptr->COMMEND +
                           "&VALUE1="       + ptr->VALUE1  +
                           "&VALUE2="       + ptr->VALUE2  +
                           "&VALUE3="       + ptr->VALUE3  +
                           "&VALUE4="       + ptr->VALUE4;

  int httpResponseCode = http.POST(httpRequestData);
  Serial.print(httpRequestData);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  http.end();           // Free resources
}////httpPOSTRequest_End

void send_http_post(){
  httpPOSTRequest(dataSend[0]);
  httpPOSTRequest(dataSend[1]);
  httpPOSTRequest(dataSend[2]);
}

void routine_http_post(unsigned long millisec){
  if ((millisec - timer_SHT40) > 1000*60*5) {
    timer_SHT40 = millisec;
    send_http_post();
  }//if
}
#include <WiFi.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient
#include <HTTPClient.h>
#define SERIAL_MAX  64
/*
const char* ssid      = "CNR_L580W_2CD944";
const char* password  = "#234567!";
*/
const char* ssid      = "Daesung2G";
const char* password  = "smarthive123";

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

#include  "Adafruit_SHT31.h"
Adafruit_SHT31 sht31 = Adafruit_SHT31();

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
uint8_t led_show      = 250;
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

struct dataSet {
  String TYPE;
  String MODULE;
  String COMMEND;
  String VALUE1;
  String VALUE2;
  String VALUE3;
  String VALUE4;
};

struct dataSet dataSend[4] = {
  {"SENSOR",":01","LOG","1404","1404","0","0"},
  {"SENSOR",":02","LOG","1404","1404","0","0"},
  {"SENSOR",":03","LOG","1404","1404","0","0"},
  {"SENSOR",":04","LOG","1404","1404","0","0"}
};
uint8_t sensor_socket[4] = {0,2,4,6};
const uint8_t sensor_quantity = 4;

void get_sensor(uint8_t sensor_number) {
  tca_select(sensor_socket[sensor_number]);
  Wire.beginTransmission(68); //0x44
  if (!Wire.endTransmission() && sht31.begin()) {
    dataSend[sensor_number].VALUE1  = int16_t(sht31.readTemperature() * 100);
    dataSend[sensor_number].VALUE2  = int16_t(sht31.readHumidity() * 100);
  } else {
    dataSend[sensor_number].VALUE1  = 14040;
    dataSend[sensor_number].VALUE2  = 14040;
  }
}

void save_sensor_value() {
  for(uint8_t index=0; index<sensor_quantity; index++){
    get_sensor(index);
  }
}

void routine_sensor(unsigned long millisec) {
  if ((millisec - timer_SHT40) > 300) {
    timer_SHT40 = millisec;
    save_sensor_value();
  }//if
}

//// ------------ MQTT Callback ------------
void callback(char* topic, byte* payload, unsigned int length) {
  char mqtt_buf[SERIAL_MAX] = "";
  int8_t mqtt_num = 0;
  for (int i = 0; i < length; i++) {
    switch (payload[i]) {
      case ';':
        mqtt_buf[mqtt_num] = 0x00;
        mqtt_service(mqtt_buf);
        mqtt_num = 0;
        break;
      default :
        mqtt_buf[ mqtt_num ++ ] = payload[i];
        mqtt_num %= SERIAL_MAX;
        break;
    }
  }
}

char Serial_buf[SERIAL_MAX];
int8_t Serial_num;
void Serial_process() {
  char ch;
  ch = Serial.read();
  switch ( ch ) {
    case ';':
      Serial_buf[Serial_num] = 0x00;
      mqtt_service(Serial_buf);
      Serial_num = 0;
      break;
    default :
      Serial_buf[ Serial_num ++ ] = ch;
      Serial_num %= SERIAL_MAX;
      break;
  }
}

//// ----------- Command  -----------
void AT_commandHelp() {
  Serial.println("------------ AT command help ------------");
  Serial.println(";AT+HELP=   bool;     this messege.");
  Serial.println(";AT+RELAY=  bool;     All relay on or off test.");
  Serial.println(";AT+LOG=    bool;     Serial log view");
  Serial.println(";AT+SHOW=   bool;     Builtin led on off");
}

void mqtt_service(char* service_buf) {
  String str1 = strtok(service_buf, "=");
  String str2 = strtok(0x00, " ");
  command_Service(str1, str2);
}

void command_Service(String command, String value) {
  if (command == "AT+HELP") {
    AT_commandHelp();
  }else if (command == "AT+RELAY") {
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
  Serial.print("AT command:");
  Serial.print(command);
  Serial.print("=");
  Serial.println(value);
}

boolean led_status = false;
void builtin_led(unsigned long millisec){
  if ((millisec - time_led_show) > 500) {
    if(led_show<250){
      time_led_show = millisec;
      digitalWrite(BUILTIN_LED_A, led_status);
      digitalWrite(BUILTIN_LED_B, !led_status);
      led_show ++;
      led_status = !led_status;
    }else{
      digitalWrite(BUILTIN_LED_A, pin_off);
      digitalWrite(BUILTIN_LED_B, pin_off);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUILTIN_LED_A, OUTPUT);
  pinMode(BUILTIN_LED_B, OUTPUT);

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
  AT_commandHelp();
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
  if (Serial.available()) {Serial_process();}
  if (mqttClient.connected()){mqttClient.loop();}
  else{reconnect();}
  routine_sensor(millisec);
  routine_http_post(millisec);
  serial_monit(millisec);
  builtin_led(millisec);
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
  for(uint8_t index=0; index<sensor_quantity; index++){
    httpPOSTRequest(&dataSend[index]);
  }
}

void routine_http_post(unsigned long millisec){
  if ((millisec - timer_post) > 1000*60*5) {
    timer_post = millisec;
    send_http_post();
  }//if
}

unsigned long timer_serial_monit = 0;
void serial_monit(unsigned long millisec){
  if (run_log && ((millisec - timer_serial_monit) > 1000)) {
    timer_serial_monit = millisec;
    for (uint8_t index = 0; index < sensor_quantity; index++){
      Serial.print("TCA Port");
      Serial.print(sensor_socket[index]);
      Serial.print(", T: ");
      Serial.print(dataSend[index].VALUE1);
      Serial.print("°C ,H: ");
      Serial.print(dataSend[index].VALUE2);
      Serial.println("%");
    }
    Serial.println("\r\n");
  }
}
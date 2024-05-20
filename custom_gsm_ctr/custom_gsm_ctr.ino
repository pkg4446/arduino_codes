#include <EEPROM.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient

#define TOTAL_RELAY 10
#define EEPROM_SIZE 16
#define COMMAND_LENGTH  16
#define UPDATE_INTERVAL 1000L

HardwareSerial nextion(2);
enum Func {
    Water_A = 0,
    Water_B,
    Cooler,
    Heater,
    Lamp_A,
    Lamp_B,
    Lamp_C,
    Pump,
    Spare_A,
    Spare_B
};
/***************EEPROM*********************/
const uint8_t eep_ssid[EEPROM_SIZE] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
const uint8_t eep_pass[EEPROM_SIZE] = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
/***************EEPROM*********************/
/***************MQTT_CONFIG****************/
const char*     mqttServer    = "smarthive.kr";
const uint16_t  mqttPort      = 1883;
const char*     mqttUser      = "hive";
const char*     mqttPassword  = "hive";
const char*     topic_pub     = "SHS";
WiFiClient    mqtt_client;
PubSubClient  mqttClient(mqtt_client);
uint8_t       mqtt_count      = 0;
/***************MQTT_CONFIG****************/
/***************PIN_CONFIG*****************/
const int8_t Relay[TOTAL_RELAY] = {2,4,5,12,13,23,27,26,25,33};
/***************PIN_CONFIG*****************/
/***************Interval_timer*************/
unsigned long prevUpdateTime = 0L;
/***************Interval_timer*************/
/***************Variable*******************/
char ssid[EEPROM_SIZE];
char password[EEPROM_SIZE]; //#234567!
/***************Variable*******************/
/***************Functions******************/
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  char mqtt_buf[COMMAND_LENGTH] = "";
  for (int i = 0; i < length; i++) {
    mqtt_buf[i] = payload[i];
  }
  Serial.print("Message arrived: ");
  rootDvice.print(mqtt_buf);
  Serial.println(mqtt_buf);
}
/***************Functions******************/
void command_helf_wifi() {
  Serial.println("********** help **********");
  Serial.println("help  this text");
  Serial.println("show  wifi scan");
  Serial.println("ssid  ex)ssid your ssid");
  Serial.println("pass  ex)pass your password");
  Serial.println("********** help **********");
  Serial.print("your ssid: "); Serial.println(ssid);
  Serial.print("your pass: "); Serial.println(password);
  Serial.println("------- wifi config -------");
}
/***************Functions******************/

void setup() {
  Serial.begin(115200);
  rootDvice.begin(115200, SERIAL_8N1, 18, 19);

  for (uint8_t index = 0; index < TOTAL_RELAY; index++)
  {
    pinMode(Relay[index], OUTPUT);
    digitalWrite(Relay[index], false);
  }

  Serial.println("Custom Board done.");
}

// the loop function runs over and over again forever
void loop() {
  delay(1);
}
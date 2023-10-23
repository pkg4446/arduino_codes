#include "pin_setup.h"
#include "shift_regs.h"
#include "moter_control.h"

#include <SPI.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <Ethernet.h>
#include <EthernetClient.h>
#include <Dns.h>
#include <Dhcp.h>

#include <ArduinoJson.h>

#define DEBUG
#define DEBUG_MQTT
//#define DEBUG_SHIFT_REGS

/************************* Ethernet Client Setup *****************************/
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
/************************* Mqtt Server Setup *********************************/
#define AIO_SERVER      "mqtt.kro.kr"
#define AIO_SERVER_PORT  1883
#define AIO_USERNAME    "test"
#define AIO_KEY         "test"
//Set up the ethernet client
EthernetClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVER_PORT, AIO_USERNAME, AIO_KEY);
// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }
// Setup a mqtt 
Adafruit_MQTT_Publish   response = Adafruit_MQTT_Publish(&mqtt,  "arduino_test_p");
Adafruit_MQTT_Subscribe request  = Adafruit_MQTT_Subscribe(&mqtt, "arduino_test_s");
/************************* Mqtt End *********************************/

MOTOR driver[4];

void init_port_base(){
  for (uint8_t index=0 ; index<7; index++) {
    pinMode(p_relay[index],OUTPUT);
    digitalWrite(p_relay[index], false);
  }
}

//**********MQTT**********//
void MQTT_connect() {
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  int8_t ret;
  Serial.println("Connecting to MQTT.");
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds.");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}

void mqtt_requeset(){
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(1))) {
    if (subscription == &request) {
      String receive = (char *)request.lastread;
      #ifdef DEBUG_MQTT
        Serial.print("MQTT: ");
        Serial.println(receive);
      #endif
      StaticJsonDocument<256> json;
      deserializeJson(json, receive);
      String temp_name = json["name"];
      Serial.println(temp_name);
    }
  }
}

void mqtt_response(const char* send_data){
  if (! response.publish(send_data)) {
    Serial.println("Failed");
  } else {
    Serial.println("OK!");
  }
}
//**********End Of MQTT**********//

//********** setup**********//
void setup() {
  Serial.begin(115200);
  Serial.println("System boot... wait a few seconds.");
  Ethernet.begin(mac);
  shift_regs_init();
  for (uint8_t index=0 ; index<4; index++) {
    driver[index].init(stepDriver[index]);
    driver[index].status();
  }
  init_port_base();
  mqtt.subscribe(&request);
}//********** End Of Setup() **********//

//********** loop **********//
void loop() {
  //if need asynchronous, add something
  MQTT_connect();
  mqtt_requeset();
  #ifdef DEBUG_SHIFT_REGS
    display_pin_values();
  #endif
}//**********End Of loop()**********//


#ifdef DEBUG_SHIFT_REGS
unsigned long interval_74HC165 = 0L;
void display_pin_values()
{
    BYTES_VAL_T pinValues = read_shift_regs();
    if(millis() > interval_74HC165 + 1000){
      interval_74HC165 = millis();
      Serial.print("Pin States:\r\n");
      for(int i = 0; i < DATA_WIDTH; i++)
      {
          Serial.print("  Pin-");
          Serial.print(i);
          Serial.print(": ");
          if((pinValues >> i) & 1)
              Serial.print("HIGH");
          else
              Serial.print("LOW");
          Serial.print("\r\n");
      }
      Serial.print("done.\r\n");
    }
}
#endif
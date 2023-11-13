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

#define JSON_STACK 144

#define DEBUG
#define DEBUG_MQTT
//#define DEBUG_SHIFT_REGS

/************************* Mqtt End *********************************/

/************************* values *********************************/
MOTOR driver[4];
MOTOR builtin[4];
unsigned long relay_start_time[7] = {0UL,};
unsigned long relay_end_time[7]   = {0UL,};
bool relay_state[7];
bool online = false;
/************************* values *********************************/


/***************/
/***************/
/***************/
/***************/

void init_port_base(){
  for (uint8_t index=0 ; index<7; index++) {
    pinMode(relay_pin[index],OUTPUT);
    digitalWrite(relay_pin[index], false);
    relay_state[index] = false;
  }
}


  uint8_t dir[6]  = {PIN_PL6,PIN_PE6,PIN_PE2,PIN_PE7,PIN_PH7,PIN_PL7};
  uint8_t step[6] = {PIN_PE5,PIN_PH4,PIN_PE3,PIN_PH5,PIN_PE4,PIN_PH3};

  uint8_t dirconfirm[] = {1,1,1,PIN_PL7,1,1};
  uint8_t stepconfirm[] = {PIN_PE5,PIN_PE6,1,PIN_PE4,PIN_PE3,1};
  
  uint8_t stepcheck[] = {PIN_PL6,PIN_PH4,PIN_PE2,PIN_PE7,PIN_PH5,PIN_PH7,PIN_PL7,PIN_PH3};

//********** setup**********//
void setup() {
  Serial.begin(115200);
  Serial.println("System boot... wait a few seconds.");

  /********************
  {PIN_PL6,PIN_PE5},   ,0
  {PIN_PE6,PIN_PH4},  2,
  {PIN_PE2,PIN_PE3},  ,5
  {PIN_PE7,PIN_PH5},  ,4 
  {PIN_PH7,PIN_PE4},  ,4
  {PIN_PL7,PIN_PH3}
  *********************/
  for(uint8_t index=0; index<6; index++){
    pinMode(dir[index],OUTPUT);
    pinMode(step[index],OUTPUT);
  }
  for(uint8_t index=0; index<8; index++){
    digitalWrite(stepcheck[index], false);
  }
  
  pinMode(BUITIN_EN,OUTPUT);
  digitalWrite(BUITIN_EN, true);
  //digitalWrite(PIN_PL7, true);
  
  //digitalWrite(52, true);
}//********** End Of Setup() **********//

//********** loop **********//
uint8_t index = 0;
uint8_t index_num = 0;
void loop() {
  digitalWrite(stepconfirm[4], true);
  digitalWrite(stepconfirm[4], false);
  delay(5);
  
  if(index_num++ > 200){
    Serial.println(index);
    
    digitalWrite(stepcheck[index], true);
    index_num = 0;
    index ++;
    
    if(index > 8 ){
      index = 0;
    }
  }

}//**********End Of loop()**********//

#include "pin_setup.h"

#include <SPI.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <Ethernet.h>
#include <EthernetClient.h>
#include <Dns.h>
#include <Dhcp.h>

#include <Arduino_JSON.h>

#define DEBUG
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

#define SERIAL_MAX 16
#define NUMBER_OF_SHIFT_CHIPS 2
#define DATA_WIDTH  NUMBER_OF_SHIFT_CHIPS * 8

//DS=PL2=47, this pin used when using more than two
#define LOAD 49 // PL=PL0=49 //out
#define DIN  48 // Q7=PL1=48 //in
#define CLK  46 // CP=PL3=46 //out
#define CE   16 // CE=PH1=16 //out
////pin set

const uint8_t step_cw_limit[4]  = {0,1,2,3};
const uint8_t step_ccw_limit[4] = {4,5,6,7};
//value
uint32_t hight_max = 9999999; // Initial speed for acceleration and deceleration
#define BYTES_VAL_T uint16_t

//출력 - 브레이크 릴레이 3개, 스탭모터 핀 2(cw,ccw)*3개, 로봇 제어용 릴레이 2개.
//입력 - 컵 유무(정전용량 센서) 3개, 스텝모터 리미트 2*3개

void init_port_base(){
  /*
  DDRA |= 0x3F; //0b00111111
  DDRB |= 0x3C; //0b00111100
  DDRC |= 0xF0; //0b11110000
  //All output pins false
  PORTA &= 0xC0; //0b11000000
  PORTB &= 0xC3; //0b11000011
  PORTC &= 0x0F; //0b00001111
  //74HC165
  DDRL |= 0x0B; //0b00001001 //LOAD,CLK Out
  DDRL &= 0xFD; //0b11111101 //DIN In
  DDRH |= 0x02; //0b00000010 //CE Out
  //74HC165 set
  PORTL &= 0xF7; //0b11110111 //CLK false
  PORTL |= 0x01; //0b00000001 //LOAD true
  PORTH &= 0xFD; //0b11111101 //CE false
  */
  for (uint8_t index=0 ; index<7; index++) {
    pinMode(p_relay[index],OUTPUT);
    digitalWrite(p_relay[index], false);
  }
  for (uint8_t index=0 ; index<4; index++) {
    pinMode(step_cw[index],OUTPUT);
    pinMode(step_ccw[index],OUTPUT);
    digitalWrite(step_cw[index], false);
    digitalWrite(step_ccw[index], false);
  }
  pinMode(LOAD,OUTPUT);
  pinMode(DIN,INPUT);
  pinMode(CLK,OUTPUT);
  pinMode(CE,OUTPUT);
  digitalWrite(CLK,  false);
  digitalWrite(LOAD, true);
  digitalWrite(CE,   false);
}

BYTES_VAL_T read_shift_regs()
{
    BYTES_VAL_T bitVal;
    BYTES_VAL_T bytesVal = 0;
    digitalWrite(LOAD, false);  //remove_noize
    delayMicroseconds(1);
    digitalWrite(LOAD, true);
    for(int i = 0; i < DATA_WIDTH; i++)
    {
        bitVal = digitalRead(DIN);
        bytesVal |= (bitVal << ((DATA_WIDTH-1) - i));
        digitalWrite(CLK, true);
        digitalWrite(CLK, false);
    }
    return(bytesVal);
}

boolean swich_values(uint8_t pin)
{
  BYTES_VAL_T pinValues = read_shift_regs();
  if((pinValues >> pin) & 1){
    return true;
  }else{
    return false;
  }
}

void steper(uint8_t number, bool direction, uint32_t step, uint8_t celeration, uint8_t speed_max, uint16_t speed_min){
  //브레이크 풀기 후 딜레이 추가.
  boolean celerations = false;
  float  speed_change = 0.0;
  if(celeration < 1){celeration = 1;}
  else if(celeration > 45){celeration = 45;}
  if(speed_max < 1){speed_max = 1;}
  const float percent = celeration/100.0;
  const float section = step*percent;
  if(speed_min > speed_max){
    celerations  = true;
    speed_change = ((speed_min - speed_max) / section);
  }

  #ifdef DEBUG
  Serial.print("zero_set: ");
  Serial.print(zero_set[number]);
  Serial.print(", position: ");
  Serial.print(position[number]);
  Serial.print(", celeration: ");
  Serial.print(percent);
  Serial.print(", section: ");
  Serial.print(section);
  Serial.print(", speed_change: ");
  Serial.println(speed_change);
  #endif

  float speed       = speed_min;
  uint32_t distance = uint32_t(section)+1;
  uint16_t adjust   = speed_min;

  if(direction){ //up
    for (uint32_t index=0; index < step; index++) {
      //speed change
      if(zero_set[number] && (position[number] < hight_max)){
        if(celerations){
          if(index < distance){
            if(speed > speed_max){speed -= speed_change;}
            else{speed = speed_max;}
            adjust = uint16_t(speed);
          }else if(index > (step - distance)){
            speed += speed_change;
            adjust = uint16_t(speed);
          }else{
            adjust = uint16_t(speed); //for delay (move smoth)
          }
        }
      }else{
        adjust = 0;
        //---------check this**********----------- limit sw pin
        if(swich_values(step_cw_limit[number])){
          zero_set[number] = true;
          position[number] = hight_max;
          break; //when push the limit sw, stop
        }
      }
      //---------check this**********---------- max hight
      //if(swich_values(step_cw_limit[number])) break; //when push the limit sw, stop
      //if(position[number] > 1000) break; //if position is higher than maximum hight, stop
      digitalWrite(step_cw[number], true);
      position[number] += 1;
      digitalWrite(step_cw[number], false);
      delayMicroseconds(adjust);
    }
  }else if(!direction){  //down
    for (uint32_t index=0; index<step; index++) {
      //speed change
      if(zero_set[number]){
        if(celerations){
          if(index < distance){
            if(speed >speed_max){speed -= speed_change;}
            else{speed = speed_max;}
            adjust = uint16_t(speed);
          }else if(index > (step - distance)){
            speed += speed_change;
            adjust = uint16_t(speed);
          }else{
            adjust = uint16_t(speed); //for delay (move smoth)
          }
        }
      }else{
        adjust = 0;
        //---------check this**********----------- limit sw pin
        if(swich_values(step_ccw_limit[number])){
          zero_set[number] = true;
          position[number] = 0;
          break; //when push the limit sw, stop
        }
      }
      digitalWrite(step_ccw[number], true);
      if(position[number] > 0){
        position[number] -= 1;
      }else{
        zero_set[number] = false;
      }
      digitalWrite(step_ccw[number], false);
      delayMicroseconds(adjust);
    }
  }else{
    #ifdef DEBUG
    Serial.println("need zero set");
    #endif
  }
  #ifdef DEBUG
  Serial.print("position: ");
  Serial.println(position[number]);
  //딜레이 추가 후 브레이크 잠금 추가.
  #endif
}

//*********command chage the Something**********************//
char    Serial_buf[SERIAL_MAX];
uint8_t Serial_num;
unsigned long pre_loop_1 = 0UL;

void Serial_process() {
  char ch;
  ch = Serial.read();
  if(ch==0x03 && Serial_num>10){
    Serial_buf[Serial_num] = 0x00;
    command_Service(Serial_buf);
    Serial_num = 0;
  }else if(ch==0x02 && (Serial_num==0 || Serial_num>11)){
    Serial_num = 0;
    Serial_buf[10]=0x00;
  }else{
    Serial_buf[ Serial_num++ ] = ch;
    Serial_num %= SERIAL_MAX;
  }
}

void command_Service(char* commend_buf) {
  //check sum
  char checksum = commend_buf[0]^commend_buf[1]^commend_buf[2]^commend_buf[3]^commend_buf[4]^commend_buf[5]^commend_buf[6]^commend_buf[7]^commend_buf[8]^commend_buf[9];
  
  #ifdef DEBUG
  Serial.print("checksum=");
  Serial.print(uint8_t(checksum));
  Serial.print("=");
  Serial.println(uint8_t(commend_buf[10]));
  #endif

  char res[11] = "checksum===";
       res[9] = checksum;
  mqtt_response(res);
  //run
  if(checksum == commend_buf[10]){
    //command
    //0,0x02:start/0,0x4D/1,0x46(0x42):direction/2,0x00~0x03:moter/3,celeration:1~100/4,speed_max/5,speed_min*255/6,speed_min
    //7,step1*255*255/8,step2:*255/9,step3/10,checksum/11,0x03:end/
    if(commend_buf[0]==0x4D){//M
      uint32_t step = uint32_t(commend_buf[7])*256*256 + uint16_t(commend_buf[8])*256 + uint8_t(commend_buf[9]);
      boolean  direction = false;
      if(commend_buf[1]==0x46) direction = true;
      uint8_t  motor_num  = uint8_t(commend_buf[2]);
      uint8_t  celeration = uint8_t(commend_buf[3]);
      uint8_t  speed_max  = uint8_t(commend_buf[4]);
      uint16_t speed_min  = uint16_t(commend_buf[5])*255 + uint8_t(commend_buf[6]);
      //
      #ifdef DEBUG
      Serial.print("step=");
      Serial.print(step);
      Serial.print(", direction=");
      Serial.print(direction);
      Serial.print(", motor_num=");
      Serial.print(motor_num);
      Serial.print(", celeration=");
      Serial.print(celeration);
      Serial.print(", speed_max=");
      Serial.print(speed_max);
      Serial.print(", speed_min=");
      Serial.println(speed_min);
      //
      //steper(motor_num, direction, step, celeration, speed_max, speed_min);
      //
      Serial.println("motor moved");
      #endif
    }
  }else{
    //response
    #ifdef DEBUG
    Serial.println("commend wrong");
    #endif
  }
}//Command_service() END

//********** setup**********//
void setup() {
  Serial.begin(115200);
  Serial.println("System boot... wait a few seconds.");
  Ethernet.begin(mac);
  init_port_base();
  delay(1000); //give the ethernet a second to initialize
  mqtt.subscribe(&request);
}//********** End Of Setup() **********//

//********** loop **********//
void loop() {
  if (Serial.available()) {
    Serial_process();
  }
  MQTT_connect();
  mqtt_requeset();
  //if need asynchronous, add something
  #ifdef DEBUG
  display_pin_values();
  #endif
}//**********End Of loop()**********//

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
      char Mqtt_buf[SERIAL_MAX];
      uint8_t Mqtt_num = 0;      
      for (uint8_t index=0; index<receive.length(); index++) {
        if(receive[index]==0x03 && Mqtt_num>10){                        //
          Mqtt_buf[Mqtt_num] = 0x00;
          command_Service(Mqtt_buf);
          const char* res = "moter command done.";
          mqtt_response(res);
          Mqtt_num = 0;
        }else if(receive[index]==0x02 && (Mqtt_num==0 || Mqtt_num>10)){ //
          Mqtt_num = 0;
          Mqtt_buf[10]=0x00;
        }else{
          Mqtt_buf[ Mqtt_num++ ] = receive[index];
          Mqtt_num %= SERIAL_MAX;
        }
      }
      #ifdef DEBUG
      Serial.print("MQTT: ");
      Serial.println(receive);
      #endif
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
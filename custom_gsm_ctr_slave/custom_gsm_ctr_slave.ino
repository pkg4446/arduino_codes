#include <EEPROM.h>
#include <Wire.h>
#include <WiFi.h>
#include <DS3231.h>
#include <max6675.h>
#include <Adafruit_SHT31.h>
#include "uart_print.h"

#define TOTAL_RELAY 10
#define EEPROM_SIZE_CONFIG  24
#define EEPROM_SIZE_VALUE   3
#define EEPROM_SIZE_CTR     8
#define COMMAND_LENGTH  32
#define UPDATE_INTERVAL 1000L
//항온, 팬, 하우스 윙
const uint8_t thermoDO[2] = {32,34};
const uint8_t thermoCS    = 15;
const uint8_t thermoCLK   = 14;

MAX6675 thermocouple1(thermoCLK, thermoCS, thermoDO[0]);
MAX6675 thermocouple2(thermoCLK, thermoCS, thermoDO[1]);
Adafruit_SHT31 sht31 = Adafruit_SHT31();

DS3231 RTC_DS3231;
HardwareSerial nxSerial(2);
bool    nextion_shift = false;
uint8_t nextion_page  = 0;
enum RelayFunc {
  Cooler = 0,
  Heater,
  Fan_A,
  Fan_B,
  Wing_A,
  Wing_B,
  Spare_A,
  Spare_B,
  Spare_C,
  Spare_D
};
/***************EEPROM*********************/
const uint8_t eep_var[EEPROM_SIZE_VALUE*EEPROM_SIZE_CTR] = {48,49,50, 51,52,53, 54,55,56,
                                                            57,58,59, 60,61,62, 63,64,65,
                                                            66,67,68, 69,70,71};
/***************EEPROM*********************/
/***************PIN_CONFIG*****************/
const int8_t Relay[TOTAL_RELAY] = {2,4,5,12,13,23,27,26,25,33};
/***************PIN_CONFIG*****************/
/***************Interval_timer*************/
unsigned long prevUpdateTime = 0L;
uint8_t       update_order   = 0;
/***************Interval_timer*************/
/***************Variable*******************/
typedef struct ctr_var{
    bool    enable;
    bool    state;
    uint8_t run;
    uint8_t stop;
}ctr_var;
/***************Variable*******************/
ctr_var   iot_ctr[EEPROM_SIZE_CTR];
uint16_t  water_ctr_time[2]   = {0,};
uint16_t  Fan_A_ctr_time = 0;
/*
ctr_var water[2];   // run:동작_초, stop:정지_분
ctr_var lamp[3];    // run:시작시간,stop:정지시간
ctr_var temp_ctr;   // run:목표값,  stop:허용치
ctr_var circulate;  // run:동작_분, stop:정지_분
*/
//이산화탄소 측정 추가
bool    wifi_able;
bool    uart_type = true;
/***************Variable*******************/
char    command_buf[COMMAND_LENGTH];
int8_t  command_num;
/******************************************/
void command_service(){
  String cmd_text     = "";
  String temp_text    = "";
  bool   eep_change   = false;
  uint8_t check_index = 0;
  
  for(uint8_t index_check=0; index_check<COMMAND_LENGTH; index_check++){
    if(command_buf[index_check] == 0x20 || command_buf[index_check] == 0x00){
      check_index = index_check+1;
      break;
    }
    cmd_text += command_buf[index_check];
  }
  for(uint8_t index_check=check_index; index_check<COMMAND_LENGTH; index_check++){
    if(command_buf[index_check] == 0x20 || command_buf[index_check] == 0x00){
      check_index = index_check+1;
      break;
    }
    temp_text += command_buf[index_check];
  }
  /**********/
  Serial.print("cmd: ");
  Serial.print(cmd_text);

  if(cmd_text=="time"){
    time_show();
  }else if(cmd_text=="timeset"){
    if(wifi_able) time_set();
    else Serial.println("wifi not connected");
  }else if(cmd_text=="manual"){
    uint8_t relay_num = temp_text.toInt();
    if(relay_num < TOTAL_RELAY){
      String cmd_select = "";
      for(uint8_t index_check=check_index; index_check<COMMAND_LENGTH; index_check++){
        if(command_buf[index_check] == 0x20 || command_buf[index_check] == 0x00){
          check_index = index_check+1;
          break;
        }
        cmd_select += command_buf[index_check];
      }
      if(cmd_select == "on")  digitalWrite(Relay[relay_num], true);
      else  digitalWrite(Relay[relay_num], false);
    }
  }else if(cmd_text=="reboot"){
    ESP.restart();
  }else if(cmd_text=="send"){
    nextion_print(&nxSerial,temp_text);
  }else if(cmd_text=="set"){
    uint8_t iot_ctr_type = 255;
    if(temp_text=="temp"){iot_ctr_type=Cooler;}
    else if(temp_text=="fan"){iot_ctr_type=Fan_A;}
    if(iot_ctr_type != 255){
      eep_change = true;
      String cmd_select = "";
      for(uint8_t index_check=check_index; index_check<COMMAND_LENGTH; index_check++){
        if(command_buf[index_check] == 0x20 || command_buf[index_check] == 0x00){
          check_index = index_check+1;
          break;
        }
        cmd_select += command_buf[index_check];
      }
      String cmd_value = "";
      for(uint8_t index_check=check_index; index_check<COMMAND_LENGTH; index_check++){
        if(command_buf[index_check] == 0x20 || command_buf[index_check] == 0x00){
          check_index = index_check+1;
          break;
        }
        cmd_value += command_buf[index_check];
      }
      
      if(cmd_select=="ena"){
        iot_ctr[iot_ctr_type].enable = cmd_value.toInt();
        EEPROM.write(eep_var[iot_ctr_type*3], iot_ctr[iot_ctr_type].enable);
      }else if(cmd_select=="run"){
        iot_ctr[iot_ctr_type].run    = cmd_value.toInt();
        EEPROM.write(eep_var[iot_ctr_type*3+1], iot_ctr[iot_ctr_type].run);
      }else if(cmd_select=="stp"){
        iot_ctr[iot_ctr_type].stop   = cmd_value.toInt();
        EEPROM.write(eep_var[iot_ctr_type*3+2], iot_ctr[iot_ctr_type].stop);
      }
      if(uart_type){
        Serial.print("enable: ");Serial.print(iot_ctr[iot_ctr_type].enable);
        Serial.print(", run :");Serial.print(iot_ctr[iot_ctr_type].run);
        Serial.print(", stop :");Serial.println(iot_ctr[iot_ctr_type].stop);
      }
    }
  }else if(cmd_text=="config"){
    uint8_t iot_ctr_type = 255;
    if(temp_text=="water_a"){iot_ctr_type=Water_A;}
    else if(temp_text=="water_b"){iot_ctr_type=Water_B;}
    else if(temp_text=="water_h"){iot_ctr_type=Water_H;}
    else if(temp_text=="lamp_a"){iot_ctr_type=Lamp_A;}
    else if(temp_text=="lamp_b"){iot_ctr_type=Lamp_B;}
    else if(temp_text=="lamp_c"){iot_ctr_type=Lamp_C;}
    else if(temp_text=="circul"){iot_ctr_type=Fan_A;}
    else if(temp_text=="temp"){iot_ctr_type=Cooler;}
    if(iot_ctr_type != 255){
      if(uart_type){
        Serial.print("enable: ");Serial.print(iot_ctr[iot_ctr_type].enable);
        Serial.print(", run :");Serial.print(iot_ctr[iot_ctr_type].run);
        Serial.print(", stop :");Serial.println(iot_ctr[iot_ctr_type].stop);
      }else{

      }
    }else if(uart_type){
      for (int index = 0; index < EEPROM_SIZE_CTR; index++) {
        Serial.print("config no.");Serial.print(index);
        Serial.print(", enable: ");Serial.print(iot_ctr[index].enable);
        Serial.print(", run :");Serial.print(iot_ctr[index].run);
        Serial.print(", stop :");Serial.println(iot_ctr[index].stop);
      }
    }
  }else{ serial_err_msg(&Serial, command_buf); }
  if(eep_change){
    EEPROM.commit();
  }
}
void command_process(char ch) {
  if(ch=='\n'){
    command_buf[command_num] = 0x00;
    command_num = 0;
    command_service();
    memset(command_buf, 0x00, COMMAND_LENGTH);
  }else if(ch!='\r'){
    command_buf[command_num++] = ch;
    command_num %= COMMAND_LENGTH;
  }
}
/******************************************/
void time_show(){
  bool century = false;
  bool h12Flag;
  bool pmFlag;
  uint8_t clock_year  = RTC_DS3231.getYear();
  uint8_t clock_month = RTC_DS3231.getMonth(century);
  uint8_t clock_day   = RTC_DS3231.getDate();
  uint8_t clock_dow   = RTC_DS3231.getDoW();
  uint8_t clock_hour  = RTC_DS3231.getHour(h12Flag, pmFlag);
  uint8_t clock_min   = RTC_DS3231.getMinute();
  uint8_t clock_sec   = RTC_DS3231.getSecond();

  if(uart_type){
    Serial.print('2');
    if(century)Serial.print('1');
    else Serial.print('0');
    if(clock_year<10) Serial.print('0');
    Serial.print(clock_year);Serial.print('/');Serial.print(clock_month);Serial.print('/');Serial.print(clock_day);Serial.print(',');
    Serial.print(clock_hour);Serial.print(':');Serial.print(clock_min);Serial.print(':');Serial.println(clock_sec);
  }
  nextion_print(&nxSerial,"rtc0=20"+String(clock_year));
  nextion_print(&nxSerial,"rtc1="+String(clock_month));
  nextion_print(&nxSerial,"rtc2="+String(clock_day));
  nextion_print(&nxSerial,"rtc3="+String(clock_hour));
  nextion_print(&nxSerial,"rtc4="+String(clock_min));
  nextion_print(&nxSerial,"rtc5="+String(clock_sec));
  nextion_print(&nxSerial,"rtc6="+String(clock_dow));
}

void time_set(){
  /*
  uint8_t time_dow,time_day,time_month,time_year,time_time[3];
  RTC_DS3231.setClockMode(false);
  RTC_DS3231.setYear(time_year);
  RTC_DS3231.setMonth(time_month);
  RTC_DS3231.setDate(time_day);
  RTC_DS3231.setDoW(time_dow);
  RTC_DS3231.setHour(time_time[0]);
  RTC_DS3231.setMinute(time_time[1]);
  RTC_DS3231.setSecond(time_time[2]);
  */
}
/******************************************/
/***************Functions******************/
void setup() {
  Serial.begin(115200);
  nxSerial.begin(115200, SERIAL_8N1, 16, 17);
  Wire.begin();

  for (uint8_t index = 0; index < TOTAL_RELAY; index++)
  {
    pinMode(Relay[index], OUTPUT);
    digitalWrite(Relay[index], false);
  }

  if (!EEPROM.begin((EEPROM_SIZE_CONFIG*2) + (EEPROM_SIZE_VALUE*EEPROM_SIZE_CTR))){
    if(uart_type){
      Serial.println("Failed to initialise eeprom");
      Serial.println("Restarting...");
    }
    delay(1000);
    ESP.restart();
  }
  for (int index = 0; index < EEPROM_SIZE_CONFIG; index++) {
    ssid[index]     = EEPROM.read(eep_ssid[index]);
    password[index] = EEPROM.read(eep_pass[index]);
  }

  for (int index = 0; index < EEPROM_SIZE_CTR; index++) {
    iot_ctr[index].state  = true;
    iot_ctr[index].enable = EEPROM.read(eep_var[3*index]);
    iot_ctr[index].run    = EEPROM.read(eep_var[3*index+1]);
    iot_ctr[index].stop   = EEPROM.read(eep_var[3*index+2]);
  }

  sht31.begin(0x44);
  serial_command_help(&Serial);
  if(uart_type) Serial.println("System online");
  nextion_print(&nxSerial,"page 0");
}

// the loop function runs over and over again forever
void loop() {
  if (Serial.available()) command_process(Serial.read());
  if (nxSerial.available()) command_process(nxSerial.read());
  system_ctr(millis());
  page_change();
}

void page_change(){
  if(nextion_shift){
    nextion_shift = false;
    Serial.print("page "); Serial.println(nextion_page);
    if(nextion_page == 0){
      time_show();
      nextion_display("page_main.wifi",wifi_able,&nxSerial);
      for (uint8_t index = 0; index < 2; index++){
        nextion_display("page_main.sw_f"+String(index+1),iot_ctr[Water_A+index].enable,&nxSerial);
        nextion_display("page_main.flow"+String(index+1)+"o",iot_ctr[Water_A+index].run,&nxSerial);
        nextion_display("page_main.flow"+String(index+1)+"f",iot_ctr[Water_A+index].stop,&nxSerial);

        nextion_display("page_main.sw_l"+String(index+1),iot_ctr[Lamp_A+index].enable,&nxSerial);
        nextion_display("page_main.led"+String(index+1)+"o",iot_ctr[Lamp_A+index].run,&nxSerial);
        nextion_display("page_main.led"+String(index+1)+"f",iot_ctr[Lamp_A+index].stop,&nxSerial);
      }
      nextion_display("page_main.sw_l3",iot_ctr[Lamp_C].enable,&nxSerial);
      nextion_display("page_main.led3o",iot_ctr[Lamp_C].run,&nxSerial);
      nextion_display("page_main.led3f",iot_ctr[Lamp_C].stop,&nxSerial);

      nextion_display("page_main.sw_t",iot_ctr[Cooler].enable,&nxSerial);
      nextion_display("page_main.tempt",iot_ctr[Cooler].run,&nxSerial);
      nextion_display("page_main.tempg",iot_ctr[Cooler].stop,&nxSerial);

      nextion_display("page_main.sw_f",iot_ctr[Fan_A].enable,&nxSerial);
      nextion_display("page_main.fano",iot_ctr[Fan_A].run,&nxSerial);
      nextion_display("page_main.fanf",iot_ctr[Fan_A].stop,&nxSerial);
    }else if(nextion_page == 4){
      for (uint8_t index = 0; index < 2; index++){
        nextion_display("page_liq.sw_liq"+String(index+1),iot_ctr[Water_A+index].enable,&nxSerial);
        nextion_display("page_liq.run"+String(index+1),iot_ctr[Water_A+index].run,&nxSerial);
        nextion_display("page_liq.stp"+String(index+1),iot_ctr[Water_A+index].stop,&nxSerial);
      }
      nextion_display("page_liq.sw_liq_h",iot_ctr[Water_H].enable,&nxSerial);
      nextion_display("page_liq.run_h",iot_ctr[Water_H].run,&nxSerial);
    }else if(nextion_page == 5){
      nextion_display("page_temp.sw_temp",iot_ctr[Cooler].enable,&nxSerial);
      nextion_display("page_temp.run_t",iot_ctr[Cooler].run,&nxSerial);
      nextion_display("page_temp.stp_t",iot_ctr[Cooler].stop,&nxSerial);
    }else if(nextion_page == 6){
      for (uint8_t index = 0; index < 3; index++){
        nextion_display("page_led.sw_led"+String(index+1),iot_ctr[Lamp_A+index].enable,&nxSerial);
        nextion_display("page_led.run"+String(index+1),iot_ctr[Lamp_A+index].run,&nxSerial);
        nextion_display("page_led.stp"+String(index+1),iot_ctr[Lamp_A+index].stop,&nxSerial);
      }
    }else if(nextion_page == 7){
      nextion_display("page_fan.sw_fan",iot_ctr[Fan_A].enable,&nxSerial);
      nextion_display("page_fan.run_f",iot_ctr[Fan_A].run,&nxSerial);
      nextion_display("page_fan.stp_f",iot_ctr[Fan_A].stop,&nxSerial);
    }else if(nextion_page == 2){
      
    }
  }
}

void system_ctr(unsigned long millisec){
  if(millisec > prevUpdateTime + 250){
    prevUpdateTime = millisec;
    update_order += 1;
    if(update_order == 1){
      int16_t humi_now = sht31.readHumidity()*10;
      int16_t temp_air = sht31.readTemperature()*10;
      int16_t temp_liq = thermocouple2.readCelsius()*10;
      int16_t temp_rtc = RTC_DS3231.getTemperature()*10;
      if(temp_liq>999) temp_liq=999;

      if(nextion_page == 0){
        nextion_display("page_main.temp1",temp_air,&nxSerial);
        nextion_display("page_main.humi",humi_now,&nxSerial);
        nextion_display("page_main.temp3",temp_rtc,&nxSerial);
        nextion_display("page_main.temp2",thermocouple1.readCelsius()*10,&nxSerial);
        nextion_display("page_main.temp4",temp_liq,&nxSerial);
      }
      if(iot_ctr[Cooler].enable){
        if(temp_air < temp_rtc-100 || temp_air > temp_rtc+100){
          //온도센서 고장
          digitalWrite(Relay[Cooler], false);
          digitalWrite(Relay[Heater], false);
        }else if(temp_air > iot_ctr[Cooler].run + iot_ctr[Cooler].stop){
          digitalWrite(Relay[Cooler], true);
          digitalWrite(Relay[Heater], false);
          iot_ctr[Cooler].state = true;
        }else if(temp_air < iot_ctr[Cooler].run - iot_ctr[Cooler].stop){
          digitalWrite(Relay[Cooler], false);
          digitalWrite(Relay[Heater], true);
          iot_ctr[Cooler].state = false;
        }else if((iot_ctr[Cooler].state && temp_air > iot_ctr[Cooler].run)||(!iot_ctr[Cooler].state && temp_air < iot_ctr[Cooler].run)){
          digitalWrite(Relay[Cooler], false);
          digitalWrite(Relay[Heater], false);
          iot_ctr[Cooler].state = false;
        }
      }else if(nextion_page!=2){
        digitalWrite(Relay[Cooler], false);
        digitalWrite(Relay[Heater], false);
        iot_ctr[Cooler].state = false;
      }
    }else if(update_order == 2){
      if(iot_ctr[Fan_A].enable){
        if(iot_ctr[Fan_A].state){
          if(nextion_page == 0) nextion_display("page_main.fanom",(Fan_A_ctr_time/60),&nxSerial);
          if(--Fan_A_ctr_time < 1){
            iot_ctr[Fan_A].state = false;
            Fan_A_ctr_time = iot_ctr[Fan_A].stop*60;
          }else if(Fan_A_ctr_time > iot_ctr[Fan_A].run*60) Fan_A_ctr_time = iot_ctr[Fan_A].run*60;
        }else{
          if(nextion_page == 0) nextion_display("page_main.fanfm",(Fan_A_ctr_time/60),&nxSerial);
          if(--Fan_A_ctr_time < 1){
            iot_ctr[Fan_A].state = true;
            Fan_A_ctr_time = iot_ctr[Fan_A].run*60;
          }else if(Fan_A_ctr_time > iot_ctr[Fan_A].stop*60) Fan_A_ctr_time = iot_ctr[Fan_A].stop*60;
        }
        digitalWrite(Relay[Fan_A], iot_ctr[Fan_A].state);
      }else if(nextion_page!=2){
        iot_ctr[Fan_A].state = false;
        digitalWrite(Relay[Fan_A], iot_ctr[Fan_A].state);
      }
    }else {
      update_order = 0;
    }
  }
}
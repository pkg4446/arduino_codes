#include <EEPROM.h>
#include <Wire.h>
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
bool    nextion_shift         = false;
bool    nextion_shift_dubble  = false;
uint8_t nextion_page  = 0;
bool    wing_state    = false;
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
const uint8_t eep_var[EEPROM_SIZE_VALUE*EEPROM_SIZE_CTR] = {1,2,3,    4,5,6,    7,8,9,    10,11,12,
                                                            13,14,15, 16,17,18, 19,20,21, 22,23,24};
/***************EEPROM*********************/
/***************PIN_CONFIG*****************/
const int8_t Relay[TOTAL_RELAY] = {2,4,5,12,13,23,27,26,25,33};
/***************PIN_CONFIG*****************/
/***************Interval_timer*************/
unsigned long prevUpdateTime  = 0L;
unsigned long prevSendTime    = 0L;
uint8_t       update_order    = 0;
/***************Interval_timer*************/
/***************Variable*******************/
typedef struct ctr_var{
    bool    enable;
    bool    state;
    uint8_t run;
    uint8_t stop;
}ctr_var;
/***************Variable*******************/
bool      relay_state[TOTAL_RELAY] = {false,};
ctr_var   iot_ctr[EEPROM_SIZE_CTR];
uint16_t  water_ctr_time[2] = {0,};
uint16_t  fan_ctr_time[2]   = {0,};
/*
ctr_var water[2];   // run:동작_초, stop:정지_분
ctr_var lamp[3];    // run:시작시간,stop:정지시간
ctr_var temp_ctr;   // run:목표값,  stop:허용치
ctr_var circulate;  // run:동작_분, stop:정지_분
*/
//이산화탄소 측정 추가
bool    uart_type = true;
/***************Variable*******************/
char    command_buf[COMMAND_LENGTH];
int8_t  command_num;
/******************************************/
void relay_ctr(uint8_t num_relay, bool status_relay){
  digitalWrite(Relay[num_relay], status_relay);
  relay_state[num_relay] = status_relay;
  if(nextion_page == 2) nextion_display("bt1"+String(num_relay),status_relay,&nxSerial);
}

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
  Serial.println(command_buf);

  if(cmd_text=="manual"){
    uint8_t relay_num = temp_text.toInt();
    relay_num %= TOTAL_RELAY;
    String cmd_select = "";
    for(uint8_t index_check=check_index; index_check<COMMAND_LENGTH; index_check++){
      if(command_buf[index_check] == 0x20 || command_buf[index_check] == 0x00){
        check_index = index_check+1;
        break;
      }
      cmd_select += command_buf[index_check];
    }
    if(cmd_select == "on")  relay_ctr(relay_num, true);
    else  relay_ctr(relay_num, false);
  }else if(cmd_text=="reboot"){
    ESP.restart();
  }else if(cmd_text=="page"){
    prevSendTime  = millis()-200;
    nextion_shift = true;
    nextion_shift_dubble = true;
    nextion_page  = temp_text.toInt();
  }else if(cmd_text=="send"){
    nextion_print(&nxSerial,temp_text);
  }else if(cmd_text=="wing"){
    if(temp_text == "on") wing_state = true;
    else                  wing_state = false;
    relay_ctr(Wing_A, wing_state);
    relay_ctr(Wing_B, wing_state);
  }else if(cmd_text=="set"){
    uint8_t iot_ctr_type = 255;
    if(temp_text=="temp"){iot_ctr_type=Cooler;}
    else if(temp_text=="circul_i"){iot_ctr_type=Fan_A;}
    else if(temp_text=="circul_o"){iot_ctr_type=Fan_B;}
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
      if(iot_ctr_type<=Heater)      nextion_print(&nxSerial,"page 5"); //LED 페이지
      else if(iot_ctr_type<=Fan_B)  nextion_print(&nxSerial,"page 7"); //양액 페이지
    }
  }else if(cmd_text=="config"){
    uint8_t iot_ctr_type = 255;
    if(temp_text=="temp"){iot_ctr_type=Cooler;}
    else if(temp_text=="circul"){iot_ctr_type=Fan_A;}
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
  }else if(cmd_text=="temp"){
      Serial.print("CPU: ");Serial.print(RTC_DS3231.getTemperature(), 2);Serial.println("°C");
      Serial.print("PT100 1: ");Serial.print(thermocouple1.readCelsius());Serial.println("°C");
      Serial.print("PT100 2: ");Serial.print(thermocouple2.readCelsius());Serial.println("°C");
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
    relay_ctr(index, false);
  }

  if (!EEPROM.begin((EEPROM_SIZE_CONFIG*2) + (EEPROM_SIZE_VALUE*EEPROM_SIZE_CTR))){
    if(uart_type){
      Serial.println("Failed to initialise eeprom");
      Serial.println("Restarting...");
    }
    delay(1000);
    ESP.restart();
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
  if (nxSerial.available()){
    char nx_read = nxSerial.read();
    Serial.print(nx_read);
    command_process(nx_read);
  }
  unsigned long millisec = millis();
  system_ctr(millisec);
  page_change(millisec);
}

void page_change(unsigned long millisec){
  if(millisec > prevSendTime + 200){
    prevSendTime = millisec;
    if(nextion_shift){
      if(nextion_shift_dubble) nextion_shift_dubble = false;
      else nextion_shift = false;
      Serial.print("page "); Serial.println(nextion_page);
      if(nextion_page == 0){
        nextion_display("sw_t",iot_ctr[Cooler].enable,&nxSerial);
        nextion_display("tempt",iot_ctr[Cooler].run,&nxSerial);
        nextion_display("tempg",iot_ctr[Cooler].stop,&nxSerial);
        /*
        for (uint8_t index = 0; index < 2; index++){
          nextion_display("sw_f"+String(index+1),iot_ctr[Fan_A+index].enable,&nxSerial);
          nextion_display("fano"+String(index+1),iot_ctr[Fan_A+index].run,&nxSerial);
          nextion_display("fanf"+String(index+1),iot_ctr[Fan_A+index].stop,&nxSerial);
        }
        */
        nextion_display("sw_f1",iot_ctr[Fan_A].enable,&nxSerial);
        nextion_display("sw_f2",iot_ctr[Fan_B].enable,&nxSerial);
        delay(50);
        nextion_display("fano1",iot_ctr[Fan_A].run,&nxSerial);
        nextion_display("fanf1",iot_ctr[Fan_A].stop,&nxSerial);
        nextion_display("fano2",iot_ctr[Fan_B].run,&nxSerial);
        nextion_display("fanf2",iot_ctr[Fan_B].stop,&nxSerial);
      }else if(nextion_page == 5){
        nextion_display("sw_temp",iot_ctr[Cooler].enable,&nxSerial);
        nextion_display("run_t",iot_ctr[Cooler].run,&nxSerial);
        nextion_display("stp_t",iot_ctr[Cooler].stop,&nxSerial);
      }else if(nextion_page == 7){
        /*
        for (uint8_t index = 0; index < 2; index++){
          nextion_display("sw_fan"+String(index+1),iot_ctr[Fan_A+index].enable,&nxSerial);
          nextion_display("run_f"+String(index+1),iot_ctr[Fan_A+index].run,&nxSerial);
          nextion_display("stp_f"+String(index+1),iot_ctr[Fan_A+index].stop,&nxSerial);
        }
        */
        nextion_display("sw_fan1",iot_ctr[Fan_A].enable,&nxSerial);
        nextion_display("run_f1",iot_ctr[Fan_A].run,&nxSerial);
        nextion_display("stp_f1",iot_ctr[Fan_A].stop,&nxSerial);
        nextion_display("sw_fan2",iot_ctr[Fan_B].enable,&nxSerial);
        nextion_display("run_f2",iot_ctr[Fan_B].run,&nxSerial);
        nextion_display("stp_f2",iot_ctr[Fan_B].stop,&nxSerial);
      }else if(nextion_page == 2){
        for (uint8_t index = 0; index < TOTAL_RELAY/2; index++){
          nextion_display("bt1"+String(index),relay_state[index],&nxSerial);
        }
        delay(50);
        for (uint8_t index = TOTAL_RELAY/2; index < TOTAL_RELAY; index++){
          nextion_display("bt1"+String(index),relay_state[index],&nxSerial);
        }
      }
    }
  }
}

void system_ctr(unsigned long millisec){
  if(millisec > prevUpdateTime + 250){
    if(!nextion_shift){
      prevUpdateTime = millisec;
      update_order += 1;
      if(update_order == 1){
        int16_t temp_air = thermocouple1.readCelsius()*10;
        int16_t temp_out = thermocouple2.readCelsius()*10;
        if(temp_air == -1) temp_air=999;
        if(temp_out == -1) temp_out=999;
        /*
        int16_t sht31_humi = sht31.readHumidity()*10;
        int16_t sht31_temp = sht31.readTemperature()*10;
        */
        int16_t temp_rtc = RTC_DS3231.getTemperature()*10;
        if(nextion_page == 0){
          nextion_display("temp_air",temp_air,&nxSerial);
          nextion_display("temp_out",temp_out,&nxSerial);
          nextion_display("bt_wing",wing_state,&nxSerial);
        }
        if(iot_ctr[Cooler].enable){
          if(temp_air < temp_rtc-100 || temp_air > temp_rtc+100){
            //온도센서 고장
            relay_ctr(Cooler, false);
            relay_ctr(Heater, false);
          }else if(temp_air > iot_ctr[Cooler].run + iot_ctr[Cooler].stop){
            relay_ctr(Cooler, true);
            relay_ctr(Heater, false);
            iot_ctr[Cooler].state = true;
          }else if(temp_air < iot_ctr[Cooler].run - iot_ctr[Cooler].stop){
            relay_ctr(Cooler, false);
            relay_ctr(Heater, true);
            iot_ctr[Cooler].state = false;
          }else if((iot_ctr[Cooler].state && temp_air > iot_ctr[Cooler].run)||(!iot_ctr[Cooler].state && temp_air < iot_ctr[Cooler].run)){
            relay_ctr(Cooler, false);
            relay_ctr(Heater, false);
            iot_ctr[Cooler].state = false;
          }
        }else if(nextion_page!=2){
          relay_ctr(Cooler, false);
          relay_ctr(Heater, false);
          iot_ctr[Cooler].state = false;
        }
      }else if(update_order == 2){
        for(uint8_t index=0; index<2; index++){
          if(iot_ctr[Fan_A+index].enable){
            if(iot_ctr[Fan_A+index].state){
              //if(nextion_page == 0) nextion_display("fanom",(fan_ctr_time[index]/60),&nxSerial);
              if(--fan_ctr_time[index] < 1){
                iot_ctr[Fan_A+index].state = false;
                fan_ctr_time[index] = iot_ctr[Fan_A+index].stop*60;
              }else if(fan_ctr_time[index] > iot_ctr[Fan_A+index].run) fan_ctr_time[index] = iot_ctr[Fan_A+index].run;
            }else{
              //if(nextion_page == 0) nextion_display("fanfm",(fan_ctr_time[index]/60),&nxSerial);
              if(--fan_ctr_time[index] < 1){
                iot_ctr[Fan_A+index].state = true;
                fan_ctr_time[index] = iot_ctr[Fan_A+index].run;
              }else if(fan_ctr_time[index] > iot_ctr[Fan_A+index].stop*60) fan_ctr_time[index] = iot_ctr[Fan_A+index].stop*60;
            }
            relay_ctr(Fan_A+index, iot_ctr[Fan_A+index].state);
          }else if(nextion_page!=2){
            iot_ctr[Fan_A+index].state = false;
            relay_ctr(Fan_A+index, iot_ctr[Fan_A+index].state);
          }
        }
      }else {
        update_order = 0;
      }
    }
  }
}
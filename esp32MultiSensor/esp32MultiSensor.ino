/******************************** 2024-01-11 ********************************/
#include  <WiFi.h>
#include  <HTTPClient.h>
#include  <Wire.h>
#define   TCAADDR     0x70
#define   SERIAL_MAX  128
#define   JSON_KEY    4

#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include <Adafruit_SHT31.h>
Adafruit_SHT31 sht31  = Adafruit_SHT31();

uint8_t   LED = 17;

char      deviceID[18];
int16_t   Temperature[8]  = {14040,};
int16_t   Humidity[8]     = {14040,};

uint8_t       control_index = 1;
int16_t       control_temp  = 0;
uint32_t      heater_run    = 0;
unsigned long heater_on     = 0UL;

unsigned long timer_SHT31 = 0;
unsigned long timer_SEND  = 0;
unsigned long timer_WIFI  = 0;

uint16_t log_index   = 0; 
String time_stmp  = "Didn't get time from server.";

void tca_select(uint8_t index) {
  if (index > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << index);
  Wire.endTransmission();
}

/******************EEPROM******************/
#include "EEPROM.h"
#define   EEPROM_SIZE 16

const uint8_t eep_ssid[EEPROM_SIZE] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
const uint8_t eep_pass[EEPROM_SIZE] = {16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

char ssid[EEPROM_SIZE];
char password[EEPROM_SIZE]; //#234567!
/******************EEPROM******************/

char    Serial_buf[SERIAL_MAX];
int8_t  Serial_num;
void wifi_config_change() {
  String at_cmd     = strtok(Serial_buf, "=");
  String ssid_value = strtok(NULL, "=");
  String pass_value = strtok(NULL, ";");
  if(at_cmd=="AT+WIFI"){
    Serial.print("ssid_value=");
    for (int index = 0; index < EEPROM_SIZE; index++) {
      if(index < ssid_value.length()){
        Serial.print(ssid_value[index]);
        EEPROM.write(eep_ssid[index], byte(ssid_value[index]));
      }else{
        EEPROM.write(eep_ssid[index], byte(0x00));
      }      
    }
    Serial.println("");
    Serial.print("pass_value=");
    for (int index = 0; index < EEPROM_SIZE; index++) {
      if(index < pass_value.length()){
        Serial.print(pass_value[index]);
        EEPROM.write(eep_pass[index], byte(pass_value[index]));
      }else{
        EEPROM.write(eep_pass[index], byte(0x00));
      }    
    }
    Serial.println("");
    EEPROM.commit();
    ESP.restart();
  }else{
    Serial.println(Serial_buf);
  }
}//Command_service() END

void Serial_process() {
  char ch;
  ch = Serial.read();
  switch ( ch ) {
    case ';':
      Serial_buf[Serial_num] = 0x00;
      wifi_config_change();
      Serial_num = 0;
      break;
    default :
      Serial_buf[ Serial_num ++ ] = ch;
      Serial_num %= SERIAL_MAX;
      break;
  }
}

// standard Arduino setup()
void setup()
{
  Serial.begin(115200);

  if (!EEPROM.begin(EEPROM_SIZE*2)){
    Serial.println("Failed to initialise eeprom");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }

  for (int index = 0; index < EEPROM_SIZE; index++) {
    ssid[index]     = EEPROM.read(eep_ssid[index]);
    password[index] = EEPROM.read(eep_pass[index]);
  }

  Serial.println("------- wifi config -------");
  Serial.println("AT+WIFI=SSID=PASS;");
  Serial.print("ssid: "); Serial.println(ssid);
  Serial.print("pass: "); Serial.println(password);
  Serial.println("---------------------------");

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Wire.begin();
  pinMode(LED, OUTPUT);
  
  unsigned long wifi_config_update = 0UL;
  while (WiFi.status() != WL_CONNECTED) {
    if (Serial.available()) Serial_process();
    unsigned long update_time = millis();
    if(update_time - wifi_config_update > 3000){
      digitalWrite(LED, false);
      wifi_config_update = update_time;
      Serial.println("Connecting to WiFi..");
    }
  }
  Serial.println("Connected to the WiFi network");
  Serial.println("ver 1.0.2");
}

void loop()
{
  unsigned long millisec = millis();
  get_sensor(millisec);
  send_sensor(millisec);
  if (Serial.available()) Serial_process();
  mesh_restart(millisec);
}

void wifiCheck(unsigned long millisec) {
  if ((millisec - timer_WIFI) > 1000) {
    timer_WIFI = millisec;
        
    if (WiFi.status() != WL_CONNECTED) {
      digitalWrite(LED, true);
    } else {
      WiFi.begin(ssid, password);
      digitalWrite(LED, false);
    }

  }
}

void send_sensor(unsigned long millisec) {
  if ((millisec - timer_SEND) > 60 * 1000) {
    timer_SEND = millisec;

    for (uint8_t channel = 0; channel < 8; channel++) {
      Serial.print("TCA Port #"); Serial.print(channel);
      Serial.print(", T: ");
      Serial.print(Temperature[channel]);
      Serial.print("°C ,H: ");
      Serial.print(Humidity[channel]);
      Serial.println("%");
    }
    if (WiFi.status() == WL_CONNECTED) {
      for (int i = 0; i < 17; i++) {
        deviceID[i]   = WiFi.macAddress()[i];
      }
      
      httpPOSTRequest("http://smarthive.kro.kr/api/costom/log");
      //httpPOSTRequest("http://192.168.1.15:3004/api/costom/log");
    } else {
      Serial.println("WiFi not connected");
    }
    Serial.println("");
    //여기 sd카드 저장
    log2SD();
  }
}

void get_sensor(unsigned long millisec) {
  if ((millisec - timer_SHT31) > 300) {
    timer_SHT31 = millisec;

    for (uint8_t channel = 0; channel < 8; channel++) {
      tca_select(channel);
      Wire.beginTransmission(68); //0x44
      if (!Wire.endTransmission() && sht31.begin(0x44)) {
        Temperature[channel]  = sht31.readTemperature() * 100;
        Humidity[channel]     = sht31.readHumidity() * 100;
      } else {
        Temperature[channel]  = 14040;
        Humidity[channel]     = 14040;
      }
    }//for

    if(Temperature[control_index] < control_temp-1){
      heater_on   = millis();
      heater_run  = 0;
      digitalWrite(LED, true); //heater on
    }else if(Temperature[control_index] > control_temp){
      heater_run = (millis() - heater_on)/1000;
      digitalWrite(LED, false); //heater off
      httpPOST_Heater("http://smarthive.kro.kr/api/costom/log_h");
    }
  }//if
}


////Send Data//////////////////////////////////////
void httpPOSTRequest(String serverUrl) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverUrl);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String httpRequestData =  (String)"MODULE=" + deviceID +
                            "&TEMP1=" + Temperature[0] +
                            "&TEMP2=" + Temperature[1] +
                            "&TEMP3=" + Temperature[2] +
                            "&TEMP4=" + Temperature[3] +
                            "&TEMP5=" + Temperature[4] +
                            "&TEMP6=" + Temperature[5] +
                            "&TEMP7=" + Temperature[6] +
                            "&TEMP8=" + Temperature[7] +
                            "&HUMI1=" + Humidity[0] +
                            "&HUMI2=" + Humidity[1] +
                            "&HUMI3=" + Humidity[2] +
                            "&HUMI4=" + Humidity[3] +
                            "&HUMI5=" + Humidity[4] +
                            "&HUMI6=" + Humidity[5] +
                            "&HUMI7=" + Humidity[6] +
                            "&HUMI8=" + Humidity[7];

  int httpResponseCode = http.POST(httpRequestData);
  String res = http.getString().c_str();
  Serial.print("respose : ");Serial.println(res);
  res.replace("\"","");
  res.replace("{","");
  res.replace("}","");

  String json[JSON_KEY];
  String json_key[JSON_KEY];
  String json_value[JSON_KEY];

  json[0] = strtok(&res[0], ",");
  for (int index = 1; index < JSON_KEY; index++) {
    json[index] = strtok(0x00, ",");
  }
  for (int index = 0; index < JSON_KEY; index++) {
    json_key[index]   = strtok(&json[index][0], ":");
    json_value[index] = strtok(0x00, "");
  }
  bool result = false;
  String time_online = "";
  uint16_t temp   = 1;
  uint16_t sensor = 1;
  for (int index = 0; index < JSON_KEY; index++) {
    if(json_key[index] == "result" && json_value[index] == "true"){result = true;}
    else if(json_key[index] == "data"){time_online  = json_value[index];}
    else if(json_key[index] == "temp"){temp         = json_value[index].toInt();}
    else if(json_key[index] == "sen"){ sensor       = json_value[index].toInt();}
  }
  if(result){
    time_stmp     = time_online;
    control_temp  = temp*100;
    control_index = sensor;
  }
  /*
  char response[24];
  for(int index = 0; index <24; index++){
    response[index] = res[index+23];
  }  
  time_stmp = String(response);
  Serial.print("respose pharse: ");  
  Serial.println(time_stmp);
  */
  Serial.print("HTTP Response code: ");  
  Serial.println(httpResponseCode);
  http.end();           // Free resources
}////httpPOSTRequest_End


////Send Data//////////////////////////////////////
void httpPOST_Heater(String serverUrl) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverUrl);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String httpRequestData =  (String)"MODULE=" + deviceID +
                            "&RUNTIME=" + heater_run;

  int httpResponseCode = http.POST(httpRequestData);
  String res = http.getString().c_str();
  Serial.print("respose : ");Serial.println(res);
  
  Serial.print("HTTP Response code: ");  
  Serial.println(httpResponseCode);
  http.end();           // Free resources
}////httpPOSTRequest_End
//fileSystem

uint32_t listDir(fs::FS &fs, const char * dirname){  
  uint32_t file_index = 0;
  File root = fs.open(dirname);
  if(!root){return 0;}
  if(!root.isDirectory()){return 0;}
  File file = root.openNextFile();
  Serial.println("* Log file list *");
  while(file){
      if(!file.isDirectory()){
        file_index++;
        Serial.print("  FILE: ");
        Serial.print(file.name());
        Serial.print("  SIZE: ");
        Serial.println(file.size());
        } 
      file = root.openNextFile();
  }
  return file_index;
}

boolean readFile(fs::FS &fs, String path){
    boolean response = true;    
    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        response = false;
    }
    if(file.size()>1048576){response = false;}    
    file.close();    
    return response;
}

void writeFile(fs::FS &fs, String path, const char * message){
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(!file.print(message)){Serial.println("Write failed");} 
    file.close();
}

void appendFile(fs::FS &fs, String path, String message){
    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(!file.print(message)){Serial.println("Append failed");}
    file.close();
}

void log2SD(){
  if(!SD.begin()){
    Serial.println("Card Mount Failed");
    // SD카드 에러 표시
    return;
  }else{
    // SD카드 정상 표시
  }

  uint32_t file_index  = listDir(SD, "/");  

  String filename  = "/log_" + String(file_index) + ".csv"; 
  boolean file_max = readFile(SD, filename);
  if(!file_max){
    file_index++;
    filename  = "/log_" + String(file_index) + ".csv";
    String sd_init = (String)deviceID + "TEMP1,TEMP2,TEMP3,TEMP4,TEMP5,TEMP6,TEMP7,TEMP8,HUMI1,HUMI2,HUMI3,HUMI4,HUMI5,HUMI6,HUMI7,HUMI8,INTERVAL,TIMESTAMP\n";
    writeFile(SD, filename, sd_init.c_str());
    log_index = 0;
  }
  log_index++;
  String sd_log = (String)log_index + Temperature[0] + "," + Temperature[1] + "," + Temperature[2] + "," + Temperature[3] + "," + 
                                      Temperature[4] + "," + Temperature[5] + "," + Temperature[6] + "," + Temperature[7] + "," +
                                      Humidity[0] + "," + Humidity[1] + "," + Humidity[2] + "," + Humidity[3] + "," + 
                                      Humidity[4] + "," + Humidity[5] + "," + Humidity[6] + "," + Humidity[7] + "," +
                                      "600" + "," + time_stmp  + "\n";
  appendFile(SD, filename, sd_log.c_str());
  SD.end();
}

unsigned long timer_restart = 0;
uint8_t restart_count       = 0;
void mesh_restart(unsigned long millisec){
  if(millisec - timer_restart > 1000*60){
    timer_restart = millisec;
    if(restart_count++ > 240) ESP.restart();
  }
}
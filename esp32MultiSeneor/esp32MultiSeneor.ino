#include  <WiFi.h>
#include  <HTTPClient.h>
#include  "Wire.h"
#define   TCAADDR 0x70

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include  "Adafruit_SHT31.h"
Adafruit_SHT31 sht31  = Adafruit_SHT31();

const char* ssid      = "Daesung2G";
const char* password  = "smarthive123";

uint8_t   LED = 17;

char      deviceID[18];
int16_t   Temperature[8]  = {14040,};
int16_t   Humidity[8]     = {14040,};

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

// standard Arduino setup()
void setup()
{
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.begin(115200);
  Wire.begin();
  pinMode(LED, OUTPUT);

  Serial.begin(115200);
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
  } else {
    Serial.print("WiFi connected ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void loop()
{
  get_sensor(millis());
  send_sensor(millis());
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
  if ((millisec - timer_SEND) > 10 * 1000) {
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
  char response[24];
  for(int index = 0; index <24; index++){
    response[index] = res[index+23];
  }  
  time_stmp = String(response);
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

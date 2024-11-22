#include <Update.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>

#define EEPROM_SIZE     24
#define COMMAND_LENGTH  128
#define SERIAL_LENGTH   32

String currentVersion = "0.0.0";  // 현재 버전
/******************EEPROM******************/
const uint8_t eep_ssid = 0;
const uint8_t eep_pass = 24;
/******************EEPROM******************/
char ssid[EEPROM_SIZE];
char password[EEPROM_SIZE]; //#234567!

//// ----------- Command  -----------
void command_help_wifi() {
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
void command_help() {
  Serial.println("********** help **********");
  Serial.println("help   this text");
  Serial.println("show   wifi scan");
  Serial.println("check  firmware update check");
  Serial.println("rest   reboot");
  Serial.println("********** help **********");
}
/**********************************************/
void WIFI_scan(){
  WiFi.disconnect(true);
    Serial.println("WIFI Scanning…");
    uint8_t networks = WiFi.scanNetworks();
    if (networks == 0) {
        Serial.println("no networks found");
    } else {
      Serial.print(networks);
      Serial.println(" networks found");
      Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
      for (int index = 0; index < networks; ++index) {
        // Print SSID and RSSI for each network found
        Serial.printf("%2d",index + 1);
        Serial.print(" | ");
        Serial.printf("%-32.32s", WiFi.SSID(index).c_str());
        Serial.print(" | ");
        Serial.printf("%4d", WiFi.RSSI(index));
        Serial.print(" | ");
        Serial.printf("%2d", WiFi.channel(index));
        Serial.print(" | ");
        switch (WiFi.encryptionType(index))
        {
        case WIFI_AUTH_OPEN:
            Serial.print("open");
            break;
        case WIFI_AUTH_WEP:
            Serial.print("WEP");
            break;
        case WIFI_AUTH_WPA_PSK:
            Serial.print("WPA");
            break;
        case WIFI_AUTH_WPA2_PSK:
            Serial.print("WPA2");
            break;
        case WIFI_AUTH_WPA_WPA2_PSK:
            Serial.print("WPA+WPA2");
            break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
            Serial.print("WPA2-EAP");
            break;
        case WIFI_AUTH_WPA3_PSK:
            Serial.print("WPA3");
            break;
        case WIFI_AUTH_WPA2_WPA3_PSK:
            Serial.print("WPA2+WPA3");
            break;
        case WIFI_AUTH_WAPI_PSK:
            Serial.print("WAPI");
            break;
        default:
            Serial.print("unknown");
        }
        Serial.println();
        delay(10);
      }
    }
    Serial.println("");
    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();
    WiFi.begin(ssid, password);
}
/**********************************************/
String String_slice(uint8_t *check_index, String text, char check_char){
  String response = "";
  for(uint8_t index_check=*check_index; index_check<text.length(); index_check++){
    if(text[index_check] == check_char || text[index_check] == 0x00){
      *check_index = index_check+1;
      break;
    }
    response += text[index_check];
  }
  return response;
}
/**********************************************/
char    Serial_buf[SERIAL_LENGTH];
int8_t  Serial_num;

void wifi_config_change() {

  bool   eep_change = false;
  uint8_t check_index = 0;

  String cmd_text  = String_slice(&check_index, Serial_buf, 0x20);
  String temp_text = String_slice(&check_index, Serial_buf, 0x20);
  
  if(cmd_text=="help"){
    command_help_wifi();
  }else if(cmd_text=="show"){
    WIFI_scan();
  }else if(cmd_text=="ssid"){
    WiFi.disconnect(true);
    Serial.print("ssid=");
    if(temp_text.length() > 0){
      for (int index = 0; index < EEPROM_SIZE; index++) {
        if(index < temp_text.length()){
          Serial.print(temp_text[index]);
          ssid[index] = temp_text[index];
          EEPROM.write(eep_ssid+index, byte(temp_text[index]));
        }else{
          EEPROM.write(eep_ssid+index, byte(0x00));
        }
      }
      eep_change = true;
    }
    Serial.println("");

  }else if(cmd_text=="pass"){
    WiFi.disconnect(true);
    Serial.print("pass=");
    if(temp_text.length() > 0){
      for (int index = 0; index < EEPROM_SIZE; index++) {
        if(index < temp_text.length()){
          Serial.print(temp_text[index]);
          password[index] = temp_text[index];
          EEPROM.write(eep_pass+index, byte(temp_text[index]));
        }else{
          EEPROM.write(eep_pass+index, byte(0x00));
        }
      }
      eep_change = true;
    }
    Serial.println("");

  }else{
    Serial.println(Serial_buf);
  }
  if(eep_change){
    EEPROM.commit();
    WiFi.begin(ssid, password);
    if(WiFi.status() == WL_CONNECTED) ESP.restart();
  }
}//wifi_config_change() END
void serail_Process_wifi(char ch) {
  if(ch=='\n'){
    Serial_buf[Serial_num] = 0x00;
    Serial_num = 0;
    wifi_config_change();
    //memset(Serial_buf, 0x00, SERIAL_LENGTH);
  }else if(ch!='\r'){
    Serial_buf[Serial_num++] = ch;
    Serial_num %= SERIAL_LENGTH;
  }
}
/**********************************************/
/**********************************************/
void serial_service() {
  bool   eep_change = false;
  uint8_t check_index = 0;

  String cmd_text  = String_slice(&check_index, Serial_buf, 0x20);
  String temp_text = String_slice(&check_index, Serial_buf, 0x20);

  Serial.print("CMD:");
  Serial.println(cmd_text);
  Serial.print("DTL:");
  Serial.println(temp_text);
  
  if(cmd_text=="help"){
    command_help();
  }else if(cmd_text=="show"){
    WIFI_scan();
  }else if(cmd_text=="check"){
    httpPOSTRequest();
  }else if(cmd_text=="rest"){
    ESP.restart();
  }else{
    Serial.println(Serial_buf);
  }
}//serial_service() END

void serail_Process(char ch) {
  if(ch=='\n'){
    Serial_buf[Serial_num] = 0x00;
    Serial_num = 0;
    serial_service();
    //memset(Serial_buf, 0x00, SERIAL_LENGTH);
  }else if(ch!='\r'){
    Serial_buf[Serial_num++] = ch;
    Serial_num %= SERIAL_LENGTH;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.print("ver:");
  Serial.println(currentVersion);
  
  if (!EEPROM.begin(EEPROM_SIZE*2)){
    Serial.println("Failed to initialise eeprom");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }

  for (int index = 0; index < EEPROM_SIZE; index++) {
    ssid[index]     = EEPROM.read(eep_ssid+index);
    password[index] = EEPROM.read(eep_pass+index);
  }
  
  command_help_wifi();

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    if (Serial.available()) serail_Process_wifi(Serial.read());
  }
  Serial.println("Connected to the WiFi network");
  command_help();
}//End Of Setup()

void loop() {
  if (Serial.available()) serail_Process(Serial.read());
}

void httpPOSTRequest() {
  String serverUrl = "http://192.168.1.2:3002/firmware/device";   //API adress
  HTTPClient http;
  WiFiClient http_client;
  http.begin(http_client, serverUrl);

  // 타임아웃 설정 증가
  http.setTimeout(30000);  // 30초

  http.addHeader("Content-Type", "application/json");
  String httpRequestData = (String)"{\"ver\":\"" + String(currentVersion) + "\"}";

  int httpResponseCode = http.POST(httpRequestData);

  if (httpResponseCode == 200) {
    int contentLength = http.getSize();
    Serial.printf("Update size: %d\n", contentLength);
    
    if (contentLength <= 0) {
        Serial.println("Invalid content length");
        http.end();
        return;
    }
    
    if (!Update.begin(contentLength)) {
        Serial.printf("Not enough space for update. Required: %d\n", contentLength);
        http.end();
        return;
    }
    
    WiFiClient * stream = http.getStreamPtr();
    
    // 버퍼 크기 증가 및 타임아웃 처리 추가
    size_t written = 0;
    uint8_t buff[2048] = { 0 };
    int timeout = 0;
    
    while (written < contentLength) {
        delay(1);  // WiFi 스택에 시간 양보
        
        size_t available = stream->available();
        
        if (available) {
            size_t toRead = min(available, sizeof(buff));
            size_t bytesRead = stream->readBytes(buff, toRead);
            
            if (bytesRead > 0) {
                size_t bytesWritten = Update.write(buff, bytesRead);
                if (bytesWritten > 0) {
                    written += bytesWritten;
                    timeout = 0;  // 타임아웃 리셋
                    
                    // 진행률 표시
                    float progress = (float)written / contentLength * 100;
                    Serial.printf("Progress: %.1f%%\n", progress);
                }
            }
        } else {
            timeout++;
            if (timeout > 100) {  // 약 10초 타임아웃
                Serial.println("Download timeout");
                Update.abort();
                break;
            }
            delay(100);
        }
    }
    
    if (written == contentLength) {
        if (Update.end(true)) {
            Serial.println("Update Success!");
            ESP.restart();
        } else {
            Serial.printf("Update failed with error: %d\n", Update.getError());
        }
    } else {
        Update.abort();
        Serial.println("Update failed: incomplete download");
    }
} 
else if (httpResponseCode == 204) {
    Serial.println("No update available");
}
else {
    Serial.printf("HTTP error: %d\n", httpResponseCode);
}

  http.end();           // Free resources
}////httpPOSTRequest_End
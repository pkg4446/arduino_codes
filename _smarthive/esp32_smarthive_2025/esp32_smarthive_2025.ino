#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define EEPROM_SIZE 24
#define EEPROM_SET 2
#define SERIAL_MAX  128

#define SDA2 33
#define SCL2 32
TwoWire Wire2 = TwoWire(1);

//#define SENSOR_SHT40
#ifdef SENSOR_SHT40
  #include "Adafruit_SHT4x.h"
  Adafruit_SHT4x sht40 = Adafruit_SHT4x();
#else
  #include <Adafruit_SHT31.h>
  Adafruit_SHT31 sht31_1 = Adafruit_SHT31();
  Adafruit_SHT31 sht31_2 = Adafruit_SHT31(&Wire2);
#endif

const boolean pin_on  = true;
const boolean pin_off = false;

uint8_t sht_port = 0;
//// ------------ EEPROM ------------
const uint8_t eep_ssid    = 0;
const uint8_t eep_pass    = 24;
const uint8_t EEP_temp    = 48;
const uint8_t EEP_stable  = 49;
//// ------------ EEPROM Variable ---
char ssid[EEPROM_SIZE];
char password[EEPROM_SIZE]; //#234567!
uint8_t control_temperature = 3;
//// ----------- Flage --------------
bool use_stable  = false;
bool run_heater  = false;
//// ----------- Variable -----------
char deviceID[18];
//// ----------- Sensor -------------
float temperature   = 0.00f;
float humidity      = 0.00f;
//// ----------- heater work --------
uint16_t work_total = 0;
uint16_t work_heat  = 0;
////for millis() func//
unsigned long timer_SHT     = 0UL;
unsigned long time_stayble  = 0UL;
unsigned long time_send     = 0UL;
//// ------------- PIN --------------
const uint8_t RELAY_HEATER  = 12;
//// ----------- Command  -----------
char    Serial_buf[SERIAL_MAX];
int8_t  Serial_num = 0;
//// ----------- WiFi 연결 상태 ------
bool wifi_connected = false;

// HTTP 클라이언트 객체를 전역으로 선언하여 반복 생성 방지
HTTPClient http;
WiFiClient http_client;

////--------------------- String_slice ----------------////
String String_slice(uint8_t *check_index, String text, char check_char) {
  String response = "";
  for(uint8_t index_check = *check_index; index_check < text.length(); index_check++) {
    if(text[index_check] == check_char || text[index_check] == 0x00) {
      *check_index = index_check + 1;
      break;
    }
    response += text[index_check];
  }
  return response;
}

////--------------------- device restart --------------////
unsigned long timer_restart = 0;
uint8_t       restart_count = 0;
void restart(unsigned long millisec) {
  if(millisec - timer_restart > 1000 * 60) {
    timer_restart = millisec;
    if(restart_count++ > 240) ESP.restart();
  }
}

////--------------------- HTTP POST 요청 관리 함수 -----------////
bool httpPOSTRequest(const String& httpRequestData) {
  if (!wifi_connected) return false;
  
  // String serverUrl = "http://192.168.1.29:3010/log/hive";   // API 주소
  String serverUrl = "http://bee.smarthive.kr/log/hive";  // 대체 API 주소
  
  http.begin(http_client, serverUrl);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);
  
  int httpResponseCode = http.POST(httpRequestData);
  bool success = (httpResponseCode == 200);
  
  if (success) {
    String response = http.getString();
    if (response.length() > 0) {
      Serial.println(response);
      uint8_t cmd_index = 0;
      String_slice(&cmd_index, response, 0x20);
      if(String_slice(&cmd_index, response, 0x20) == "WEB"){
        uint8_t goal = String_slice(&cmd_index, response, 0x20).toInt();
        bool stable = String_slice(&cmd_index, response, 0x20).toInt();
        bool change = false;
        
        if (goal != control_temperature) change = true;
        if (stable != use_stable) change = true;
        
        if (change) {
          control_temperature = goal;
          use_stable = stable;
          EEPROM.write(EEP_temp, control_temperature);
          EEPROM.write(EEP_stable, use_stable);
          EEPROM.commit();
          if (wifi_connected) {
            post_config("mod", control_temperature, use_stable);
          }
        }
      }

    } else {
      Serial.println("Data uploaded");      
    }
  } else {
    Serial.print("HTTP code: ");    
    Serial.println(httpResponseCode);
  }
  
  http.end(); // 자원 해제
  return success;
}

////--------------------- 데이터 POST 함수 --------------////
void post_config(const String& api, uint8_t post_goal, uint8_t post_run) {
  if (!wifi_connected) return;
  
  String json = "{\"DVC\":\"" + String(deviceID) + "\",\"API\":\"" + api + 
                "\",\"DATA\":{\"goal\":" + String(post_goal) + 
                ",\"run\":" + String(post_run) + "}}";
  Serial.println(json);
  httpPOSTRequest(json);
}

void post_callback(unsigned long millisec, bool manual = false) {
  if (!wifi_connected) return;
  if (manual){
    post_data();
  }else if (millisec - time_send > 1000*60*5){
    time_send = millisec;
    post_data();
  }
}

void post_data() {
  String json = "{\"DVC\":\"" + String(deviceID) + "\",\"API\":\"log\",\"DATA\":{\"temp\":\"" + String(temperature);
  json += "\",\"humi\":\"" + String(humidity);
  json += "\",\"work\":" + String(work_heat);
  json += ",\"runt\":" + String(work_total) + "}}";
  Serial.println(json);  
  work_heat  = 0;
  work_total = 0;
  httpPOSTRequest(json);
}


////--------------------- service help ----------------////
void help() {
  Serial.println("************** help **************");
  Serial.println("run     * homeothermy mode, on/off");
  Serial.println("set     * temperature goal setup, 0~40. ex) set 25");
  Serial.println("config  * read temperature setup");
  Serial.println("show    * sensor value show");
  Serial.println("post    * sensor value post");
  Serial.println("reboot  * system reboot");
  Serial.println("ssid    * ex)ssid your ssid");
  Serial.println("pass    * ex)pass your password");
  Serial.println("wifi    * WIFI connect");
  Serial.println("**********************************");
}

////--------------------- service serial --------------////
void read_config() {
  Serial.print("Temperature goal: ");
  Serial.print(control_temperature);
  Serial.print(", Set Operation: ");
  Serial.println(use_stable);
}

void serial_monit() {
  Serial.print("TCA Port ");
  Serial.print(sht_port);
  Serial.print(", T: ");
  Serial.print(temperature);
  Serial.print("°C, H: ");
  Serial.print(humidity);
  Serial.println("%");
  Serial.print("USE_heater = ");
  Serial.print(use_stable);
  Serial.print(", heater ");
  Serial.print(run_heater);
  Serial.println(";");
}

// 전방 선언
void wifi_connect();

////--------------------- service serial --------------////
void command_Service(const String& command, const String& value) {
  if (command == "run") {
    bool stable = false;
    if (value == "on") stable = true;
    
    if (stable != use_stable) {
      use_stable = stable;
      EEPROM.write(EEP_stable, use_stable);
      EEPROM.commit();
      if (wifi_connected) {
        post_config("set", control_temperature, use_stable);
      }
    }
    
    Serial.print("Operation mode ");
    Serial.println(stable ? "ON" : "OFF");
  } else if (command == "set") {
    uint8_t goal = value.toInt();
    if (goal > 40) goal = 40;
    
    if (goal != control_temperature) {
      control_temperature = goal;
      EEPROM.write(EEP_temp, control_temperature);
      EEPROM.commit();
      if (wifi_connected) {
        post_config("set", control_temperature, use_stable);
      }
    }
    
    Serial.print("Temperature goal ");
    Serial.println(control_temperature);    
  } else if (command == "config") {
    read_config();
  } else if (command == "show") {
    serial_monit();
  } else if (command == "post") {
    post_callback(millis(), true);
  } else if (command == "reboot") {
    ESP.restart();
  } else if (command == "ssid") {
    WiFi.disconnect(true);
    Serial.print("ssid: ");
    if (value.length() > 0) {
      for (int index = 0; index < EEPROM_SIZE; index++) {
        if (index < value.length()) {
          Serial.print(value[index]);
          ssid[index] = value[index];
          EEPROM.write(eep_ssid + index, byte(value[index]));
        } else {
          ssid[index] = 0x00;
          EEPROM.write(eep_ssid + index, byte(0x00));
        }
      }
    }
    Serial.println("");
    EEPROM.commit();
    wifi_connected = false;
  } else if (command == "pass") {
    WiFi.disconnect(true);
    Serial.print("pass: ");
    if (value.length() > 0) {
      for (int index = 0; index < EEPROM_SIZE; index++) {
        if (index < value.length()) {
          Serial.print(value[index]);
          password[index] = value[index];
          EEPROM.write(eep_pass + index, byte(value[index]));
        } else {
          password[index] = 0x00;
          EEPROM.write(eep_pass + index, byte(0x00));
        }
      }
    }
    Serial.println("");
    EEPROM.commit();
    wifi_connected = false;
  } else if (command == "wifi") {
    wifi_connect();
  } else {
    help();
  }
}

void command_parser(const String& cmd) {
  Serial.print("cmd: ");
  Serial.println(cmd);
  
  uint8_t cmd_index = 0;
  String command = String_slice(&cmd_index, cmd, 0x20);
  String value = String_slice(&cmd_index, cmd, 0x00);
  command_Service(command, value);
}

void Serial_process(char ch) {
  if (ch == '\n') {
    Serial_buf[Serial_num] = 0x00;
    command_parser(Serial_buf);
    Serial_num = 0;
  } else if (ch != '\r' && (Serial_num < SERIAL_MAX - 1)) {
    Serial_buf[Serial_num++] = ch;
  }
}

////--------------------- set up ----------------------////
void setup() {
  // 핀 설정
  pinMode(RELAY_HEATER, OUTPUT);
  digitalWrite(RELAY_HEATER, pin_off);
  
  // 시리얼 및 I2C 초기화
  Serial.begin(115200);
  Wire.begin();
  Wire2.begin(SDA2, SCL2, 400000);
  
  // EEPROM 초기화
  if (!EEPROM.begin(EEPROM_SIZE * 2 + EEPROM_SET)) {
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }

  // EEPROM에서 설정 읽기
  for (int index = 0; index < EEPROM_SIZE; index++) {
    ssid[index] = EEPROM.read(eep_ssid + index);
    password[index] = EEPROM.read(eep_pass + index);
  }
  
  // 기본값 설정
  if (EEPROM.read(EEP_temp) == 255) {
    EEPROM.write(EEP_temp, 3);
    EEPROM.commit();
  }
  
  if (EEPROM.read(EEP_stable) == 255) {
    EEPROM.write(EEP_stable, 0);
    EEPROM.commit();
  }
  
  control_temperature = byte(EEPROM.read(EEP_temp));
  use_stable = (EEPROM.read(EEP_stable) != 0);
  
  // WiFi 연결
  wifi_connect();
  read_config();

  Serial.print("ID: ");
  Serial.println(deviceID);
  Serial.println("System online. ver 0.0.1");
  Serial.println("---------------------------");
  while (millis()<=500) {delay(1);}
  sensor_get(millis());
  post_callback(millis(), true);
}

void wifi_connect() {
  Serial.println("------- WiFi config -------");
  Serial.print("ssid: "); Serial.println(ssid);
  Serial.print("pass: "); Serial.println(password);
  Serial.println("---------------------------");
  
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long wifi_config_update = millis();
  bool wifi_connect_success = true;

  while (WiFi.status() != WL_CONNECTED) {
    if (Serial.available()) Serial_process(Serial.read());    
    // 10초마다 자동 연결 시도
    unsigned long update_time = millis();
    if (update_time - wifi_config_update > 10*1000) {
      wifi_config_update = update_time;
      Serial.println("Connecting to scan");
      
      // 자동 연결 시도
      WiFi.disconnect(true);
      int networks = WiFi.scanNetworks();
      
      for (int index = 0; index < networks; ++index) {
        String SSID = WiFi.SSID(index);
        // CNR_L580W_ 패턴 확인
        if (SSID.length() > 9 && SSID.startsWith("CNR_L580W_")) {
          WiFi.scanDelete();
          Serial.print("Found SSID: "); Serial.println(SSID);
          
          // SSID 저장
          for (int i = 0; i < EEPROM_SIZE; i++) {
            if (i < SSID.length()) {
              EEPROM.write(eep_ssid + i, byte(SSID[i]));
            } else {
              EEPROM.write(eep_ssid + i, byte(0x00));
            }      
          }
          
          // 비밀번호 저장
          String pass_value = "#234567!";
          for (int i = 0; i < EEPROM_SIZE; i++) {
            if (i < pass_value.length()) {
              EEPROM.write(eep_pass + i, byte(pass_value[i]));
            } else {
              EEPROM.write(eep_pass + i, byte(0x00));
            }    
          }
          
          EEPROM.commit();
          Serial.println("Device restart...");
          delay(100);
          ESP.restart();
        }
      }
      
      Serial.println("---- Available WiFi networks ----");
      for (int index = 0; index < networks; ++index) {
        Serial.print("\t"); Serial.print(index); Serial.print(": "); Serial.println(WiFi.SSID(index));
      }
      Serial.println("---------------------------");
      WiFi.scanDelete();
      wifi_connect_success = false;
      restart_count = 230; // 재시작 예정
      break;
    }
  }

  if (wifi_connect_success) {
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    wifi_connected = true;
  } else {
    Serial.println("WiFi connection failed");
    wifi_connected = false;
  }
  
  Serial.println("---------------------------");
  
  // 장치 ID 설정
  for (uint8_t index = 0; index < 17; index++) {
    deviceID[index] = (WiFi.macAddress()[index] == ':') ? '_' : WiFi.macAddress()[index];
  }
}

// WiFi 연결 체크 및 필요시 재연결 함수
void check_wifi_connection() {
  static unsigned long last_wifi_check = 0;
  unsigned long current_time = millis();
  
  // WiFi 연결 확인
  if (current_time - last_wifi_check > 1000*60*3) {
    last_wifi_check = current_time;
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost. Reconnecting...");
      wifi_connected = false;
      WiFi.disconnect();
      WiFi.reconnect();
      
      // 연결 시도 (최대 10초)
      unsigned long reconnect_start = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - reconnect_start < 10000) {
        delay(500);
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi reconnected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        wifi_connected = true;
      } else {
        Serial.println("WiFi reconnection failed");
      }
    }
  }
}

void loop() {
  unsigned long now = millis();
  
  if (Serial.available()) {
    Serial_process(Serial.read());
  }
  
  // WiFi 연결 상태 확인
  check_wifi_connection();
  
  // 센서 값 읽기
  sensor_get(now);
  
  // 안정화 제어
  stable(now);
  
  // 재시작 체크
  restart(now);
  
  // 데이터 전송
  if (wifi_connected) {
    post_callback(now, false);
  }
  
  // 짧은 지연으로 워치독 타이머 트리거 방지
  delay(10);
}

void stable(unsigned long millisec) {
  if ((millisec - time_stayble) > 1000) {
    time_stayble = millisec;
    run_heater = !isnan(temperature) && use_stable && (temperature < control_temperature);
    work_total += 1;
    
    if (run_heater) {
      digitalWrite(RELAY_HEATER, pin_on);
      work_heat += 1;
    } else {
      digitalWrite(RELAY_HEATER, pin_off);
    }
  }
}

void sensor_get(unsigned long millisec) {
  if ((millisec - timer_SHT) > 500) {
    timer_SHT = millisec;
    bool sensor_conn = false;
    
#ifdef SENSOR_SHT40
    if (sht40.begin()) {
      sht_port = 1;
      sensors_event_t humi, temp;
      sht40.getEvent(&humi, &temp);
      temperature = temp.temperature;
      humidity = humi.relative_humidity;
      sensor_conn = true;
    } else if (sht40.begin(Wire2)) {
      sht_port = 2;
      sensors_event_t humi, temp;
      sht40.getEvent(&humi, &temp);
      temperature = temp.temperature;
      humidity = humi.relative_humidity;
      sensor_conn = true;
    }
#else
    if (sht31_1.begin(0x44)) {
      sensor_conn = true;
      sht_port = 1;
      temperature = sht31_1.readTemperature();
      humidity = sht31_1.readHumidity();
      // reset 사용 최소화
      static unsigned long last_reset_time = 0;
      if (millisec - last_reset_time > 60000) { // 1분마다 리셋
        sht31_1.reset();
        last_reset_time = millisec;
      }
    } else if (sht31_2.begin(0x44)) {
      sensor_conn = true;
      sht_port = 2;
      temperature = sht31_2.readTemperature();
      humidity = sht31_2.readHumidity();
      // reset 사용 최소화
      static unsigned long last_reset_time2 = 0;
      if (millisec - last_reset_time2 > 60000) { // 1분마다 리셋
        sht31_2.reset();
        last_reset_time2 = millisec;
      }
    }
#endif
    
    if (!sensor_conn) {
      temperature = NAN;
      humidity = NAN;
    }
  }
}
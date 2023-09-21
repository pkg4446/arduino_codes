#include <Wire.h>                      // lcd I2C 통신 선언
#include <HTTPClient.h>
#include <Arduino_JSON.h> //https://github.com/arduino-libraries/Arduino_JSON
#include <WiFi.h>

#include <SHT1x.h>
#define dataPin  23   //MOSI
#define clockPin 18   //SCK
SHT1x sht1x(dataPin, clockPin);

#define Relay_1 25 //(Relay) 출력핀 설정    히터
#define Relay_2 26 //(Relay) 출력핀 설정    팬
#define Relay_3 27 //(Relay) 출력핀 설정    솔밸브 (설탕)

#define Level_sensor_1 32              //(Level_sensor_2) 입력핀 설정 : 설탕 위쪽
#define Level_sensor_2 33              //(Level_sensor_1) 입력핀 설정 : 설탕 아랫쪽

////for millis() func//
unsigned long sensorTime  = 0;
unsigned long postTime    = 0;
unsigned long receiveTime = 0;
unsigned long loopTime    = 0;
unsigned long LCD_Refresh = 0;

String Server       = "http://smarthive.kr/hive/";   //API adress
String SirialNumber;   //API adress

struct sendData   {
  String MESURE_VAL_01; //온도
  String MESURE_VAL_02; //습도
  String MESURE_VAL_03; //설탕
};

struct receiveData {
  String CONTROL_VAL_01; //온도
  String CONTROL_VAL_02; //습도
  String CONTROL_VAL_03; //설탕
};

//send data set start
struct sendData     dataSend;
struct receiveData  dataReceive;

boolean emergency = false;

void setup() {
  Serial.begin(115200);

  pinMode(Relay_1, OUTPUT);       //(Relay) 출력핀 설정    히터
  pinMode(Relay_2, OUTPUT);       //(Relay) 출력핀 설정    팬
  pinMode(Relay_3, OUTPUT);       //(Relay) 출력핀 설정    솔밸브

  pinMode(Level_sensor_1, INPUT_PULLDOWN); //(Level_sensor_1) 입력핀 설정 : 설탕 위쪽
  pinMode(Level_sensor_2, INPUT_PULLDOWN); //(Level_sensor_2) 입력핀 설정 : 설탕 아래쪽

  Serial.println("System loading...");

  WiFi.begin("Daesung2G", "smarthive123");
  SirialNumber = WiFi.macAddress();

  Serial.println(SirialNumber);

  dataReceive.CONTROL_VAL_01 = "30";
  dataReceive.CONTROL_VAL_02 = "40";
  //재부팅할 경우 서버 접속 시도 : 미등록장비 등록, 장비 설정 가져오기
  dataSend.MESURE_VAL_01 = String(sht1x.readTemperatureC());
  dataSend.MESURE_VAL_02 = String(sht1x.readHumidity());
  dataSend.MESURE_VAL_03 = "0";
  boolean pass = true;

  while (pass) {
    if (WiFi.status() == WL_CONNECTED) {
      pass = false;
      httpPOSTRequest(&dataSend, Server);
      httpGETRequest(&dataReceive, Server + "?eqnum=" + SirialNumber);
    }
  }

  Serial.println("System all green");
}////SetUP()END

int8_t emergencyCount = 0;

void loop() {
  ////////////////////////////////////////정기적 통신
  if ((millis() - sensorTime) > 1000) {
    ////센서 계측
    sensorTime = millis();
    dataSend.MESURE_VAL_01 = String(sht1x.readTemperatureC());
    dataSend.MESURE_VAL_02 = String(sht1x.readHumidity());
    dataSend.MESURE_VAL_03 = String(random(100));
    ////비접촉수위센서 테스트
    if (digitalRead(Level_sensor_1)) Serial.println("Level_sensor_1");
    if (digitalRead(Level_sensor_2)) Serial.println("Level_sensor_2");
  }//서버 정기보고

  ////////////////////////////////////////정기적 통신
  if ((millis() - postTime) > 1000 * 60 * 10) {
    ////AP동시접속 가능 갯수를 고려하여 AP접속 및 접속종료 추가할것
    ////서버와 통신 시작
    postTime = millis();

    if (WiFi.status() == WL_CONNECTED) {
      httpPOSTRequest(&dataSend, Server);
    }
    else {
      Serial.print("WiFi Disconnected");
    }
    ////서버와 통신 종료
  }//서버 정기보고

  if ((millis() - receiveTime) > 1000 * 60 * 5) {
    ////AP동시접속 가능 갯수를 고려하여 AP접속 및 접속종료 추가할것
    ////서버와 통신 시작
    receiveTime = millis();
    if (WiFi.status() == WL_CONNECTED) {
      httpGETRequest(&dataReceive, Server + "?eqnum=" + SirialNumber);
    }
    else {
      Serial.print("WiFi Disconnected");
    }
    ////서버와 통신 종료
  }//장치 설정 정보 가져옴

  ////여기서 부터 대전에서 릴레이 제어부분 추가하기
  if ((millis() - loopTime) > 1000 * 1 * 1) {
    //여기다가 벌통 항상성 유지 코드.
    if (dataSend.MESURE_VAL_01.toInt() > dataReceive.CONTROL_VAL_01.toInt() + 3) {
      Serial.println("히터 종료 및 팬 가동(비상사태)");
      emergency = true;
      //히터 종료 및 팬 가동(비상사태) //서버에 보고
      digitalWrite(Relay_1, false); //히터 끄기
      digitalWrite(Relay_2, true); //팬 켜기
      //하드웨어 동작 부분
    } else if (dataSend.MESURE_VAL_01.toInt() >= dataReceive.CONTROL_VAL_01.toInt()) {
      Serial.println("히터 끄기");
      //히터 종료
      digitalWrite(Relay_1, false); //히터 끄기
      digitalWrite(Relay_2, false); //팬 끄기
      //하드웨어 동작 부분
    } else if (dataSend.MESURE_VAL_01.toInt() < dataReceive.CONTROL_VAL_01.toInt() - 3) {
      Serial.println("히터 켜기");
      //히터 켜기
      digitalWrite(Relay_1, true); //히터 켜기
      digitalWrite(Relay_2, false); //팬 끄기
      //하드웨어 동작 부분
    }//온도 조절 종료

    if (dataReceive.CONTROL_VAL_03.toInt() == true) {
      if (digitalRead(Level_sensor_1) == false && digitalRead(Level_sensor_2) == false) {
        //서버에 보고
        Serial.println("설탕물 없음");
        emergency = true;
        dataSend.MESURE_VAL_03 = "no sugar";
        //설탕물 없음. 설탕물 공급
        digitalWrite(Relay_3, true); //설탕 공급
        //하드웨어 동작 부분
      } else if (digitalRead(Level_sensor_1) == false && digitalRead(Level_sensor_2) == true) {
        dataSend.MESURE_VAL_03 = "drinking sugar";
        //설탕물 소비 중
        //하드웨어 동작 부분        
        digitalWrite(Relay_3, false); //설탕 차단
      } else if (digitalRead(Level_sensor_1) == true && digitalRead(Level_sensor_2) == false) {
        //서버에 보고
        Serial.println("설탕물 센서 고장");
        emergency = true;
        dataSend.MESURE_VAL_03 = "sugar sensor error";        
        digitalWrite(Relay_3, false); //설탕 차단
        //센서 이상
        //하드웨어 동작 부분
      } else if (digitalRead(Level_sensor_1) == true && digitalRead(Level_sensor_2) == true) {
        //서버에 보고
        Serial.println("설탕물 가득참");
        emergency = true;
        dataSend.MESURE_VAL_03 = "no sugar";
        //만 수위
        digitalWrite(Relay_3, false); //설탕 차단
        //하드웨어 동작 부분
      }
    }//어플에서 설탕물을 자동공급하도록 설정한 경우

    if (emergency == true) {
      //플레그(이상사태)가 참일경우 서버에 보고
      if (emergencyCount == 0)
      {
        emergency = false;
        Serial.println("something wrong!");
        //여기에 post
        if (WiFi.status() == WL_CONNECTED) {
          httpPOSTRequest(&dataSend, Server);
        }        
      }
      ////서버와 통신 종료
      emergencyCount++;
      if (emergencyCount >= 60) emergencyCount = 0;
      Serial.println(emergencyCount);
    }
  }//벌통 제어
  ////대전에서 릴레이 제어부분 추가하기 종료.
}////loop()END

/////Receive Data/////////////////////////////////////
void httpGETRequest(struct receiveData *ptr, String serverUrl) {

  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverUrl);

  int httpResponseCode = http.GET();
  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();           // Free resources

  JSONVar myObject = JSON.parse(payload);

  Serial.print("JSON object = ");
  Serial.println(myObject);

  JSONVar keys = myObject.keys();
  String Keys;
  String Values[keys.length()];
  for (int i = 0; i < keys.length(); i++) {
    Keys     = JSON.stringify(keys[i]);
    if (Keys == "\"DATA_01\"") {
      ptr->CONTROL_VAL_01  = replacement(JSON.stringify(myObject[keys[i]]));
    } else if (Keys == "\"DATA_02\"") {
      ptr->CONTROL_VAL_02  = replacement(JSON.stringify(myObject[keys[i]]));
    } else if (Keys == "\"DATA_03\"") {
      ptr->CONTROL_VAL_03 = replacement(JSON.stringify(myObject[keys[i]]));
    }
  }
  Serial.print(ptr->CONTROL_VAL_01 );
  Serial.print(ptr->CONTROL_VAL_02 );
  Serial.println(ptr->CONTROL_VAL_03 );
}////httpGETRequest_End

String replacement(String str) {
  char str_r[8] = "";
  for (int i = 1; str[i] != '\"'; i++) {
    str_r[i - 1] = str[i];
  }
  return str_r;
}

////Send Data//////////////////////////////////////
void httpPOSTRequest(struct sendData *ptr, String serverUrl) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverUrl);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String httpRequestData = "eqnum="   + SirialNumber +
                           "&temp="   + ptr->MESURE_VAL_01 +
                           "&humi="   + ptr->MESURE_VAL_02 +
                           "&sugar=" + ptr->MESURE_VAL_03;                           

  int httpResponseCode = http.POST(httpRequestData);
  Serial.print(httpRequestData);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  http.end();           // Free resources
}////httpPOSTRequest_End

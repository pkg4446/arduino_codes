#include <Wire.h>                      // lcd I2C 통신 선언
#include <HTTPClient.h>
#include <Arduino_JSON.h> //https://github.com/arduino-libraries/Arduino_JSON
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>         //https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
LiquidCrystal_I2C lcd(0x27, 20, 4);    // I2C lcd 주소값 확인 , 20문자에 , 4열
// 0x3F, 0x27 둘중 하나의 주소를 사용하시면 됩니다

#include <SHT1x.h>
#define dataPin  23   //MOSI
#define clockPin 18   //SCK
SHT1x sht1x(dataPin, clockPin);

#define Relay_1 25 //(Relay) 출력핀 설정    히터
#define Relay_2 2  //(Relay) 출력핀 설정    가습기
#define Relay_3 4  //(Relay) 출력핀 설정    팬
#define Relay_4 16 //(Relay) 출력핀 설정    솔밸브 (물)
#define Relay_5 17 //(Relay) 출력핀 설정    솔밸브 (설탕)
#define Relay_6 5

#define Level_sensor_1 33              //27번 핀 (Level_sensor_1) 입력핀 설정 : 물 아랫쪽
#define Level_sensor_2 32              //14번 핀 (Level_sensor_2) 입력핀 설정 : 물 위쪽
#define Level_sensor_3 34              //12번 핀 (Level_sensor_3) 입력핀 설정 : 설탕 아랫쪽
#define Level_sensor_4 36              //13번 핀 (Level_sensor_4) 입력핀 설정 : 설탕 위쪽

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
  String MESURE_VAL_03; //화분
  String MESURE_VAL_04; //무게
  String MESURE_VAL_05; //설탕
  String MESURE_VAL_06; //물
  String MESURE_VAL_07; //이산화탄소
  String MESURE_VAL_08; //암모니아
  String MESURE_VAL_09; //진동
  String MESURE_VAL_10; //소음
  String MESURE_VAL_11; //밝기
};

struct receiveData {
  String CONTROL_VAL_01; //온도
  String CONTROL_VAL_02; //습도
  String CONTROL_VAL_03; //설탕
  String CONTROL_VAL_04; //물
  String CONTROL_VAL_05;
  String CONTROL_VAL_06;
  String CONTROL_VAL_07;
};

//send data set start
struct sendData     dataSend;
struct receiveData  dataReceive;

boolean emergency = false;

void setup() {
  Serial.begin(115200);

  pinMode(Relay_1, OUTPUT);             //(Relay) 출력핀 설정    히터
  pinMode(Relay_2, OUTPUT);             //(Relay) 출력핀 설정    가습기
  pinMode(Relay_3, OUTPUT);             //(Relay) 출력핀 설정    팬
  pinMode(Relay_4, OUTPUT);             //(Relay) 출력핀 설정    솔밸브 (물)
  pinMode(Relay_5, OUTPUT);             //(Relay) 출력핀 설정    솔밸브 (설탕)
  pinMode(Relay_6, OUTPUT);             //핀(Relay) 출력핀 설정

  pinMode(Level_sensor_1, INPUT);     //27번 핀(Level_sensor_1) 입력핀 설정 : 물 아랫쪽
  pinMode(Level_sensor_2, INPUT);     //27번 핀(Level_sensor_2) 입력핀 설정 : 물 위쪽
  pinMode(Level_sensor_3, INPUT);     //27번 핀(Level_sensor_3) 입력핀 설정 : 설탕 아랫쪽
  pinMode(Level_sensor_4, INPUT);     //27번 핀(Level_sensor_4) 입력핀 설정 : 설탕 위쪽

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
  dataSend.MESURE_VAL_04 = "0";
  dataSend.MESURE_VAL_05 = "0";
  dataSend.MESURE_VAL_06 = "0";
  dataSend.MESURE_VAL_07 = "0";
  dataSend.MESURE_VAL_08 = "0";
  dataSend.MESURE_VAL_09 = "0";
  dataSend.MESURE_VAL_10 = "0";
  dataSend.MESURE_VAL_11 = "0";
  boolean pass = true;

  // 커서 설정시 0부터 시작하기 때문에 첫문자는 0으로 설정
  lcd.begin();                          // lcd 초기화
  lcd.backlight();                     // lcd 백라이트를 켠다
  lcd.setCursor(0, 0);                 // 커서를 첫번째 문자 첫번째 라인으로 설정
  lcd.print("DAESUNG");                // "   " 안에 원하는 문자를 출력
  lcd.setCursor(0, 1);                 // 커서를 두번째 문자 두번째 라인으로 설정
  lcd.print("HIVE_MONITOR");           // "   " 안에 원하는 문자를 출력
  lcd.setCursor(0, 2);                 // 커서를 세번째 문자 세번째 라인으로 설정
  lcd.print("V.1.0.0");                // "   " 안에 원하는 문자를 출력
  lcd.setCursor(0, 3);                 // 커서를 네번째 문자 네번째 라인으로 설정
  lcd.print("2022-01-18");             // "   " 안에 원하는 문자를 출력

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
    dataSend.MESURE_VAL_11 = String(random(100));
    ////비접촉수위센서 테스트
    if (digitalRead(Level_sensor_1)) Serial.println("Level_sensor_1");
    if (digitalRead(Level_sensor_2)) Serial.println("Level_sensor_2");
    if (digitalRead(Level_sensor_3)) Serial.println("Level_sensor_3");
    if (digitalRead(Level_sensor_4)) Serial.println("Level_sensor_4");
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
      while (true) {
        lcd.begin();                          // lcd 초기화
        lcd.backlight();                     // lcd 백라이트를 켠다
        lcd.setCursor(0, 0);                 // 커서를 첫번째 문자 첫번째 라인으로 설정
        lcd.print("WiFi Disconnected");                // "   " 안에 원하는 문자를 출력
        lcd.setCursor(0, 1);                 // 커서를 두번째 문자 두번째 라인으로 설정
        lcd.print("WiFi Disconnected");           // "   " 안에 원하는 문자를 출력
        lcd.setCursor(0, 2);                 // 커서를 세번째 문자 세번째 라인으로 설정
        lcd.print("WiFi Disconnected");                // "   " 안에 원하는 문자를 출력
        lcd.setCursor(0, 3);                 // 커서를 네번째 문자 네번째 라인으로 설정
        lcd.print("WiFi Disconnected");             // "   " 안에 원하는 문자를 출력
        delay (1000);
      }
    }
    ////서버와 통신 종료
  }//장치 설정 정보 가져옴

  //////////////////////////////////////////LCD 갱신
  if ((millis() - LCD_Refresh) > 1000 * 1 * 10) {
    LCD_Refresh = millis();
    lcd.begin();                           // lcd 초기화
    lcd.clear();                          // lcd 지우기
    //온도 측정 후 LCD 표시
    lcd.setCursor(0, 0);                  // 커서를 첫번째 문자 첫번째 라인으로 설정
    lcd.print("Temp.:");                  // "   " 안에 원하는 문자를 출력
    lcd.setCursor(8, 0);                  // 커서를 첫번째 문자 첫번째 라인으로 설정
    lcd.print(dataSend.MESURE_VAL_01); // temp_c를 소숫점 1째 자리로 출력
    lcd.setCursor(13, 0);                 // 커서를 첫번째 문자 첫번째 라인으로 설정
    lcd.print("'C");                      // "   " 안에 원하는 문자를 출력
    //습도 측정 후 LCD 표시
    lcd.setCursor(0, 1);                  // 커서를 첫번째 문자 첫번째 라인으로 설정
    lcd.print("Humi.:");                  // "   " 안에 원하는 문자를 출력
    lcd.setCursor(8, 1);                  // 커서를 첫번째 문자 첫번째 라인으로 설정
    lcd.print(dataSend.MESURE_VAL_02); // humidity를 소숫점 1째 자리로 출력
    lcd.setCursor(13, 1);                 // 커서를 첫번째 문자 첫번째 라인으로 설정
    lcd.print("%");                       // "   " 안에 원하는 문자를 출력
    //습도 측정 후 LCD 표시
    lcd.setCursor(0, 2);                  // 커서를 첫번째 문자 첫번째 라인으로 설정
    lcd.print("TempS.:");                  // "   " 안에 원하는 문자를 출력
    lcd.setCursor(8, 2);                  // 커서를 첫번째 문자 첫번째 라인으로 설정
    lcd.print(dataReceive.CONTROL_VAL_01); // humidity를 소숫점 1째 자리로 출력
    lcd.setCursor(13, 2);                 // 커서를 첫번째 문자 첫번째 라인으로 설정
    lcd.print("%");                       // "   " 안에 원하는 문자를 출력
    //습도 측정 후 LCD 표시
    lcd.setCursor(0, 3);                  // 커서를 첫번째 문자 첫번째 라인으로 설정
    lcd.print("HumiS.:");                  // "   " 안에 원하는 문자를 출력
    lcd.setCursor(8, 3);                  // 커서를 첫번째 문자 첫번째 라인으로 설정
    lcd.print(dataReceive.CONTROL_VAL_02); // humidity를 소숫점 1째 자리로 출력
    lcd.setCursor(13, 3);                 // 커서를 첫번째 문자 첫번째 라인으로 설정
    lcd.print("%");                       // "   " 안에 원하는 문자를 출력
  }//LCD 갱신 종료

  ////여기서 부터 대전에서 릴레이 제어부분 추가하기
  if ((millis() - loopTime) > 1000 * 1 * 1) {
    //여기다가 벌통 항상성 유지 코드.
    if (dataSend.MESURE_VAL_01.toInt() > dataReceive.CONTROL_VAL_01.toInt() + 3) {
      Serial.println("히터 종료 및 팬 가동(비상사태)");
      emergency = true;
      //히터 종료 및 팬 가동(비상사태) //서버에 보고
      digitalWrite(Relay_1, false); //히터 끄기
      digitalWrite(Relay_3, true); //팬 켜기
      //하드웨어 동작 부분
    } else if (dataSend.MESURE_VAL_01.toInt() >= dataReceive.CONTROL_VAL_01.toInt()) {
      Serial.println("히터 끄기");
      //히터 종료
      digitalWrite(Relay_1, false); //히터 끄기
      //하드웨어 동작 부분
    } else if (dataSend.MESURE_VAL_01.toInt() < dataReceive.CONTROL_VAL_01.toInt() - 3) {
      Serial.println("히터 켜기");
      //히터 켜기
      digitalWrite(Relay_1, true); //히터 켜기
      digitalWrite(Relay_3, false); //팬 끄기
      //하드웨어 동작 부분
    }//온도 조절 종료

    if (dataSend.MESURE_VAL_02.toInt() > dataReceive.CONTROL_VAL_02.toInt() + 20) {
      Serial.println("가습 종료 및 팬 가동(과습)");
      emergency = true;
      //가습 종료 및 팬 가동(과습) //서버에 보고
      digitalWrite(Relay_2, false); //가습 끄기
      digitalWrite(Relay_3, true); //팬 켜기
      //하드웨어 동작 부분
    } else if (dataSend.MESURE_VAL_02.toInt() >= dataReceive.CONTROL_VAL_02.toInt()) {
      Serial.println("가습기 끄기");
      //가습 종료
      digitalWrite(Relay_2, false); //가습 끄기
      //하드웨어 동작 부분
    } else if (dataSend.MESURE_VAL_02.toInt() < dataReceive.CONTROL_VAL_02.toInt() - 3) {
      Serial.println("가습기 켜기");
      //가습
      digitalWrite(Relay_2, true); //가습 켜기
      digitalWrite(Relay_3, false); //팬 끄기
      //하드웨어 동작 부분
    }//습기조절 종료

    if (dataReceive.CONTROL_VAL_03.toInt() == true) {
      if (digitalRead(Level_sensor_1) == false && digitalRead(Level_sensor_2) == false) {
        //서버에 보고
        Serial.println("물 없음");
        emergency = true;
        dataSend.MESURE_VAL_03 = "no water";
        //물 없음. 물 공급
        digitalWrite(Relay_4, true); //물 공급
        //하드웨어 동작 부분
      } else if (digitalRead(Level_sensor_1) == true && digitalRead(Level_sensor_2) == false) {
        dataSend.MESURE_VAL_03 = "drinking water";
        digitalWrite(Relay_4, false); //물 차단
        //물 소비 중
        //하드웨어 동작 부분
      } else if (digitalRead(Level_sensor_1) == false && digitalRead(Level_sensor_2) == true) {
        //서버에 보고
        Serial.println("수위센서 고장");
        emergency = true;
        dataSend.MESURE_VAL_03 = "water sensor error";
        digitalWrite(Relay_4, false); //물 차단
        //센서 이상
        //하드웨어 동작 부분
      } else if (digitalRead(Level_sensor_1) == true && digitalRead(Level_sensor_2) == true) {
        //서버에 보고
        Serial.println("물 가득참");
        emergency = true;
        digitalWrite(Relay_4, false); //물 차단
        dataSend.MESURE_VAL_03 = "no water";
        //만 수위
        //하드웨어 동작 부분
      }
    }//어플에서 물을 자동공급하도록 설정한 경우에

    if (dataReceive.CONTROL_VAL_04.toInt() == true) {
      if (digitalRead(Level_sensor_3) == false && digitalRead(Level_sensor_4) == false) {
        //서버에 보고
        Serial.println("설탕물 없음");
        emergency = true;
        dataSend.MESURE_VAL_03 = "no sugar";
        //설탕물 없음. 설탕물 공급
        digitalWrite(Relay_5, true); //설탕 공급
        //하드웨어 동작 부분
      } else if (digitalRead(Level_sensor_3) == true && digitalRead(Level_sensor_4) == false) {
        dataSend.MESURE_VAL_03 = "drinking sugar";
        //설탕물 소비 중
        //하드웨어 동작 부분        
        digitalWrite(Relay_5, false); //설탕 차단
      } else if (digitalRead(Level_sensor_3) == false && digitalRead(Level_sensor_4) == true) {
        //서버에 보고
        Serial.println("설탕물 센서 고장");
        emergency = true;
        dataSend.MESURE_VAL_03 = "sugar sensor error";        
        digitalWrite(Relay_5, false); //설탕 차단
        //센서 이상
        //하드웨어 동작 부분
      } else if (digitalRead(Level_sensor_3) == true && digitalRead(Level_sensor_4) == true) {
        //서버에 보고
        Serial.println("설탕물 가득참");
        emergency = true;
        dataSend.MESURE_VAL_03 = "no sugar";
        //만 수위
        digitalWrite(Relay_5, false); //설탕 차단
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
        else {
          Serial.print("WiFi Disconnected");
          lcd.begin();                          // lcd 초기화
          lcd.backlight();                     // lcd 백라이트를 켠다
          lcd.setCursor(0, 0);                 // 커서를 첫번째 문자 첫번째 라인으로 설정
          lcd.print("WiFi Disconnected");                // "   " 안에 원하는 문자를 출력
          lcd.setCursor(0, 1);                 // 커서를 두번째 문자 두번째 라인으로 설정
          lcd.print("WiFi Disconnected");           // "   " 안에 원하는 문자를 출력
          lcd.setCursor(0, 2);                 // 커서를 세번째 문자 세번째 라인으로 설정
          lcd.print("WiFi Disconnected");                // "   " 안에 원하는 문자를 출력
          lcd.setCursor(0, 3);                 // 커서를 네번째 문자 네번째 라인으로 설정
          lcd.print("WiFi Disconnected");             // "   " 안에 원하는 문자를 출력
          delay (5000);
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
    } else if (Keys == "\"DATA_04\"") {
      ptr->CONTROL_VAL_04 = replacement(JSON.stringify(myObject[keys[i]]));
    } else if (Keys == "\"DATA_05\"") {
      ptr->CONTROL_VAL_05 = replacement(JSON.stringify(myObject[keys[i]]));
    } else if (Keys == "\"DATA_06\"") {
      ptr->CONTROL_VAL_06 = replacement(JSON.stringify(myObject[keys[i]]));
    } else if (Keys == "\"DATA_07\"") {
      ptr->CONTROL_VAL_07 = replacement(JSON.stringify(myObject[keys[i]]));
    }
  }
  Serial.print(ptr->CONTROL_VAL_01 );
  Serial.print(ptr->CONTROL_VAL_02 );
  Serial.print(ptr->CONTROL_VAL_03 );
  Serial.print(ptr->CONTROL_VAL_04 );
  Serial.print(ptr->CONTROL_VAL_05 );
  Serial.print(ptr->CONTROL_VAL_06 );
  Serial.println(ptr->CONTROL_VAL_07 );
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
                           "&flower=" + ptr->MESURE_VAL_03 +
                           "&weigh="  + ptr->MESURE_VAL_04 +
                           "&sugar="  + ptr->MESURE_VAL_05 +
                           "&water="  + ptr->MESURE_VAL_06 +
                           "&co2="    + ptr->MESURE_VAL_07 +
                           "&nh3="    + ptr->MESURE_VAL_08 +
                           "&noise="  + ptr->MESURE_VAL_09 +
                           "&vib="    + ptr->MESURE_VAL_10 +
                           "&light="  + ptr->MESURE_VAL_11;

  int httpResponseCode = http.POST(httpRequestData);
  Serial.print(httpRequestData);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  http.end();           // Free resources
}////httpPOSTRequest_End

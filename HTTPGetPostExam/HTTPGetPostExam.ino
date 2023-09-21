#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>
#include <Arduino_JSON.h> //https://github.com/arduino-libraries/Arduino_JSON
#define SERIAL_MAX  256
#include <Adafruit_AHT10.h>
Adafruit_AHT10 aht;


////for millis() func//
unsigned long lastTime   = 0;
unsigned long timerDelay = 1000*60*5;

String SirialNumber = "002";                         //Device number
String Server       = "http://192.168.1.15:3004/hive/";    //API adress
//String Server       = "http://smarthive.kr/hive/";    //API adress


struct dataSet {
  String MESURE_VAL_01;
  String MESURE_VAL_02;
  String MESURE_VAL_03;
  String MESURE_VAL_04;
};

////AT Code for chage the Something//////////////////////////
char Serial_buf[SERIAL_MAX];
int16_t Serial_num;

void AT_commandHelp() {
  Serial.println("---------------- AT command help --------------");
  Serial.println("AT+SN=xxxxxxx              SirialNumber Change.");
  Serial.println("AT+Echo=xxxxx              AT Code Ehco Test.");
}

void Serial_service() {
  String str1 = strtok(Serial_buf, "=");
  String str2 = strtok(NULL, " ");
  if (str1 == "AT+SN") {
    Serial.print("시리얼 넘버 : ");
    Serial.println(str2);
  } else if (str1 == "AT+Echo") {
    Serial.print("Echo : ");
    Serial.println(str2);
  }
  else {
    AT_commandHelp();
  }
}

void Serial_process() {
  char ch;
  ch = Serial.read();
  switch ( ch ) {
    case '\n':
      Serial_buf[Serial_num] = NULL;
      Serial.print("ehco : ");
      Serial.println(Serial_buf);
      Serial_service();
      Serial_num = 0;
      break;
    default :
      Serial_buf[ Serial_num ++ ] = ch;
      Serial_num %= SERIAL_MAX;
      break;
  }
}
////End of AT Code for chage the Something///////////////////

void setup() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
  if (! aht.begin()) {
    Serial.println("Could not find AHT10? Check wiring");
    while (1) delay(10);
  }
  WiFiManager wm;
  bool res = wm.autoConnect("AutoConnectAP", "password");
  AT_commandHelp();
}////SetUP()END

void loop() {
  if (Serial.available()) {
    Serial_process();
  }
  
  struct dataSet dataReceive;

  if ((millis() - lastTime) > timerDelay) {
    //send data set start
    struct dataSet dataSend;
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    
    dataSend.MESURE_VAL_01 = String(temp.temperature);
    dataSend.MESURE_VAL_02 = String(humidity.relative_humidity);
    ////sample data set start
    dataSend.MESURE_VAL_03 = String(random(2) + 3);
    dataSend.MESURE_VAL_04 = String(random(100));
    //sample data set end
    //send data set end
    
    if (WiFi.status() == WL_CONNECTED) {
      httpPOSTRequest(&dataSend, Server);      
      httpGETRequest(&dataReceive, Server + SirialNumber + "/1");
      Serial.println(dataReceive.MESURE_VAL_01);
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}////loop()END


/////Receive Data/////////////////////////////////////
void httpGETRequest(struct dataSet *ptr, String serverUrl) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverUrl);

  int httpResponseCode = http.GET();
  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
    payload.replace("[", " ");
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
    if (Keys == "\"MESURE_VAL_01\"") {
      ptr->MESURE_VAL_01  = JSON.stringify(myObject[keys[i]]);
    } else if (Keys == "\"MESURE_VAL_02\"") {
      ptr->MESURE_VAL_02  = JSON.stringify(myObject[keys[i]]);
    } else if (Keys == "\"MESURE_VAL_03\"") {
      ptr->MESURE_VAL_03 = JSON.stringify(myObject[keys[i]]);
    } else if (Keys == "\"MESURE_VAL_04\"") {
      ptr->MESURE_VAL_04 = JSON.stringify(myObject[keys[i]]);
    }
  }
}////httpGETRequest_End

////Send Data//////////////////////////////////////
void httpPOSTRequest(struct dataSet *ptr, String serverUrl) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverUrl);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String httpRequestData = "eqnum="  + SirialNumber +
                           "&temp="  + ptr->MESURE_VAL_01 +
                           "&humi="  + ptr->MESURE_VAL_02 +
                           "&weigh=" + ptr->MESURE_VAL_03 +
                           "&sugar=" + ptr->MESURE_VAL_04;

  int httpResponseCode = http.POST(httpRequestData);
  Serial.print(httpRequestData);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  http.end();           // Free resources
}////httpPOSTRequest_End

#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>
#define SERIAL_MAX  256

////for millis() func//
unsigned long lastTime   = 0;
unsigned long timerDelay = 5*1000;

String SirialNumber = "0000";                         //Device number
String Server       = "http://smarthive.kr:3000/device/sensor/log/";     //API adress

struct dataSet {
  String MESURE_VAL_01;
  String MESURE_VAL_02;
  String MESURE_VAL_03;
  String MESURE_VAL_04;
  String MESURE_VAL_05;
  String MESURE_VAL_06;
  String MESURE_VAL_07;
  String MESURE_VAL_08;
};

void setup() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
  WiFiManager wm;
  bool res = wm.autoConnect("smartHive");
  SirialNumber = WiFi.macAddress();
}////SetUP()END

void loop() {
  struct dataSet dataSend;
  struct dataSet dataReceive;

  ////sample data set start
  dataSend.MESURE_VAL_01 = String(random(100));
  dataSend.MESURE_VAL_02 = String(random(100));
  dataSend.MESURE_VAL_03 = String(random(100));
  dataSend.MESURE_VAL_04 = String(random(100));
  dataSend.MESURE_VAL_05 = String(random(100));
  dataSend.MESURE_VAL_06 = String(random(100));
  dataSend.MESURE_VAL_07 = String(random(100));
  dataSend.MESURE_VAL_08 = String(random(100));
  ////sample data set end

  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      httpPOSTRequest(&dataSend, Server);
      Serial.println(dataReceive.MESURE_VAL_01);
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}////loop()END

////Send Data//////////////////////////////////////
void httpPOSTRequest(struct dataSet *ptr, String serverUrl) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverUrl);

  http.addHeader("Content-Type", "application/json");
  String httpRequestData =  (String)"{\"MD_ID\" : \"" + SirialNumber + "\"," +
                            "\"TMPR\"   :" + ptr->MESURE_VAL_01 + "," +
                            "\"HMDT\"   :" + ptr->MESURE_VAL_02 + "," +
                            "\"CD\"     :" + ptr->MESURE_VAL_03 + "," +
                            "\"AMN\"    :" + ptr->MESURE_VAL_04 + "," +
                            "\"HYD_SLF\":" + ptr->MESURE_VAL_05 + "," +
                            "\"CO2\"    :" + ptr->MESURE_VAL_06 + "," +
                            "\"MTHN\"   :" + ptr->MESURE_VAL_07 + "," +
                            "\"VOCS\"   :" + ptr->MESURE_VAL_08 + "}";

  int httpResponseCode = http.POST(httpRequestData);

  Serial.println(serverUrl);
  Serial.println(httpRequestData);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);

  String payload = http.getString();
  Serial.print("JSON object = ");
  Serial.println(payload);

  http.end();           // Free resources
}////httpPOSTRequest_End

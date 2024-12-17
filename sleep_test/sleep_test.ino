#include <WiFi.h>
#include <HTTPClient.h>

#define uS_TO_S_FACTOR 1000000  //Conversion factor for micro seconds to seconds
#define SECONDE        1000L

#define SSID "Daesung"  //if connecting from another corporation, use identity@organization.domain in Eduroam
#define PASS "smarthive123"  //your Eduroam password

RTC_DATA_ATTR uint16_t bootCount = 0;

void setup(){
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(2, INPUT);
  //Increment boot number and print it every reboot
  ++bootCount;

  bool wifi_able = true;
  unsigned long wifi_config_update  = millis();
  while (WiFi.status() != WL_CONNECTED) {
    unsigned long update_time = millis();
    if(update_time - wifi_config_update > SECONDE*3){
      wifi_able = false;
      break;
    }
  }

  bool pin_state = digitalRead(2);

  while (pin_state){
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(500);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    delay(500);
    pin_state = digitalRead(2);
  }

  if(wifi_able) httpPOSTRequest("http://192.168.1.2:3002/");
  //Set timer to 5 seconds
  esp_sleep_enable_timer_wakeup(5 * uS_TO_S_FACTOR);
  //Go to sleep now
  esp_deep_sleep_start();
}

void loop(){}

////Send Data//////////////////////////////////////
void httpPOSTRequest(String serverUrl) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverUrl);

  http.addHeader("Content-Type", "application/json");
  String httpRequestData =  (String)"{\"dev\":\"esp32c6\",\"count\":"+String(bootCount)+"}";

  int httpResponseCode = http.POST(httpRequestData);
  http.end();           // Free resources
}////httpPOSTRequest_End

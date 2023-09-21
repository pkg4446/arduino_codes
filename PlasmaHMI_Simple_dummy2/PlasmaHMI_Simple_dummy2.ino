#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>
#define SERIAL_MAX  256

////for millis() func//
unsigned long prevUpdatePlasma  = 0UL;
unsigned long prevUpdateSensor  = 0UL;
unsigned long UpdateSensor      = 0UL;


//// ------------ Value ------------
uint8_t   onTime  = 100;
uint16_t  offTime = 120 - onTime;
boolean   plasmaPost  = false;
boolean   PlasmaOnOff = true;

uint32_t  PlasmaOn    = true;
uint32_t  PlasmaOff   = true;

String   deviceID;

int8_t ppm_O3 = 0;

uint8_t ppm_O3_U = 0;
uint8_t ppm_O3_D = 0;


void setup() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
  WiFiManager wm;
  bool res = wm.autoConnect("SmartHivePlasma");
  deviceID = WiFi.macAddress();
  Serial.println(deviceID);

  PlasmaOn  = onTime * 60;
  PlasmaOff = offTime * 60;
}////SetUP()END

void loop() {
  PlasmaSchedule(millis());
  Sensor_O3(millis());
}////loop()END

//// ------------ Plasma ------------
void PlasmaSchedule(unsigned long currentTime) {
  if (currentTime > prevUpdatePlasma + 1000) {
    prevUpdatePlasma = currentTime;
    if (PlasmaOnOff) {
      if (PlasmaOn < 1) {
        Serial.println("Plasma OFF");
        PlasmaOnOff = false;
        PlasmaOn = onTime * 60;
      }
      PlasmaOn--;
      Serial.print("ON: ");
      Serial.println(PlasmaOn);
    }
    //// ------------ Plasma Off ------------
    else {
      if (PlasmaOff < 1) {
        Serial.println("Plasma ON");
        PlasmaOnOff = true;
        PlasmaOff   = offTime * 60;
      }
      PlasmaOff--;
      Serial.print("OFF: ");
      Serial.println(PlasmaOff);
    }

    if (plasmaPost != PlasmaOnOff) {
      plasmaPost = PlasmaOnOff;
      httpPOSTRequest("PLZ", String(PlasmaOnOff));
    }
  }//millis()
}//plasma

//// ------------ Sensor_O3 ------------
void Sensor_O3(unsigned long currentTime) {

  if (currentTime > UpdateSensor + 1000 * 60) {
    UpdateSensor = currentTime;
    if (PlasmaOnOff) {
      Serial.print("On:");
      ppm_O3_U++;
      Serial.print(ppm_O3_U);
      Serial.print(", O3:");
      if (ppm_O3_U > 6) {
        ppm_O3_U = 0;
        ppm_O3  += 1;
      }
      Serial.println(ppm_O3);
    } else {
      Serial.print("OFF:");
      ppm_O3_D++;
      Serial.print(ppm_O3_D);
      Serial.print(", O3:");
      if (ppm_O3_D > 1) {
        ppm_O3_D = 0;
        ppm_O3   = ppm_O3/2;
      }
      Serial.println(ppm_O3);
    }
    if (ppm_O3 < 0)   ppm_O3 = 0;
    if (ppm_O3 > 100) ppm_O3 = 1;
  }

  if (currentTime > prevUpdateSensor + 1000 * 60 * 5) {
    prevUpdateSensor = currentTime;
    httpPOSTRequest("O3", String(ppm_O3 + random(-1,1) ));
  }

}//Sensor_O3()

void httpPOSTRequest(String TYPE, String DATA) {
  String serverUrl = "http://smarthive.kro.kr/reg/save/"; //API adress
  HTTPClient http;
  WiFiClient http_client;
  http.begin(http_client, serverUrl);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String httpRequestData = (String)"MODULE="  + deviceID +
                           "&TYPE="         + TYPE +
                           "&DATA="         + DATA;

  int httpResponseCode = http.POST(httpRequestData);
  Serial.print(httpRequestData);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  http.end();           // Free resources
}////httpPOSTRequest_End

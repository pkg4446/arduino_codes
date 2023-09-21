#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>
#define SERIAL_MAX  256

////for millis() func//
unsigned long lastTime   = 0;
unsigned long timerDelay = 5 * 60 * 1000;

String SirialNumber = "0000";                         //Device number
String Server       = "https://plasmaapi.smarthive.work/api/daesung/sensor/";     //API adress

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

  const char* rootCACertificate = \
                                  "-----BEGIN CERTIFICATE-----\n" \
                                  "MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n" \
                                  "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
                                  "DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n" \
                                  "SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n" \
                                  "GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n" \
                                  "AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n" \
                                  "q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n" \
                                  "SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n" \
                                  "Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n" \
                                  "a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n" \
                                  "/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n" \
                                  "AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n" \
                                  "CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n" \
                                  "bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n" \
                                  "c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n" \
                                  "VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n" \
                                  "ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n" \
                                  "MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n" \
                                  "Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n" \
                                  "AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n" \
                                  "uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n" \
                                  "wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n" \
                                  "X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n" \
                                  "PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n" \
                                  "KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n" \
                                  "-----END CERTIFICATE-----\n";


  WiFiClientSecure client;
  client.setCACert(rootCACertificate);
  client.setInsecure();
  client.connect(serverUrl.c_str(), 443);

  HTTPClient http;
  http.begin(client, serverUrl);

  http.addHeader("Content-Type", "application/json");
  String httpRequestData =  (String)"{\"token\" : \"safemotion\","  +
                            "\"macAdd\" : \"" + SirialNumber + "\"," +
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

#include <Arduino.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>
#include <SocketIOclient.h>
#include <ArduinoJson.h>
#define SERIAL_MAX  256

SocketIOclient socketIO;
unsigned long messageTimestamp = 0;

String Name       = "101";
String Token      = "asdasdasdsads";
String Server     = "http://192.168.1.11:3000/hive";    //API 주소
String serverName = Server + "/" + Name + "/" + Token;

struct dataSet {
  String Temp;
  String Humi;
  String Batt; 
  String Net;
};

struct dataSet dataSend;
struct dataSet dataReceive;

////AT Code for chage the Name & Token///////////////////
char Serial_buf[SERIAL_MAX];
int16_t Serial_num;

void AT_commandHelp() {
  Serial.println("------------ AT command help --------");
  Serial.println("AT+ID=xxxxxxx            Change Name" );
  Serial.println("AT+Token=xxxx            Change Token");
}

void Serial_service() {
  String str1 = strtok(Serial_buf, "=");
  String str2 = strtok(NULL, " ");
  if (str1 == "AT+ID"){
    Serial.print("이름 : ");
    Serial.println(str2);
  } else if (str1 == "AT+Token") {
    Serial.print("토큰 : ");
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
/////////////////////////////////////////////////////


void setup() {
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    Serial.begin(115200);
    WiFiManager wm;
    bool res;
    res = wm.autoConnect("AutoConnectAP","password"); // password protected ap
    socketIO.begin("192.168.1.11", 3000, "/socket.io/?EIO=4");
    socketIO.onEvent(socketIOEvent);
}

void loop() {
    socketIO.loop();

    uint64_t now = millis();
    if(now - messageTimestamp > 10000) {
        messageTimestamp = now;

        DynamicJsonDocument doc(1024);
        JsonArray array = doc.to<JsonArray>();
        array.add("reply");
        array.add("messageTest");
        String output;
        serializeJson(doc, output);
        
        socketIO.sendEVENT(output);
    }
}


void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case sIOtype_DISCONNECT:
            Serial.printf("[IOc] Disconnected!\n");
            break;
        case sIOtype_CONNECT:
            Serial.printf("[IOc] Connected to url: %s\n", payload);
            // join default namespace (no auto join in Socket.IO V3)
            break;
        case sIOtype_EVENT:
        {
            char * sptr = NULL;
            int id = strtol((char *)payload, &sptr, 10);
            Serial.printf("[IOc] get event: %s id: %d\n", payload, id);

            if (WiFi.status() == WL_CONNECTED) {
              //httpPOSTRequest(&dataSend);
              //httpGETRequest(&dataReceive);
              Serial.println(dataReceive.Temp);
            }
            else {
              Serial.println("WiFi Disconnected");
            }
        }
            break;
        case sIOtype_ACK:
            Serial.printf("[IOc] get ack: %u\n", length);
            break;
        case sIOtype_ERROR:
            Serial.printf("[IOc] get error: %u\n", length);
            break;
        case sIOtype_BINARY_EVENT:
            Serial.printf("[IOc] get binary: %u\n", length);
            break;
        case sIOtype_BINARY_ACK:
            Serial.printf("[IOc] get binary ack: %u\n", length);
            break;
    }
}

/////Receive Data/////////////////////////////////////
void httpGETRequest(struct dataSet *ptr) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverName);

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

  Serial.println(payload);
  
}////httpGETRequest_End

////Send Data//////////////////////////////////////
void httpPOSTRequest(struct dataSet *ptr) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverName);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String httpRequestData = "temp=" + ptr->Temp +
                           "&humi=" + ptr->Humi +
                           "&batt=" + ptr->Batt +
                           "&net=" + ptr->Net;

  int httpResponseCode = http.POST(httpRequestData);
  Serial.print(httpRequestData);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  http.end();           // Free resources
}////httpPOSTRequest_End

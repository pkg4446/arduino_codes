#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient
#include <HTTPClient.h>
#define SERIAL_MAX  64
HardwareSerial rootDvice(2);

const char* mqttServer    = "smarthive.kr";
const int   mqttPort      = 1883;
const char* mqttUser      = "hive";
const char* mqttPassword  = "hive";
const char* topic_pub     = "SHS";
char        deviceID[18];
char        sendID[21]    = "ID=";

WiFiClient mqtt_client;
PubSubClient mqttClient(mqtt_client);

struct dataSet {
  String TYPE;
  String MODULE;
  String VALUE1;
  String VALUE2;
};

//// ------------ MQTT Callback ------------
void callback(char* topic, byte* payload, unsigned int length) {
  char mqtt_buf[SERIAL_MAX] = "";
  for (int i = 0; i < length; i++) {
    mqtt_buf[i] = payload[i];
  }
  Serial.print("Message arrived: ");
  rootDvice.print(mqtt_buf);
  Serial.println(mqtt_buf);
}

//// ----------- Command  -----------
char    command_Buf[SERIAL_MAX];
int16_t command_Num;

void command_Service() {
  struct dataSet dataSend;
  dataSend.MODULE = strtok(command_Buf, "=");
  dataSend.TYPE   = strtok(NULL, "=");
  dataSend.VALUE1 = strtok(NULL, "=");
  dataSend.VALUE2 = strtok(NULL, ";");
  if (dataSend.TYPE != "P") {
    httpPOSTRequest(&dataSend);
  }
}//Command_service() END

void command_Process() {
  char ch;
  ch = rootDvice.read();
  switch (ch) {
    case ';':
      command_Buf[command_Num] = ';';
      command_Service();
      command_Num = 0;
      break;
    default:
      command_Buf[command_Num++] = ch;
      command_Num %= SERIAL_MAX;
      break;
  }
}

void setup() {
  Serial.begin(115200);
  rootDvice.begin(115200, SERIAL_8N1, 21, 22);

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  WiFiManager wm;
  wm.setConfigPortalTimeout(120);
  bool res;
  res = wm.autoConnect("SmartHive");
  if (!res) {
    wm.resetSettings();
    return;
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  for (int i = 0; i < 17; i++) {
    sendID[i + 3] = WiFi.macAddress()[i];
    deviceID[i]   = sendID[i + 3];
  }
  
  char* topic_sub = deviceID;
  char* sub_ID    = sendID;
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect(deviceID, mqttUser, mqttPassword )) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }

  mqttClient.subscribe(topic_sub);
  mqttClient.publish(topic_pub, sub_ID);

  Serial.print("subscribe: ");
  Serial.print(topic_sub);
  Serial.println(" - MQTT Connected");
}//End Of Setup()

void reconnect(){
  char* topic_sub = deviceID;
  char* sub_ID    = sendID;
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect(deviceID, mqttUser, mqttPassword )) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
}

void loop() {
  if (mqttClient.connected()){mqttClient.loop();}
  else{reconnect();}
  if (rootDvice.available()) command_Process();//post
}

void httpPOSTRequest(struct dataSet *ptr) {
  String serverUrl = "http://dev.smarthive.kr/reg/hive/";   //API adress
  HTTPClient http;
  WiFiClient http_client;
  http.begin(http_client, serverUrl);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String httpRequestData = (String)"FARM="  + deviceID    +
                           "&MODULE="       + ptr->MODULE +
                           "&TYPE="         + ptr->TYPE   +
                           "&VALUE1="       + ptr->VALUE1 +
                           "&VALUE2="       + ptr->VALUE2;

  int httpResponseCode = http.POST(httpRequestData);
  Serial.print(httpRequestData);
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  http.end();           // Free resources
}////httpPOSTRequest_End

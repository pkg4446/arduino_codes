#include <WiFi.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient

const char* ssid     = "Daesung2G";
const char* password = "smarthive123";

const char* mqttServer    = "mqtt.kro.kr";
const int   mqttPort      = 1883;
const char* mqttUser      = "test";
const char* mqttPassword  = "test";
const char* topic_pub     = "master";

char deviceID[18];
char sendID[20]   = "ID=";
  
WiFiClient espClient;
PubSubClient mqttClient(espClient);

//// ------------ MQTT Callback ------------
void callback(char* topic, byte* payload, unsigned int length) {
  char mqtt_buf[64] = "";
  for (int i = 0; i < length; i++) {
    mqtt_buf[i] = payload[i];
  }
  Serial.print("Message arrived: ");
  Serial.println(mqtt_buf);
}

void setup() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  WiFi.begin(ssid, password);

  Serial.begin(115200);
  
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
}

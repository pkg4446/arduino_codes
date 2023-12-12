#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);
IPAddress myDns(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

EthernetServer TCPserver(502);
EthernetClient client;

char recieve[140];
uint8_t char_index = 0;

void TCPServer() {
  // Wait for a TCP client from Arduino #1:
  EthernetClient newClient = TCPserver.accept();
  if(newClient && !client) client = newClient;
  
  if (client) {
    unsigned long test1 = micros();
    // Read the command from the TCP client:
    recieve[char_index++] = client.read();
    if(char_index >= 140) char_index = 0;
    Ethernet.maintain();    
    Serial.println(micros() - test1);
  }
  if (client) {
    unsigned long test1 = micros();
    client.println("Hello, client!");
    Ethernet.maintain();
    Serial.println(micros() - test1);
  }


}

void setup() {
  Serial.begin(115200);
  Serial.println("Ethernet TCP Example");
  
  Ethernet.init(53);

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    }
    // try to configure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, gateway, subnet);
  } 

  Serial.println("Ethernet connected");

  // start the server
  TCPserver.begin();

  Serial.print("System online");
  Serial.print(" : DHCP assigned IP ");
  Serial.println(Ethernet.localIP());
}



void loop()
{
  TCPServer();
}




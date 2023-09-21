#include  <WiFi.h>
#include  <HTTPClient.h>
#include  "Wire.h"
#define   TCAADDR 0x70

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include  "Adafruit_SHT31.h"
Adafruit_SHT31 sht31  = Adafruit_SHT31();

uint8_t   LED = 17;

char      deviceID[18];
int16_t   Temperature[8]  = {14040,};
int16_t   Humidity[8]     = {14040,};

unsigned long timer_SHT31 = 0;
unsigned long timer_SEND  = 0;
unsigned long timer_WIFI  = 0;

uint16_t log_index   = 0; 
String time_stmp  = "Didn't get time from server.";

void tca_select(uint8_t index) {
  if (index > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << index);
  Wire.endTransmission();
}

// standard Arduino setup()
void setup()
{
  Serial.begin(115200);
  Wire.begin();
  pinMode(LED, OUTPUT);

  Serial.begin(115200);
}

void loop()
{
  get_sensor(millis());
  send_sensor(millis());
}

void send_sensor(unsigned long millisec) {
  if ((millisec - timer_SEND) > 1 * 1000) {
    timer_SEND = millisec;

    for (uint8_t channel = 0; channel < 8; channel++) {
      Serial.print("TCA Port #"); Serial.print(channel);
      Serial.print(", T: ");
      Serial.print(Temperature[channel]);
      Serial.print("°C ,H: ");
      Serial.print(Humidity[channel]);
      Serial.println("%");
    }
  }
}

void get_sensor(unsigned long millisec) {
  if ((millisec - timer_SHT31) > 300) {
    timer_SHT31 = millisec;

    for (uint8_t channel = 0; channel < 8; channel++) {
      tca_select(channel);
      Wire.beginTransmission(68); //0x44
      if (!Wire.endTransmission() && sht31.begin(0x44)) {
        Temperature[channel]  = sht31.readTemperature() * 100;
        Humidity[channel]     = sht31.readHumidity() * 100;
      } else {
        Temperature[channel]  = 14040;
        Humidity[channel]     = 14040;
      }
    }//for
  }//if
}

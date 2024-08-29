#include  <Arduino.h>
#include  <Wire.h>

#define SDA2 33
#define SCL2 32
TwoWire Wire2 = TwoWire(1);

//#define SENSOR_SHT40
#ifdef SENSOR_SHT40
  #include "Adafruit_SHT4x.h"
  Adafruit_SHT4x sht40 = Adafruit_SHT4x();
#else
  #include  <Adafruit_SHT31.h>
  Adafruit_SHT31 sht31_1  = Adafruit_SHT31();
  Adafruit_SHT31 sht31_2  = Adafruit_SHT31(&Wire2);
#endif

const boolean pin_on  = true;
const boolean pin_off = false;

uint8_t sht_port      = 0;

const uint8_t tempGap = 1;
//// ----------- Variable -----------
//// ----------- Sensor -----------
int16_t temperature = 14040;
int16_t humidity    = 14040;
////for millis() func//
unsigned long timer_SHT     = 0UL;
//// ----------- Command  -----------
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  Wire2.begin(SDA2,SCL2,400000);
}

void loop() {
  unsigned long now = millis();
  get_sensor(now);
  serial_monit(now);
}

void get_sensor(unsigned long millisec) {
  if ((millisec - timer_SHT) > 500) {
    timer_SHT = millisec;
    bool sensor_conn = false;
    #ifdef SENSOR_SHT40
      if (sht40.begin()) {
        sht_port = 1;
        sensors_event_t humi, temp;
        sht40.getEvent(&humi, &temp);
        temperature  = temp.temperature * 100;
        humidity     = humi.relative_humidity * 100;
        sensor_conn  = true;
      }else if (sht40.begin($Wire2)) {
        sht_port = 2;
        sensors_event_t humi, temp;
        sht40.getEvent(&humi, &temp);
        temperature  = temp.temperature * 100;
        humidity     = humi.relative_humidity * 100;
        sensor_conn  = true;
      }
    #else
      if (sht31_1.begin(0x44)) {
        sensor_conn = true;
        sht_port = 1;
        float temp_temperature = sht31_1.readTemperature();
        if(isnan(temp_temperature)){
          temperature  = 15060;
          humidity     = 15060;
        }else{
          temperature = temp_temperature * 100;
          humidity    = sht31_1.readHumidity() * 100;
        }
        sht31_1.reset();
      }else if (sht31_2.begin(0x44)) {
        sensor_conn = true;
        sht_port = 2;
        float temp_temperature = sht31_2.readTemperature();
        if(isnan(temp_temperature)){
          temperature  = 15060;
          humidity     = 15060;
        }else{
          temperature = temp_temperature * 100;
          humidity    = sht31_2.readHumidity() * 100;
        }
        sht31_2.reset();
      }
    #endif
    if(!sensor_conn){
      temperature  = 14040;
      humidity     = 14040;
    }
  }//if
}

unsigned long timer_serial_monit = 0;
void serial_monit(unsigned long millisec){
  if (run_log && ((millisec - timer_serial_monit) > 1000)) {
    timer_serial_monit = millisec;
    Serial.println(ERR_Message);
    Serial.print("TCA Port");
    Serial.print(sht_port);
    Serial.print(", T: ");
    Serial.print(temperature);
    Serial.print("°C ,H: ");
    Serial.print(humidity);
    Serial.println("%");
    mesh.update();
    Serial.print("USE_heater = ");
    Serial.println(use_stable_h);
    Serial.print(", heater");
    Serial.print(run_heater);
    Serial.println(";");
    mesh.update();
  }
}
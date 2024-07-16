#include <Wire.h>

#define UPDATE_INTERVAL 200L
#define TCA9548A_COUNT  7
#define SENSOR_COUNT    50
#define MOVING_AVERAGE  10

#define SDA_PIN 21
#define SCL_PIN 22

/*********************************************************/
byte tcaAddresses[TCA9548A_COUNT] = {0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76}; // 멀티플렉서 주소
const uint8_t TMP112_ADDRESS = 0x48; // TMP112 온도 센서 주소
/*********************************************************/
int16_t temperatures[SENSOR_COUNT][MOVING_AVERAGE];
/*********************************************************/
unsigned long pre_update_time   = 0UL;
unsigned long pre_display_time  = 0UL;
uint8_t  index_sensor   = 0;
uint8_t  index_average  = 0;
/*********************************************************/
void tcaSelect(uint8_t tcaAddress, uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(tcaAddress);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void tcaDisable(uint8_t tcaAddress) {
  Wire.beginTransmission(tcaAddress);
  Wire.write(0);
  Wire.endTransmission();
}

int16_t readTemp() {
  Wire.requestFrom(TMP112_ADDRESS, 2);
  if (Wire.available() == 2) {
    int16_t raw = (Wire.read() << 8) | Wire.read();
    raw >>= 4; // TMP112는 12비트 해상도, 따라서 4비트 쉬프트
    return int16_t(raw * 6.25); // TMP112 온도 변환
  } else {
    return 9999; // 읽기 실패
  }
}

void sensor_mapping(unsigned long millisec){
  if(millisec > pre_update_time + UPDATE_INTERVAL){//runtime : 32 millisec
    uint8_t  index_sensor = 0;
    // 모든 센서 값 읽기
    for (uint8_t index = 0; index < TCA9548A_COUNT; index++) {
      uint8_t channelLimit = (index == TCA9548A_COUNT - 1) ? 2 : 8; // 마지막 멀티플렉서는 2개의 채널만 사용
      for (uint8_t ch = 0; ch < channelLimit; ch++) {
        tcaSelect(tcaAddresses[index], ch);
        temperatures[index_sensor++][index_average] = readTemp();
      }
      tcaDisable(tcaAddresses[index]); // 각 멀티플렉서 비활성화
    }
    if(++index_average>=MOVING_AVERAGE) index_average = 0;
  }
}

void sensor_value_init(){
  for (uint8_t index = 0; index < SENSOR_COUNT; index++) {
    for (uint8_t average = 1; average < MOVING_AVERAGE; average++) {
      temperatures[index][average] = temperatures[index][0];
    }
  }
}
/*********************************************************/

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000); // I2C 속도를 100kHz로 설정
  Serial.println("\nSystem Online");
  sensor_mapping(UPDATE_INTERVAL+1);
  sensor_value_init();
}

void loop() {
  unsigned long millisec = millis();
  sensor_mapping(millisec);
  sensor_display(millisec);
}



void sensor_display(unsigned long millisec){
  if(millisec > pre_display_time + 3000){
    pre_display_time = millisec;
    
    Serial.print("\nTemperature readings:");
    Serial.println(index_average);
    for (uint8_t row = 0; row < 5; row++){
      for (uint8_t col = 0; col < 10; col++){
        uint8_t index = col*5 + row;
        Serial.print(index);
        Serial.print(":");
        int32_t sensor_temperature = 0;
        for (uint8_t average = 0; average < MOVING_AVERAGE; average++) {
          sensor_temperature += temperatures[index][average];
        }
        sensor_temperature /= MOVING_AVERAGE;
        if (sensor_temperature == 9999) {
          Serial.print("Error\t");
        } else {
          Serial.print(sensor_temperature);
          Serial.print(" °C\t");
        }
      }
      Serial.println();
    }
  }
}
#define TOTAL_LED       3
#define TOTAL_CONTROL   3
#define OUTPUT          5
#define EEPROM_SIZE     24
#define COMMAND_LENGTH  32
#define SECONDE         1000L

#define LED_INTERNET  0
#define HEATER        0
#define VALVE_A       1
#define VALVE_B       2

#define SONA      2
#define SONA_DATA 4

////--------------------- Pin out ---------------------////
uint8_t LED[TOTAL_LED] = {4,5,33};
uint8_t RELAY[OUTPUT]  = {23,25,26,27,32};
////--------------------- Pin out ---------------------////
////--------------------- setup() ---------------------////
void setup()
{
  Serial.begin(115200);
  for (uint8_t index = 0; index < TOTAL_LED; index++){
    pinMode(LED[index], OUTPUT);
    // digitalWrite(LED[index], false);
  }
  for (uint8_t index = 0; index < OUTPUT; index++){
    pinMode(RELAY[index], OUTPUT);
    // digitalWrite(RELAY[index], false);
  }
}
////--------------------- setup() ---------------------////
////--------------------- loop() ----------------------////
void loop()
{
  for (uint8_t index = 0; index < OUTPUT; index++){
    digitalWrite(RELAY[index], true);
    Serial.print("ON: ");
    Serial.println(RELAY[index]);
    delay(1000);  // 1초 동안 ON

    digitalWrite(RELAY[index], false);
    Serial.print("OFF: ");
    Serial.println(RELAY[index]);
    delay(1000);  // 1초 동안 OFF
  }
}
////--------------------- loop() ----------------------////`

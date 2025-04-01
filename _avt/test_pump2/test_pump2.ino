// #define TOTAL_LED       3
// #define OUTPUT          5

// uint8_t LED[TOTAL_LED] = {4,5,33};
uint8_t RELAY[5]  = {23,25,26,27,32};

void setup() {
  Serial.begin(115200);

  // 테스트할 핀들 설정
  pinMode(23, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);
}

void loop() {
  // 핀 번호 배열
  uint8_t pins[] = {23,25,26,27,32};

  for (int i = 0; i < 4; i++) {
    digitalWrite(pins[i], HIGH);
    Serial.print("ON: ");
    Serial.println(pins[i]);
    delay(1000);  // 1초 동안 ON

    digitalWrite(pins[i], LOW);
    Serial.print("OFF: ");
    Serial.println(pins[i]);
    delay(1000);  // 1초 동안 OFF
  }
}
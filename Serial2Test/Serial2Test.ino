#include <HardwareSerial.h>
#define SERIAL_MAX  256

const int8_t Relay[6] = {5, 17, 16, 4, 2, 15};

#define UPDATE_INTERVAL 1000L
unsigned long prevUpdateTime = 0L;

HardwareSerial nxSerial(2);

////AT Code for chage the Something//////////////////////////
char Serial_buf[SERIAL_MAX];
int16_t Serial_num;

void Serial_process() {
  char ch;
  ch = nxSerial.read();
  switch ( ch ) {
    case '\n':
      Serial_buf[Serial_num] = NULL;
      Serial_service();
      Serial_num = 0;
      break;
    case '!':
      Serial_buf[Serial_num] = NULL;
      Serial_service();
      Serial_num = 0;
      break;
    default :
      Serial_buf[ Serial_num ++ ] = ch;
      Serial_num %= SERIAL_MAX;
      break;
  }
}

void Serial_service() {
  Serial.println(Serial_buf);
  String str1 = strtok(Serial_buf, "=");
  String str2 = strtok(NULL, " ");

  if (str1 == "AT+ON") {
    digitalWrite(Relay[1], HIGH);
    Serial.print(str1);
    Serial.println("OK");
  } else if (str1 == "AT+OFF") {
    digitalWrite(Relay[1], LOW);
    Serial.print(str1);
    Serial.println("OK");
  } else if (str1 == "AT+WIFI") {
    Serial.print(str1);
    Serial.println("OK");
  } else if (str1 == "AT+T") {
    Serial.print(str1);
    Serial.println("OK");
    Serial.print(str2);
    Serial.println("OK");
  }
}

void setup() {
  Serial.begin(115200);
  nxSerial.begin(115200, SERIAL_8N1, 18, 19);
  pinMode(Relay[0], OUTPUT);
  pinMode(Relay[1], OUTPUT);
  pinMode(Relay[2], OUTPUT);
  pinMode(Relay[3], OUTPUT);
  pinMode(Relay[4], OUTPUT);
  pinMode(Relay[5], OUTPUT);

  prevUpdateTime = millis();
}//End Of Setup()

uint32_t number = 1;

void loop() {
  if (nxSerial.available()) {
    Serial_process();
  }

  unsigned long currentTime = millis();
  if (currentTime > prevUpdateTime + UPDATE_INTERVAL) {
    prevUpdateTime = currentTime;
    sendTime(true, number);
    sendTime(false, number);
    number++;
  }
}

void sendTime(boolean onoff, uint32_t valueN) {
  String cmd;
  if (onoff) {
    cmd = "timeOn.val=";
  } else {
    cmd = "timeOff.val=";
  }
  char buf[10] = {0};
  sprintf(buf, "%d", number);
  cmd += buf;
  send2Nextion(cmd);
}
void send2Nextion(String cmd) {
  nxSerial.print(cmd);
  nxSerial.write(0xFF);
  nxSerial.write(0xFF);
  nxSerial.write(0xFF);
}

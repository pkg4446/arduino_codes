#define SECONDE         1000L

// 핀 설정
const int inputPin = A0;   // 센서 입력
const int outputPin = 9;   // PWM 출력

// PID 변수
double setpoint = 500;    // 목표값
double temp_air, output;
double Kp = 0, Ki = 0, Kd = 0;

// 자동 튜닝용 변수
bool isTuning = false;
double Ku, Pu;
unsigned long lastTime, startTime;
double lastInput;
double ITerm = 0;
int oscillationCount = 0;

void setup() {
  Serial.begin(115200);
  pinMode(outputPin, OUTPUT);
  Serial.println("명령어: 't'를 입력하면 자동 튜닝을 시작합니다.");
}

void loop() {
  temp_air = analogRead(inputPin);

  if (isTuning) {
    runAutoTune();
  } else {
    runPID();
  }

  // 데이터 모니터링 (시리얼 플로터용)
  Serial.print(setpoint); Serial.print(",");
  Serial.println(temp_air);
  delay(10);
}

// --- PID 제어 로직 ---
void runPID() {
  unsigned long now = millis();
  double timeChange = (double)(now - lastTime) / SECONDE; // 초 단위

  if (timeChange <= 0) return;

  double error = setpoint - temp_air;
  ITerm += (Ki * error * timeChange);
  // Anti-windup (출력 제한)
  ITerm = constraint(ITerm);

  double dInput = (temp_air - lastInput) / timeChange;

  output = Kp * error + ITerm - Kd * dInput;
  output = constraint(output);

  analogWrite(outputPin, output);
  //여기 출력온도로에서 끝.

  lastInput = temp_air;
  lastTime = now;
}

void runAutoTune() {
  // 릴레이 제어 (On/Off 제어로 진동 유도)
  if (temp_air < setpoint) output = 50;
  else output = 0;
  analogWrite(outputPin, output);

  // 여기서 진동 주기와 진폭을 측정하여 Ku, Pu를 계산하는 로직이 들어갑니다.
  // 실제 구현 시에는 특정 횟수 이상의 진동(Peak-to-Peak)을 감지해야 합니다.
  
  // 예시: 10초 후 간이 계산 (실제로는 정밀한 피크 검출 로직 필요)
  if (millis() - startTime > 10000) { 
    // 임의의 측정값 가정 (사용자 환경에 따라 로직 보강 필요)
    Ku = 10.0; // 임계 이득 예시
    Pu = 1.2;  // 임계 주기 예시 (초)

    // Ziegler-Nichols 공식 적용
    Kp = 0.6 * Ku;
    Ki = 2.0 * Kp / Pu;
    Kd = Kp * Pu / 8.0;

    isTuning = false;
    Serial.print("튜닝 완료! Kp:"); Serial.print(Kp);
    Serial.print(" Ki:"); Serial.print(Ki);
    Serial.print(" Kd:"); Serial.println(Kd);
  }
}

double constraint(double val) {
  double minVal = 0;
  double maxVal = 50;
  if (val < minVal) return minVal;
  if (val > maxVal) return maxVal;
  return val;
}
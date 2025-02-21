void setup() {
  Serial.begin(115200);
  Serial.println("boot...");
}

void loop() {
  if (Serial.available()) {
    Serial.print(char(Serial.read()));
  }
}

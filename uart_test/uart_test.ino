HardwareSerial nxSerial(2);

void setup() {
  Serial.begin(115200);
  nxSerial.begin(115200, SERIAL_8N1, 18, 19);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (nxSerial.available()){
    Serial.write(nxSerial.read());
  }
}

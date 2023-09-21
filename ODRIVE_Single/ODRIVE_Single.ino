#include <ODriveArduino.h>
#define BAUDRATE 115200
HardwareSerial odrive_serial(2);

#define ESP32_UART2_PIN_RX 16  //wired to odrive pin 2
#define ESP32_UART2_PIN_TX 17  //wired to odrive pin 1

// Printing with stream operator
template<class T> inline Print& operator <<(Print &obj,     T arg) {
  obj.print(arg);
  return obj;
}
template<>        inline Print& operator <<(Print &obj, float arg) {
  obj.print(arg, 4);
  return obj;
}

float vel_limit   = 20.0f;
float current_lim = 11.0f;

const uint8_t limit_up = 2;

int zero_pos  = 0; //use limit
int down_pos  = 0;

// ODrive object
ODriveArduino odrive(odrive_serial);

void setup() {
  // put your setup code here, to run once:
  // Serial to PC
  pinMode(limit_up, INPUT_PULLUP);

  Serial.begin(BAUDRATE);
  odrive_serial.begin(BAUDRATE, SERIAL_8N1, ESP32_UART2_PIN_TX, ESP32_UART2_PIN_RX);
  while (!Serial) ; // wait for Arduino Serial Monitor to open
  Serial.println("ODriveArduino");
  while (!odrive_serial) ; // wait for Arduino Serial Monitor to open
  Serial.println("Setting parameters...");


  // In this example we set the same parameters to both motors.
  // You can of course set them different if you want.
  // See the documentation or play around in odrivetool to see the available parameters
  for (int axis = 0; axis < 2; ++axis) {
    odrive_serial << "w axis" << axis << ".controller.config.vel_limit " << vel_limit << '\n';
    odrive_serial << "w axis" << axis << ".motor.config.current_lim " << current_lim << '\n';
    // This ends up writing something like "w axis0.motor.config.current_lim 10.0\n"
  }
  Serial.println("Calibrate respective motor...");
  int8_t system_voltage = 0;
  while (!system_voltage){
    odrive_serial << "r vbus_voltage\n";
    system_voltage = (int)odrive.readFloat();    
    Serial.print("System v:");
    Serial.println(system_voltage);
  }
  calibration();
  zero_set();
  Serial.println("System On Line!");

  Serial.println("Send the character 'b' to read bus voltage");
  Serial.println("Send the character 'p' to read motor positions in a 10s loop");
  Serial.println("Send the character 'c'(-) or 'C'(+) to raise and lower the current_limit (+- 1)");
  Serial.println("Send the character 'v'(-) or 'V'(+) to raise and lower the velocity_limit (+- 5)");
  Serial.println("Send the character 'x'(-) or 'X'(+) to switch back and forth to position (+- 10)");


}

void calibration() {
  char moter   = '0';
  int motornum = moter - '0';
  int requested_state;
  requested_state = AXIS_STATE_MOTOR_CALIBRATION;
  Serial << "Axis" << moter << ": Requesting state " << requested_state << '\n';
  if (!odrive.run_state(motornum, requested_state, true)) return;
  requested_state = AXIS_STATE_ENCODER_OFFSET_CALIBRATION;
  Serial << "Axis" << moter << ": Requesting state " << requested_state << '\n';
  if (!odrive.run_state(motornum, requested_state, true, 25.0f)) return;
  requested_state = AXIS_STATE_CLOSED_LOOP_CONTROL;
  Serial << "Axis" << moter << ": Requesting state " << requested_state << '\n';
  if (!odrive.run_state(motornum, requested_state, false /*don't wait*/)) return;
}

void zero_set() {
  boolean true_false = !digitalRead(limit_up);
  zero_pos = (int)odrive.GetPosition(0);
  while (true_false) {
    true_false = !digitalRead(limit_up);
    int moter_pos = (int)odrive.GetPosition(0);
    if ((zero_pos <= moter_pos + 1)&&(zero_pos >= moter_pos - 1)){
      zero_pos --;
      odrive.SetPosition(0, zero_pos);
    }    
    Serial.println((String)"Zero Point tf: " + true_false + ":" + zero_pos + ":" + moter_pos);
    delay(1);
  }
  down_pos = zero_pos + 75;  
  Serial.println((String)"Zero Point: " + zero_pos);
  Serial.println((String)"Down Point: " + down_pos);
  odrive.SetPosition(0, zero_pos + 5);
}

void loop() {
  // put your main code here, to run repeatedly:

  //Serial.println(); //pull is true

  if (Serial.available()) {
    char c = Serial.read();

    // Run calibration sequence

    // Change Velocity incrementally
    if (c == 'z') {
      zero_set();
    }

    if (c == 'V' || c == 'v') {
      float inc = 5.0f;
      if (c == 'v') inc *= -1;
      vel_limit += inc;
      Serial.println((String)"Velocity Limit: " + vel_limit);
      for (int axis = 0; axis < 2; ++axis) {
        odrive_serial << "w axis" << axis << ".controller.config.vel_limit " << vel_limit << '\n';        
      }
    }
    // Change Current incrementally
    if (c == 'C' || c == 'c') {
      float inc = 1.0f;
      if (c == 'c') inc *= -1;
      current_lim += inc;
      Serial.println((String)"Current Limit: " + current_lim);
      for (int axis = 0; axis < 2; ++axis) {
        odrive_serial << "w axis" << axis << ".motor.config.current_lim " << current_lim << '\n';
      }
    }

    // Change Positions: Flips position back and forth between + and - value
    if (c == 'X' || c == 'x') {
      if (c == 'x') {
        odrive.SetPosition(0, down_pos);
      }
      else {        
        odrive.SetPosition(0, zero_pos + 5);
      }
    }

    // Read bus voltage
    if (c == 'b') {
      odrive_serial << "r vbus_voltage\n";
      Serial << "Vbus voltage: " << odrive.readFloat() << '\n';
    }

    // print motor positions in a 10s loop
    if (c == 'p') {
      static const unsigned long duration = 10000;
      unsigned long start = millis();
      while (millis() - start < duration) {
        for (int motor = 0; motor < 2; ++motor) {
          Serial << odrive.GetPosition(motor) << '\t';
        }
        Serial << '\n';
      }
    }
  }
}

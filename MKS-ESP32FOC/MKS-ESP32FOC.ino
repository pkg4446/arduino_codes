/// MKS ESP32 FOC Open loop speed control example; Test Library：SimpleFOC 2.1.1 ; Tested hardware：MKS ESP32 FOC V1.0
/// Enter "T+number" in the serial port to set the speed of the two motor_as.For example, to set the motor_a to rotate at a speed of 10rad/s, input "T10"
/// When the motor_a is powered on, it will rotate at 5rad/s by default
/// When using your own motor_a, do remember to modify the default number of pole pairs, the value in BLDCMotor(7).
/// The default power supply voltage of the program is 12V.
/// Please remember to modify the voltage_power_supply , voltage_limit variable values when using other voltages for power supply

#include <SimpleFOC.h>
#include <EEPROM.h>
#define EEPROM_SIZE     8
#define COMMAND_LENGTH  32

char    command_buf[COMMAND_LENGTH];
int8_t  command_num;

BLDCMotor motor_a = BLDCMotor(7,0.105,930);                               //According to the selected motor_a, modify the number of pole pairs here, the value in BLDCMotor()
BLDCDriver3PWM driver_a = BLDCDriver3PWM(32,33,25,22);
  
/// BLDC motor_a & driver_a instance
BLDCMotor motor_b = BLDCMotor(7,0.105,930);                              //Also modify the value in BLDCMotor() here
BLDCDriver3PWM driver_b  = BLDCDriver3PWM(26,27,14,12);

/// Target Variable
float method_a  = 0;
float method_b  = 0;
bool  target_mt = false;

enum MOTER_CONFIG {
  SUPPLY_A,
  SUPPLY_B,
  LIMIT_A,
  LIMIT_B,
  SPEED_A,
  SPEED_B,
  TYPEC_A,
  TYPEC_B
};

/// Serial Command Setting
void command_process(char ch) {
  if(ch=='\n'){
    command_buf[command_num] = 0x00;
    command_num = 0;
    command_service();
    memset(command_buf, 0x00, COMMAND_LENGTH);
  }else if(ch!='\r'){
    command_buf[command_num++] = ch;
    command_num %= COMMAND_LENGTH;
  }
}

void command_service(){
  String cmd_text     = "";
  String temp_text    = "";
  bool   eep_change   = false;
  uint8_t check_index = 0;
  
  for(uint8_t index_check=0; index_check<COMMAND_LENGTH; index_check++){
    if(command_buf[index_check] == 0x20 || command_buf[index_check] == 0x00){
      check_index = index_check+1;
      break;
    }
    cmd_text += command_buf[index_check];
  }
  for(uint8_t index_check=check_index; index_check<COMMAND_LENGTH; index_check++){
    if(command_buf[index_check] == 0x20 || command_buf[index_check] == 0x00){
      check_index = index_check+1;
      break;
    }
    temp_text += command_buf[index_check];
  }
  /**********/
  if(cmd_text=="run"){
    if(target_mt) motor_a.enable();
    else  motor_b.enable();
  }else if(cmd_text=="stop"){
    if(target_mt) motor_a.disable();
    else  motor_b.disable();
  }else if(cmd_text=="forward"){
    if(target_mt) method_a = float(temp_text.toInt())/100;
    else  method_b = float(temp_text.toInt())/100;
  }else if(cmd_text=="backward"){
    if(target_mt) method_a = -float(temp_text.toInt())/100;
    else  method_b = -float(temp_text.toInt())/100;
  }else if(cmd_text=="target"){
    if(temp_text == "a") target_mt=true;
    else target_mt=false;
  }else if(cmd_text=="type"){
    if(temp_text == "pos"){
      if(target_mt){
        motor_a.controller = MotionControlType::angle_openloop;
        EEPROM.write(TYPEC_A, 0);
      }else{
        motor_b.controller = MotionControlType::angle_openloop;
        EEPROM.write(TYPEC_B, 0);
      }
    }
    else{
      if(target_mt){
        motor_a.controller = MotionControlType::velocity_openloop;
        EEPROM.write(TYPEC_A, 1);
      }else{
        motor_b.controller = MotionControlType::velocity_openloop;
        EEPROM.write(TYPEC_B, 1);
      }
    }
    EEPROM.commit();
  }else if(cmd_text=="limit"){
    uint8_t limit_value = cmd_text.toInt();
    Serial.print("limit ");
    Serial.println(limit_value);
    if(target_mt){
      driver_a.voltage_limit = limit_value;
      EEPROM.write(LIMIT_A, limit_value);
    }else{
      driver_b.voltage_limit = limit_value;
      EEPROM.write(LIMIT_B, limit_value);
    }
    EEPROM.commit();
  }else if(cmd_text=="power"){
    uint8_t power_value = cmd_text.toInt();
    Serial.print("power ");
    Serial.println(power_value);
    if(target_mt){
      driver_a.voltage_power_supply = power_value;
      EEPROM.write(SUPPLY_A, power_value);
    }else{
      driver_b.voltage_power_supply = power_value;
      EEPROM.write(SUPPLY_B, power_value);
    }
    EEPROM.commit();
  }else if(cmd_text=="reboot"){
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);

  if (!EEPROM.begin(EEPROM_SIZE)){
    Serial.println("Failed to initialise eeprom");
    Serial.println("Restarting...");
    delay(100);
    ESP.restart();
  }

  for(uint8_t index=0; index<EEPROM_SIZE; index++){
    Serial.print(index);
    Serial.print("eeprom ");
    Serial.println(EEPROM.read(index));
  }

  driver_a.voltage_power_supply = EEPROM.read(SUPPLY_A);                   //According to the supply voltage, modify the value of voltage_power_supply here
  driver_a.init();
  motor_a.linkDriver(&driver_a);
  motor_a.voltage_limit  = EEPROM.read(LIMIT_A);   // [V]                   //According to the supply voltage, modify the value of voltage_limit here
  motor_a.velocity_limit = EEPROM.read(SPEED_A); // [rad/s]
  
  driver_b.voltage_power_supply = EEPROM.read(SUPPLY_B);                  //Also modify the value of voltage_power_supply here
  driver_b.init();
  motor_b.linkDriver(&driver_b);
  motor_b.voltage_limit  = EEPROM.read(LIMIT_B);   // [V]                  //Also modify the value of voltage_limit here
  motor_b.velocity_limit = EEPROM.read(SPEED_B); // [rad/s]

  // Open Loop Control Mode Setting
  if(EEPROM.read(TYPEC_A)) motor_a.controller = MotionControlType::velocity_openloop;
  else motor_a.controller = MotionControlType::angle_openloop;
  if(EEPROM.read(TYPEC_B)) motor_b.controller = MotionControlType::velocity_openloop;
  else motor_b.controller = MotionControlType::angle_openloop;
  // Initialize the Hardware
  motor_a.init();
  motor_b.init();

  motor_a.disable();
  motor_b.disable();

  Serial.println("System ready!");
}

void loop() {
  if(Serial.available()) command_process(Serial.read());
  motor_a.move(method_a);
  motor_b.move(method_b);
}

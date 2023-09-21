#define LOAD 49 // PL=PL0=49 //out
#define DIN  48 // Q7=PL1=48 //in
#define CLK  46 // CP=PL3=46 //out
#define CE   16 // CE=PH1=16 //out
////pin set
const uint8_t p_relay[7]  = {22,23,24,25,26,27,28}; //PA0~6
const uint8_t p_step[4]   = {10,11,12,13};          //PB4~7
const uint8_t p_dir[4]    = {35,34,33,32};          //PC2~5
const uint8_t test_pin[4] = {49,48,46,16};          //PC2~5
volatile uint8_t *out;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  for (uint8_t index=0 ; index<7; index++) {
    Serial.print("pin ");
    Serial.print(p_relay[index]);
    uint8_t port = digitalPinToPort(p_relay[index]);    
    out = portOutputRegister(port);
    Serial.print(" = ");
    Serial.println(port);
  }
  for (uint8_t index=0 ; index<4; index++) {
    Serial.print("pin ");
    Serial.print(p_step[index]);
    uint8_t port = digitalPinToPort(p_step[index]);    
    out = portOutputRegister(port);
    Serial.print(" = ");
    Serial.println(port);
  }
  for (uint8_t index=0 ; index<4; index++) {
    Serial.print("pin ");
    Serial.print(p_dir[index]);
    uint8_t port = digitalPinToPort(p_dir[index]);    
    out = portOutputRegister(port);
    Serial.print(" = ");
    Serial.println(port);
  }
  for (uint8_t index=0 ; index<4; index++) {
    Serial.print("pin ");
    Serial.print(test_pin[index]);
    uint8_t port = digitalPinToPort(test_pin[index]);    
    out = portOutputRegister(port);
    Serial.print(" = ");
    Serial.println(port);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}

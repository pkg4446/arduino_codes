#define serial_max 128
#define stepper_q  6

uint8_t p_ena[stepper_q] = {5,6,7,8,9,10}; // 모터를 6개 써서
uint8_t p_dir[stepper_q] = {11,12,13,14,15,16}; // uint8_t 0~255
uint8_t p_pul[stepper_q] = {17,18,19,20,21,22};

unsigned long moter_update[stepper_q] = 0UL;      // 각 스탭모터의 pul이 true가 되었던 시점

bool stepper_ena[stepper_q]           = {false,}; // 6개의 모터 ena에 대해서 false
uint32_t stepper_pul[stepper_q]       = {1000,};  // 각 스탭모터의 pul이 true가 되는 간격

//req_add 1
unsigned int val1, val2, val3 // 입력할 변수값 스텝수, 회전속도 딜레이, 회전방향
unsigned int i ; // for문에 사용할 변수

void setup()
{
  Serial.begin(11500); // 11500 : 시리얼 통신 속도(초당 비트수)
  for(uint8_t index=0; index<stepper_q; index++){
    pinMode(p_ena[index],OUTPUT); // index 0부터 5까지 계속해서 output으로 
    pinMode(p_dir[index],OUTPUT);
    pinMode(p_pul[index],OUTPUT);
  }
}

/*************** Serial communication *******************/
void AT_commandHelp() {
  Serial.println("------------ AT command help ------------");
  Serial.println("AT+HELP=null;                AT command help");
}

char Serial_buf[serial_max] = {0x00,}; 
uint8_t Serial_num = 0;
void Serial_run(){
  char Serial_read = Serial.read();
  if(Serial_read == "\n"){ // 문자열에 개행문자가 있으면, 문자열의 시작지점으로 이동.
    Serial_num = 0; // 처음 0으로 정의 ..
  }else if(Serial_read == ";"){      // ;를 문자열의 끝으로 사용
    Serial_buf[Serial_num++] = 0x00; // NULL 과 같은 의미
    Serial_num = 0; // 처음 0으로 정의 ..
    Serial_service();
  }else{
    Serial_buf[Serial_num++] = Serial_read;
    Serial_num %= serial_max; // 128까지 했으면 다시 돌아오기
  }
}

void Serial_service() {
  String str1 = strtok(Serial_buf, "="); // strtok: 쉼표로 구분 된 동물 이름의 char배열이 있고 각 이름을char배열에서 분리하려고합니다. 
  //이 경우 strtok() 기능을 사용하여 동물 이름과 char 어레이를 구분할 수 있습니다. 
  String str2 = strtok(0x00, " ");
  command_Service(str1, str2); // =이랑 공백에 대해서 strtok를 나누는거 같은데 .. 잘 모르겠어요 ...
}


void command_Service(String command, String value) {
  if (command == "AT+HELP") {
    AT_commandHelp();
  }else if (command == "AT+TEST") {
  }
  //req_add 2
  stepper_pul[stepper_q]={500,} // 1000-> 500으로 속도 변경 .. ? 
  //req_add 3
  if(Serial.available()){   //시리얼 입력이 있을경우
    val1=Serial.parseInt(); //스텝수 입력
    val2=Serial.parseInt(); //회전속도를 결정하는 딜레이값 입력(값이 작을수록 회전속도가 빨라짐)
    val3=Serial.parseInt(); //회전방향을 결정하는 입력

    digitalWrite(dir, val3);      //회전방향 출력
    
    for(i=0; i<val1; i++){        //정해진 스텝수만큼 펄스입력
      digitalWrite(steps, HIGH);
      delayMicroseconds(val2);    //딜레이값
      digitalWrite(steps, LOW);
      delayMicroseconds(val2);
    }
}

/*************** Serial communication *******************/

void loop(){
  unsigned long time_now = millis(); //mcu가 부팅하고 나서 지난시간.
  if(Serial.available()) Serial_run(); // Serial_run()이 실행되면
  moter_run(time_now); // moter_run이 실행된다.
}

void moter_run(unsigned long time_now){
  for(uint8_t index=0; index<stepper_q; index++){ 
    if(stepper_ena[index] && time_now - moter_update[index] > stepper_pul[index] ){ 
      //stepper_ena 가 true인 경우, 현재 시간이 [index]번 스탭모터의 마지막 동작한 시간으로부터, stepper_pul[index] 밀리초 이상 지난경우
      moter_update[index] = time_now;   //[index]번 스탭모터의 마지막 동작한 시간을 갱신하고,
      digitalWrite(p_pul[index],true);  //[index]번 스탭모터를 1step 동작한다.
    }
  }
  
  for(uint8_t index=0; index<stepper_q; index++){ 
    digitalWrite(p_pul[index],false); // [index번 스텝모터를 1setp 동작한다 ..... ? ]
  }
}

/********************* 추가할것 ************************/
/********************* 1. 2023-11-21 *******************/
//코드에 주석을 달아볼것
/********************* 2. 2023-11-21 *******************/
//req_add 2 코드를 추가하여 stepper_pul(스탭모터 회전속도)를 변경해볼것
//req_add 1 에 변수를 추가하여, req_add 3에서 스탭모터가 특정 값 만큼만 움직이도록 해볼것


/****************************************************************
 
 Project : HAppy BONES 부스제어기II 
 Version : Folletto ReV02.01
 Date    : 2021-05-26
 Author  : jae jun Kim
 Company : (주)플레토
 Device  : Atmega128
 Clock   : 16Mhz
 Compiler: AVR Studio 4.19 + WINAVR
 
*****************************************************************/
  
#include <avr/io.h>
#include <stdio.h>
#include <avr/delay.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#define USART_BAUDRATE 9600
#define UBRR_VALUE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)


#define S     0x53
#define E     0x45
#define T     0x54
#define A     0x41
#define C     0x43
#define K     0x4B
#define EOT   0x04

#define UPE    2
#define DOR    3
#define FE     4

#define FRAMING_ERROR (1<<FE)

#define PARITY_ERROR (1<<UPE)

#define DATA_OVERRUN (1<<DOR)

#define DATA_REGISTER_EMPTY (1<<UDRE)

#define RX_COMPLETE (1<<RXC)

#define RX_BUFFER_SIZE0  9 // USART0 Receiver buffer

unsigned int rx_buffer0[RX_BUFFER_SIZE0];

unsigned int rx_wr_index0,rx_counter0=0 , CMD=0, SEN=0,NUM=0 , k=0;


unsigned char rx_buffer_overflow0=0; // This flag is set on USART0 Receiver buffer overflow



unsigned char Sensor1,Sensor2,Sensor3,Sensor4,Sensor5,Sensor6,Sensor7; // 센서 값 (검출시 HIGH , 비검출시 LOW)
unsigned char Sensor8,Sensor9,Sensor10,Sensor11,Sensor12,Sensor13,Sensor14; // 센서 값 (검출시 HIGH , 비검출시 LOW) 
unsigned char Sensor15,Sensor16,Sensor17,Sensor18,Sensor19,Sensor20,Sensor21;
unsigned char Sensor22,Sensor23,Sensor24; 

unsigned char RXD;				//통신초기화시 필요없는 값 저장 



ISR(USART1_RX_vect){ //rx interrupt Sevice routine

 

    char status,data=0;

    status=UCSR1A;

    data=UDR1;  

 
    if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0) //no error?

    {
       
        rx_buffer0[rx_wr_index0++]=data;

		if(rx_buffer0[0]!= 0x53) rx_wr_index0=0;
		  
        if (rx_wr_index0 == RX_BUFFER_SIZE0) rx_wr_index0=0;

        if (++rx_counter0 == RX_BUFFER_SIZE0) //rx_buffer full?

        {

        rx_counter0=0;

        rx_buffer_overflow0=1;  
	 
		        
        }

    }

}



void Delay_us(unsigned char time_us)		/* time delay for us */
{ register unsigned char i;

  for(i = 0; i < time_us; i++)			// 4 cycle +
    {
		asm volatile(" PUSH  R0 ");		// 2 cycle +
    	asm volatile(" POP   R0 ");		// 2 cycle +
    	asm volatile(" PUSH  R0 ");		// 2 cycle +
    	asm volatile(" POP   R0 ");		// 2 cycle +
    	asm volatile(" PUSH  R0 ");		// 2 cycle +
    	asm volatile(" POP   R0 ");		// 2 cycle = 16 cycle = 1 us for 16MHz
    }
}


void Delay_ms(unsigned int time_ms)		/* time delay for ms */
{ register unsigned int i;

  for(i = 0; i < time_ms; i++)
    { Delay_us(250);
      Delay_us(250);
      Delay_us(250);
      Delay_us(250);
    }
}



void init_PORT(void)
{

 DDRA = 0x00; 
 DDRB = 0xFC;
 DDRD = 0xFB;  // PD3(TXD)출력 (1) , PD2(RXD) 입력(0)
 DDRC = 0x00;
 DDRE = 0xFF;
 DDRF = 0x00;
 DDRG = 0xFF;
 
 PORTE |= 0xFC; // 펌프 OFF 상태 Active high
 PORTD &= 0x0F; //모터5,6 초기화(OFF 상태)
 PORTG &= 0xF4; // 모터1 초기화(OFF 상태)
 PORTG |= 0x04;
 PORTB &= 0x03; // 모터 2,3,4 초기화 (OFF 상태)

}

void adcInit(void)  //ADC 초기화 함수
{  
    ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)); //16Mhz/128 = 125Khz  
    ADMUX |= (1<<REFS0);       //AVCC(5V)  
    ADCSRA |= (1<<ADEN);      //ADC 인에이블  
}  
   
    
uint16_t readAdc(uint8_t channel)  //값을 읽어오는 함수
{  
        
    ADMUX &= 0xE0;  
    ADMUX |= channel;  //ADC0=0x00, ADC1=0x01,ADC2=0x02, ADC3=0x03
        
    ADCSRA |= (1<<ADSC);      //변환 시작  
    while(ADCSRA&(1<<ADSC));//변환 완료되기를 기다림.  
        
    return ADCW;  //ADC값 반환  
}  



void Sensor_check(void)			/* input Sensor */ //센서 값을 체크합니다. 
{		
      
	Sensor1 = PINC ;
	Sensor2 = PINA ;
	Sensor3 = PINF ;
//   Sensor4 = readAdc(0x00);
//	Sensor5 = readAdc(0x01);

//  Sensor7 = readAdc(0x03);
   
}

void SOL_OFF (unsigned int number)
{
  if ( number== 0x01) // 펌프1
    {
	   PORTE |= 0x04;
    //   TX1_data(A);
	 }
     else if ( number== 0x02)  // 펌프2
     {
	   PORTE |= 0x08;
	 }
     else if ( number== 0x03)  // 펌프3
     {
	   PORTE |= 0x10;
	 }
	 else if ( number== 0x04) // 램프1
	 {
	   PORTE |= 0x20;
	 }
	 else if ( number== 0x05) // 램프2
     {
	   PORTE |= 0x40;
	 }
	 else if ( number== 0x06) // 예비
     {
	   PORTE |= 0x80;
	 }

}

void SOL_ON (unsigned int number)
{
  if ( number== 0x01)
     {
	   PORTE &= 0xFB;
	 }
     else if ( number== 0x02)
     {
	   PORTE &= 0xF7;
	 }
     else if ( number== 0x03)
     {
	   PORTE &= 0xEF;
	 }
	 else if ( number== 0x04)
     {
	   PORTE &= 0xDF;
	 }
	 else if ( number== 0x05)
     {
	   PORTE &= 0xBF;
	 }
	 else if ( number== 0x06)
     {
	   PORTE &= 0x7F;
	 }
}
void Motor_ON(unsigned int number)
{
  PORTG &= 0xFB; //enable
 // register unsigned int i;

  
 if( number == 0x10)  //Motor1 CCW
  { 
   while((PINC & 0x01) == 0x00)   // Motor1 도어 Opne (CW)
	   {
	    	
		PORTG |= 0x02;  // PG1=Dir =1 ------> CW
	    PORTG |= 0x01; 
        Delay_us(50);
	    PORTG &= 0xFE;
	    Delay_us(50);
	   }
  }

  else if (number== 0x11) // Motor1 도어 CLOSE1
	   {
	   for(k = 0; k < 20800; k++) // 6.5회전
      { 
	    PORTG &= 0xFD; // PG1=Dir=0 -----> CCW
	    PORTG |= 0x01; 
        Delay_us(50);
	    PORTG &= 0xFE;
	    Delay_us(50);
		 }
       }
  else if (number== 0x12) // Motor1 도어 CLOSE2
	   {
	   for(k = 0; k < 17280; k++)
      { 
	    PORTG &= 0xFD; // PG1=Dir=0 -----> CCW
	    PORTG |= 0x01; 
        Delay_us(50);
	    PORTG &= 0xFE;
	    Delay_us(50);
		 }
       }
  else if (number== 0x30)  // Motor3 도어 OPEN (CW)
	   {
	   while((PINC & 0x04) == 0x00)   // Motor3 Open 
      { 
		 
		 PORTB |= 0x04;  // PG1=Dir =1 ------> CW
	     PORTB |= 0x08; 
         Delay_us(50);
	     PORTB &= 0xF7;
	     Delay_us(50);
		 }
       }

  else if (number== 0x31) // Motor3 Close1 CCW
	   {
	    for(k = 0; k < 20500; k++)
      { 
		 
		 PORTB &= 0xFB;  // PG1=Dir =0 ------> CCW
	     PORTB |= 0x08; 
         Delay_us(50);
	     PORTB &= 0xF7;
	     Delay_us(50);
		 }
       }
  else if (number== 0x32) // Motor3 Close2 CCW
	   {
	    for(k = 0; k < 17280; k++)
      { 
		 
		 PORTB &= 0xFB;  // PG1=Dir =0 ------> CCW
	     PORTB |= 0x08; 
         Delay_us(50);
	     PORTB &= 0xF7;
	     Delay_us(50);
		 }
       }
  else if (number== 0x20)  // Motor2 CW
	   {
	while((PINC & 0x02) != 0x00)   // Motor2 Home
      { 
		 PORTB &= 0xEF;  // PG1=Dir =0 ------> CCW
	     PORTB |= 0x20; 
         Delay_us(50);
	     PORTB &= 0xDF;
	     Delay_us(50);
		 }
       }

  else if (number== 0x21) // Motor2 CCW
	   {
	    for(k = 0; k < 19840; k++)
      { 
		 PORTB |= 0x10;  // PG1=Dir =1 ------> CW
	     PORTB |= 0x20; 
         Delay_us(50);
	     PORTB &= 0xDF;
	     Delay_us(50);
		 }
       }

  else if (number== 0x40)  // Motor4 CW
	   {
	 while((PINC & 0x08) != 0x00)   // Motor4 Home
      { 
		 PORTB &= 0xBF;  // PG1=Dir =1 ------> CW
	     PORTB |= 0x80; 
         Delay_us(50);
	     PORTB &= 0x7F;
	     Delay_us(50);
		 }
       }

  else if (number== 0x41)  // Motor4 CCW
	   {
	    for(k = 0; k < 19840; k++)
      { 
		 PORTB |= 0x40;  // PG1=Dir =1 ------> CW
	     PORTB |= 0x80; 
         Delay_us(50);
	     PORTB &= 0x7F;
	     Delay_us(50);
		 }
       }

  else if (number== 0x60) // Motor6 CW
	   {
		while((PINC & 0x20) != 0x00)   // Motor6 Open
      { 
		 PORTD &= 0xEF;  // PG1=Dir =1 ------> CW
	     PORTD |= 0x20; 
         Delay_us(50);
	     PORTD &= 0xDF;
	     Delay_us(50);
		 }
       }

  else if (number== 0x61) // Motor6 CCW
	   {
	    for(k = 0; k < 19840; k++)
      { 
		 PORTD |= 0x10;  // PG1=Dir =1 ------> CW
	     PORTD |= 0x20; 
         Delay_us(50);
	     PORTD &= 0xDF;
	     Delay_us(50);
		 }
       }
  else if (number== 0x50) // Motor5 도어 OPen(CW)
	   {
        	while((PINC & 0x10) == 0x00)   // Motor5 
                  { 
		 
	            	 PORTD |= 0x40;  // PG1=Dir =1 ------> CW
	                 PORTD |= 0x80; 
                     Delay_us(50);
	                 PORTD &= 0x7F;
	                 Delay_us(50);
		          }
       }
 
  else if (number== 0x51) // Motor5 CLOSE1 (CCW)
	   {
	for(k = 0; k < 20800; k++) //22400
        { 
		 
		 PORTD &= 0xBF;  // PG1=Dir =0 ------> CCW
	     PORTD |= 0x80; 
         Delay_us(50);
	     PORTD &= 0x7F;
	     Delay_us(50);
		 }
       }
 
  else if (number== 0x52) // Motor5 CLOSE2 CCW
	   {
	    for(k = 0; k < 17280; k++)
      { 
		 PORTD &= 0xBF;  // PG1=Dir =0 ------> CCW
	     PORTD |= 0x80; 
         Delay_us(50);
	     PORTD &= 0x7F;
	     Delay_us(50);
		 }
       }
   else if (number == 0x00)  // Motor enable off
        {
		  PORTG &= 0xFF; //motor off
		}

  else if (number == 0xF1)  // Motor1 off
        {
		  PORTG &= 0xFE; //motor off
		}
  else if (number == 0xF3)  // Motor3  off
        {
		  PORTB &= 0xF7; //motor off
		}
  else if (number == 0xF2)  // Motor2  off
        {
		 PORTB &= 0xFD; //motor off
		}

  else if (number == 0xF4)  // Motor4 off
        {
		   PORTB &= 0x7F; //motor off
		}
  else if (number == 0xF6)  // Motor6 off
        {
		  PORTD &= 0xDF; //motor off
		}
  else if (number == 0xF5)  // Motor5 off
        {
		  PORTD &= 0x7F; //motor off
		}
 
}

void PickUP_initialize(void)
{
   PORTG &= 0xFB; //enable
 // register unsigned int i;

  while((PINC & 0x01) == 0x00)   // Motor1 Opne 
       {
	   	
		PORTG |= 0x02;  // PG1=Dir =1 ------> CW
	    PORTG |= 0x01; 
        Delay_us(50);
	    PORTG &= 0xFE;
	    Delay_us(50);	
       }
  //   Motor_ON(0xF1) ; // Door Close
     Delay_ms(1000);

  while((PINC & 0x02) != 0x00)   // Motor2 Home 
	   
      { 
		 PORTB &= 0xEF;  // PB4=Dir2 =0 ------>CCW
	     PORTB |= 0x20; 
         Delay_us(50);
	     PORTB &= 0xDF;
	     Delay_us(50);
		 
       }
    PORTB &= 0xFD; //motor2 off
    Motor_ON(0x11) ; // Door Close
 //   PORTG &= 0xFE; // Motor1 off
    Motor_ON(0xF1) ; // Door Close

  while((PINC & 0x04) == 0x00)   // Motor3 Open 
      { 
		 PORTB |= 0x04;  // PG1=Dir =1 ------> CW
	     PORTB |= 0x08; 
         Delay_us(50);
	     PORTB &= 0xF7;
	     Delay_us(50);
		 
       }
       Delay_ms(1000);

  while((PINC & 0x08) != 0x00)   // Motor4 Home 
      { 
		 PORTB &= 0xBF;  // PG1=Dir =1 ------> CW
	     PORTB |= 0x80; 
         Delay_us(50);
	     PORTB &= 0x7F;
	     Delay_us(50);
		 
       }
   PORTB &= 0x7F; //motor4 off
   Motor_ON(0x31) ; // Door close
   PORTB &= 0xF7; //motor3 off

  while((PINC & 0x10) == 0x00)   // Motor5 Open
      { 
		 PORTD |= 0x40;  // PD6=Dir5 =1 ------> CW
	     PORTD |= 0x80; 
         Delay_us(50);
	     PORTD &= 0x7F;
	     Delay_us(50);
		 
       }
       Delay_ms(1000);

 while((PINC & 0x20) != 0x00)   // Motor6 Home
      { 
		 PORTD &= 0xEF;  // PG1=Dir =1 ------> CW
	     PORTD |= 0x20; 
         Delay_us(50);
	     PORTD &= 0xDF;
	     Delay_us(50);
		 
       }
   PORTD &= 0xDF; //motor6 off
   Motor_ON(0x51) ; // motor 5 Close
   PORTD &= 0xDF; //motor6 off
   PORTG &= 0xFF; //motor enable off
}

void UART_initialize(void)
{
  UBRR1H = (uint8_t)(UBRR_VALUE>>8);
  UBRR1L = (uint8_t) UBRR_VALUE;
  UCSR1A = 0x00;                                // asynchronous normal mode
  UCSR1B = 0x98;                                // Rx/Tx enable, 8 data
  UCSR1C = 0x06;                               // no parity, 1 stop, 8 data
  RXD = UDR1;                                   // dummy read
}


void TX1_data(unsigned char data)		/* transmit a character by USART1 */ // TX함수 통신값을 내보낼때 사용
{
  while((UCSR1A & 0x20) == 0x00);		// data register empty ?
  UDR1 = data;
}




int main(void)
{
  
  cli();
 
  UART_initialize();							// 통신초기화 
  init_PORT();
  Delay_ms(50);
 // PickUP_initialize();							// Port 초기화
  Delay_ms(50);                                 // wait for system stabilization
  adcInit();
  sei();
  
  Delay_ms(2);

 
 
  while(1)
	{	
		int i=0;

//	Motor_ON(0x20);

      Delay_ms(100); // 반드시 적용 비 적용시 통신 안됨

   
  
       if(rx_buffer_overflow0)

        {
        
		  Delay_ms(100);

		  if(rx_buffer0[0]== 0x53 && rx_buffer0[1]== 0x45 && rx_buffer0[2]== 0x54)
		    {
                      			 			 
 			CMD = rx_buffer0[3];
			Delay_ms(10);
		    SEN = rx_buffer0[4];
            Delay_ms(10);
			NUM = rx_buffer0[5];
            Delay_ms(10);
			   		
				if(CMD == 0xA0)
				{
			 				   
					  Sensor_check();
					  Delay_ms(10);
                   	Sensor6 = readAdc(0x02);
					 Delay_ms(10);
					  TX1_data(A);
		     		  TX1_data(C);
		      	      TX1_data(K);
		     	      TX1_data(0xC0);
		      	      TX1_data(SEN);
					  TX1_data(Sensor1);
					  TX1_data(Sensor2);
					  TX1_data(Sensor3);
                      TX1_data(Sensor4); //CUP#1 센서 
					  TX1_data(Sensor5); //CUP#2 센서
					  TX1_data(Sensor6); //CUP#3 센서
					  TX1_data(Sensor7); //CUP#4 센서
		      	      TX1_data(EOT);		     	     
                    
				}

				else if( CMD == 0xB0)
				{
			       if (NUM == 0x00) // SOL OFF
				    {
					  SOL_OFF(SEN);	

                      Delay_ms(10);
					  TX1_data(A);
		     		  TX1_data(C);
		      	      TX1_data(K);
		     	      TX1_data(0xE0);
		      	      TX1_data(SEN);
					  TX1_data(NUM);
		      	      TX1_data(EOT);		     
					}
					else if (NUM == 0xFF) // SOL ON
					{
					  SOL_ON(SEN);
               
					  TX1_data(A);
		     		  TX1_data(C);
		      	      TX1_data(K);
		     	      TX1_data(0xE0);
		      	      TX1_data(SEN);
					  TX1_data(NUM);
		      	      TX1_data(EOT);
					 }
				}
			
	              

	//		}

              else if( CMD == 0xD0)
				{
			     
					  Motor_ON(SEN);	

                      Delay_ms(10);
					  TX1_data(A);
		     		  TX1_data(C);
		      	      TX1_data(K);
		     	      TX1_data(0xD0);
		      	      TX1_data(SEN);
					  TX1_data(NUM);
		      	      TX1_data(EOT);
					  k=0;	
					  PORTG &= 0xF8;  // mortor1 off
					  PORTG |= 0x04;  // mortor enable off
					  PORTD &= 0x0F;  // motor5,6 off
					  PORTB &= 0x03;	     
			
				}
			
	              

			}



          else if(rx_buffer0[0]=='H' && rx_buffer0[1]=='O' && rx_buffer0[2]=='M')
		   {

		    TX1_data(0x43);


		   }

          else if(rx_buffer0[0]=='C' && rx_buffer0[1]=='L' && rx_buffer0[2]=='E')
		   {

		   TX1_data(0x4B);

		   }
		  else
		   {
		      TX1_data(A);
		      TX1_data(C);
		      TX1_data(K);
		      TX1_data(0xFF);
		      TX1_data(0xFF);
		      TX1_data(0xFF);
		      TX1_data(0xFF);
		      TX1_data(0xFF);
		      TX1_data(0xFF); 

		   }

 
        rx_buffer_overflow0=0;

        } 



	} 

}



#pragma once

#include "arduino.h"

#define NUMBER_OF_SHIFT_CHIPS 2
#define DATA_WIDTH  NUMBER_OF_SHIFT_CHIPS * 8

#define LOAD 49 // PL=PL0=49 //out
#define DIN  48 // Q7=PL1=48 //in
#define CLK  46 // CP=PL3=46 //out
#define CE   16 // CE=PH1=16 //out

#define BYTES_VAL_T uint16_t

pinMode(LOAD,OUTPUT);
pinMode(DIN,INPUT);
pinMode(CLK,OUTPUT);
pinMode(CE,OUTPUT);
digitalWrite(CLK,  false);
digitalWrite(LOAD, true);
digitalWrite(CE,   false);

BYTES_VAL_T read_shift_regs()
{
    BYTES_VAL_T bitVal;
    BYTES_VAL_T bytesVal = 0;
    digitalWrite(LOAD, false);  //remove_noize
    delayMicroseconds(1);
    digitalWrite(LOAD, true);
    for(int i = 0; i < DATA_WIDTH; i++)
    {
        bitVal = digitalRead(DIN);
        bytesVal |= (bitVal << ((DATA_WIDTH-1) - i));
        digitalWrite(CLK, true);
        digitalWrite(CLK, false);
    }
    return(bytesVal);
}

boolean swich_values(uint8_t pin)
{
  BYTES_VAL_T pinValues = read_shift_regs();
  if((pinValues >> pin) & 1){
    return true;
  }else{
    return false;
  }
}

unsigned long interval_74HC165 = 0L;
void display_pin_values()
{
    BYTES_VAL_T pinValues = read_shift_regs();
    if(millis() > interval_74HC165 + 1000){
      interval_74HC165 = millis();
      Serial.print("Pin States:\r\n");
      for(int i = 0; i < DATA_WIDTH; i++)
      {
          Serial.print("  Pin-");
          Serial.print(i);
          Serial.print(": ");
          if((pinValues >> i) & 1)
              Serial.print("HIGH");
          else
              Serial.print("LOW");
          Serial.print("\r\n");
      }
      Serial.print("done.\r\n");
    }
}
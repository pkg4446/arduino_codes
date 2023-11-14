#include "pin_setup.h"
#include "shift_regs.h"
#include "arduino.h"

void shift_regs_init(){
  pinMode(LOAD,OUTPUT);
  pinMode(DIN,INPUT);
  pinMode(CLK,OUTPUT);
  pinMode(CE,OUTPUT);
  digitalWrite(CLK,  false);
  digitalWrite(LOAD, true);
  digitalWrite(CE,   false);
}

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

boolean swich_values(uint8_t pin, BYTES_VAL_T pinValues)
{
  if((DATA_WIDTH > pinValues) && ((pinValues >> pin) & 1)){
    return true;
  }else{
    return false;
  }
}
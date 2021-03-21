#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#define OSC_FREQ          22118400L
#define  RCAP2_50us      65536L - OSC_FREQ/40417L
#define  RCAP2_1ms       65536L - OSC_FREQ/2000L
#define  RCAP2_10ms      65536L - OSC_FREQ/1200L
#define  TIME0_500us     65536L - OSC_FREQ/8000L
#define  TIME0_10ms      65536L - OSC_FREQ/200
#define CALL_isr_UART()         TI = 1
#define CALL_isw_UART()          RI = 1
#define TRUE 1
#define FALSE 0

void initSystem();
void delay(unsigned char i);
void feeb();
#endif
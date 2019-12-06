#ifndef __TPC112S1_H__
#define __TPC112S1_H__

// 1.2v~2.6v 
//#define LIGHT_CONVER_TOOLS(num)  (num*14+1489)  

// 0.8v~2.3v  num(1~128)
//#define LIGHT_CONVER_TOOLS(num)  (num*15+992) 

// 0.9~2.5v
//#define LIGHT_CONVER_TOOLS(num)  (num*7+1117) 

// 1.2v~2.6v num(1~255)
#define LIGHT_CONVER_TOOLS(num)  (num*7+1489)  

void TPCDelay(void);

void TPCInit(void);

void TPCSendData(uint16_t SendVal);

void TPCCtrlLed(uint8_t level);

void TPCCtrlLedBreathe(uint8_t count);

#endif


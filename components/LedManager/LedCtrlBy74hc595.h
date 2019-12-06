#ifndef __74HC595_H__
#define __74HC595_H__

//#define LED_CHANGE_BUTTON

extern uint32_t key_led_map[3];

void HC595Init();

void HC595SendData(uint32_t SendVal);

#endif


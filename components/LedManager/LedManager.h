#ifndef __LED_SERVICE_H__
#define __LED_SERVICE_H__

#include "DeviceManager.h"

#define LED_ON						1
#define LED_OFF						2
#define LED_SLOW_BLINK				3
#define LED_FAST_BLINK				4
#define LED_FAST_FAST_BLINK			5

#define LAMP_OFF                    0
#define LAMP_ON                     1
#define LAMP_UNDEF                  0

#define LED_BREATHE_COUNT           2

#define CLASS_TIME (uint32_t)45*60*1000 // 45s
#define STUDY_TIME (uint32_t)60*60*1000 // 60s
#define EXAM_TIME  (uint32_t)90*60*1000 // 90s

//#define SE8324

typedef enum{
	LED_CLOUD = 0,
	LED_LOCAL,
	LED_MODE,
}LED_ID_T;

typedef enum{
	LED_BOOK_MODE = 0,
	LED_PHONE_MODE,
	LED_PAINT_MODE,
}LED_MODE_T;

typedef enum{
	LED_POWER_SWITCH = 0,
	LED_AUTO_LIGHT,
	LED_LIGHT_UP,
	LED_LIGHT_DOWN, 
	LED_LIGHT_SET,
	LED_TIME_MODE,
	LED_MODE_RESET,
}LED_CMD_T;

void LedCtrl(LED_ID_T id,LED_CMD_T cmd,uint16_t value); 


void loopLed_control(bool sel,uint8_t wheelValue);

ControllerVar_t *LedManagerCreate();

void wifi_led_control(uint8_t ledMode);
uint32_t get_loopLed_value();
void update_loopLed_value(uint32_t value,bool bitClearFlag);

#endif


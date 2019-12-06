#ifndef _TOUCH_BUTTON_H_
#define _TOUCH_BUTTON_H_
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "m_button.h"


#define APT8L08 	0
#define APT8S10 	1

typedef struct{
	uint8_t lastkeyvalue;
	uint8_t keyvalue;
}TOUCH_BTN_SERVICE;

void TouchBtnInit(ButtonCallback cb);

#endif

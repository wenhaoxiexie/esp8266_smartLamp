#include <stdio.h>
#include <stdlib.h>
#include "esp_system.h"
#include "string.h"
#include "TouchButton.h"
#include "APT8L08.h"
#include "APT8S10.h"
#include "log.h"
#include "LedManager.h"
#include "board.h"
#include "unistd.h"


#define TBT_TAG "TOUCH_BUTTON"
#define TOUCH_CHIP	APT8S10
#define TOUCH_BUTTON_TASK_PRIORITY			10
#define KEY_NUM		3

static const KeyName key_map[KEY_NUM] = { 
	KEY_1,
	KEY_2,
    KEY_3,
}; 

static uint8_t temp = 0;
static ButtonCallback btnCallback;
xQueueHandle toucn_btn_evt_queue = NULL;

void touchbtnTask(void *para)
{
	uint8_t wheel_value = 0;
	TOUCH_BTN_SERVICE touchbtn = {0};
	while(1){
		vTaskDelay(20/portTICK_PERIOD_MS);
#if (TOUCH_CHIP == APT8L08)
		APT8L08GetKeyValue(&(touchbtn.keyvalue));
		for(uint i = 0;i < KEY_NUM;i++){
			if(((touchbtn.keyvalue>>i)&0x01) != ((touchbtn.lastkeyvalue>>i)&0x01)){
				if((touchbtn.keyvalue>>i)&0x01){
					btnCallback(key_map[i], BTN_STATE_PUSH);
				}else{
					btnCallback(key_map[i], BTN_STATE_RELEASE);
				}
			}
		}
		touchbtn.lastkeyvalue = touchbtn.keyvalue;
#elif (TOUCH_CHIP == APT8S10)
		APT8S10GetWheelValue(&wheel_value);
		if(0 != wheel_value)
		{
			if(0 != wheel_value)
			{
				if(temp!=wheel_value)
				{
					LedCtrl(LED_LOCAL,LED_LIGHT_SET,wheel_value);	
				}
				temp = wheel_value;
				wheel_value = 0;
			}
		}

		APT8S10GetKeyValue(&(touchbtn.keyvalue));
		for(uint i = 0;i < KEY_NUM;i++){
			if(((touchbtn.keyvalue>>(i+1))&0x01) != ((touchbtn.lastkeyvalue>>(i+1))&0x01)){
				if((touchbtn.keyvalue>>(i+1))&0x01){
					btnCallback(key_map[i], BTN_STATE_PUSH);
				}else{
					btnCallback(key_map[i], BTN_STATE_RELEASE);
				}
			}
		}
		
		touchbtn.lastkeyvalue = touchbtn.keyvalue;
#endif		
	}
	vTaskDelete(NULL);
}


void TouchBtnInit(ButtonCallback cb)
{
	btnCallback = cb;	
#if (TOUCH_CHIP == APT8L08)
	if(APT8L08Init()){
		LOG(TBT_TAG,"APT8L08Init error");
		return;
	}
#elif (TOUCH_CHIP == APT8S10)
	if(APT8S10Init()){
		LOG(TBT_TAG,"APT8S10Init error");
		return;
	}
#endif
	if (xTaskCreatePinnedToCore(touchbtnTask,
									"touchbtnTask",
									2500,
									NULL,
									TOUCH_BUTTON_TASK_PRIORITY,
									NULL, 1) != pdPASS) {
		LOG(TBT_TAG, "Error create touchbtnTask");
		return;
	}

	LOG(TBT_TAG,"touchbtnTask created");
}


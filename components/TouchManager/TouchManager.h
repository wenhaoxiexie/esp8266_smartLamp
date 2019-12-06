#ifndef _TOUCH_MANAGER_H_
#define _TOUCH_MANAGER_H_
#include "DeviceManager.h"
#include "m_button.h"



typedef enum {
    DEVICE_NOTIFY_KEY_UNDEFINED                 = 0,

    //set
    DEVICE_NOTIFY_KEY_WIFI_SET,
 
	DEVICE_NOTIFY_KEY_AUTO_ADJUST,
	
	// SERVER
	DEVICE_NOTIFY_KEY_POWER_SWITCH_S,
	DEVICE_NOTIFY_KEY_LIGHT_SET_S,
	DEVICE_NOTIFY_KEY_AUTO_BUTTON_S,
	DEVICE_NOTIFY_KEY_TIME_S,
	// MQTT
	DEVICE_NOTIFY_KEY_POWER_SWITCH_M,
	DEVICE_NOTIFY_KEY_LIGHT_SET_M,
	DEVICE_NOTIFY_KEY_AUTO_BUTTON_M,
	DEVICE_NOTIFY_KEY_TIME_M,
	DEVICE_NOTIFY_KEY_MODE_RESET_M,
}KEY_EVENT_T;


typedef struct{
	KeyName name;
	UserKeyName push;
	UserKeyName long_push;
	uint8_t long_time;
	bool commbination;
}key_attribute_t;


typedef enum{
	READY_STATE = 0,
	FINISH_STATE,
	IDLE_STATE,
}Long_Key_State;

ControllerVar_t *TouchManagerCreate();


#endif

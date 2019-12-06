#ifndef _M_BUTTON_H_
#define _M_BUTTON_H_

typedef enum{
	KEY_1 = 0,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_MAX,
}KeyName;

typedef enum {
	USER_KEY_NULL = 0,
	USER_KEY_WIFI_SETTING,
	USER_KEY_MUTE,	
	USER_KEY_AUTO_ADJUST,
	USER_KEY_TIME_MODE,
	USER_KEY_MODE_PAINT,
	USER_KEY_LIGHT_UP,
	USER_KEY_LIGHT_DOWN,
	USER_KEY_POWER,
   	USER_KEY_MAX,
} UserKeyName;

typedef enum {
    BTN_STATE_RELEASE = 0, 
    BTN_STATE_PUSH,
} BtnState;


typedef enum{
	BTN_EVENT_IDLE = 0,
	BTN_EVENT_CLICK,
	BTN_EVENT_LONGPRESS,
	BTN_EVENT_CONTINUE,
	BTN_EVENT_RELEASE,
}BTN_EVENT_T;

typedef struct{
	UserKeyName name;
	BTN_EVENT_T event;
}Get_Key_t;

typedef void (*ButtonCallback) (KeyName id, BtnState state);


#endif

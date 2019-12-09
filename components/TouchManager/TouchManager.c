#include "TouchManager.h"
#include "log.h"
#include "TouchButton.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#include "ambientAdc.h"
#include "LedCtrlBy74hc595.h"
#include "LedManager.h"
#include "SystemMemory.h"
#include "WifiManager.h"
#include "LedCtrlByTPC112S1.h"

#define TAG "TouchManager"

static xQueueHandle QueTouchManager = NULL;
static xSemaphoreHandle xSemaphore = NULL;
static xTimerHandle btn_timer;
static Long_Key_State long_key_state;

static void btnCallback(KeyName id, BtnState state);
const static key_attribute_t key_attr[KEY_MAX] = {
	{KEY_1,	USER_KEY_POWER,USER_KEY_WIFI_SETTING,3,false},	
	{KEY_2,	USER_KEY_AUTO_ADJUST,USER_KEY_NULL,1,true},	
	{KEY_3,	USER_KEY_TIME_MODE,USER_KEY_NULL,1,true},	
	{KEY_4,	USER_KEY_MODE_PAINT,USER_KEY_NULL,3,true},		
};

static BtnState key_sta[KEY_MAX];
static bool wait_clear ;
static UserKeyName get_key;
static KeyName vaild_key;
extern bool autoButton;
extern xTimerHandle led_timer;
extern TimerHandle_t xTimers[3];
void KeyTimeroutCb()  
{  
   	Commom_msg_t msg = {0};
	msg.id = get_key;
	msg.action = BTN_STATE_PUSH;
	LOG(TAG,"*****************KeyTimeroutCb******************");
	xQueueSend(QueTouchManager,&msg, 0);
	if(long_key_state == READY_STATE)
		long_key_state = FINISH_STATE;
}  

void KeyDetectVarInit()
{
	int i = 0;
	for(i = 0;i < KEY_MAX;i++){
		key_sta[i] = BTN_STATE_RELEASE;
	}
	wait_clear = false;
	get_key = USER_KEY_NULL;
	vaild_key = KEY_MAX;
	long_key_state = IDLE_STATE;
	btn_timer = xTimerCreate("btn_timer", 50 / portTICK_RATE_MS,
                              pdTRUE, NULL, KeyTimeroutCb);
}

int CommbinationVaildCheck(KeyName id)
{
	if(KEY_2 == id || KEY_3 == id || KEY_4 == id){
		if((key_sta[KEY_2] == BTN_STATE_PUSH)&&(key_sta[KEY_3] == BTN_STATE_PUSH)&&(key_sta[KEY_4] == BTN_STATE_PUSH)){
			get_key = USER_KEY_MUTE;			
			xTimerChangePeriod(btn_timer,3000/portTICK_RATE_MS,0);
			xTimerReset(btn_timer,0);
			return 0;
		}	
	}
	return -1;
}

int VaildCheck(KeyName id)
{
	LOG(TAG,">>>>>>VaildCheck>>>>>>> id:%d",id);
	if(wait_clear || ((true == key_attr[id].commbination) && (0 == CommbinationVaildCheck(id))))
		return -1;
	for(int i = 0;i < KEY_MAX;i++){
		if((key_sta[i] == BTN_STATE_PUSH)&&(i != id))
			return -1;
	}
	vaild_key = id;

	
#ifdef LED_CHANGE_BUTTON
	HC595SendData(key_led_map[id]|get_loopLed_value());
#endif
	return 0;
}

int PushVaildCheck(KeyName id)
{
	Commom_msg_t msg = {0};
	LOG(TAG,">>>>>>PushVaildCheck>>>>>>> id:%d",id);

	if(key_attr[id].push != USER_KEY_NULL){
		msg.id = key_attr[id].push;
		msg.action = BTN_STATE_PUSH;
		xQueueSend( QueTouchManager,&msg, 0);
		return 0;
	}
	
	return -1;
}

int LongVaildCheck(KeyName id)
{
	if(key_attr[id].long_push != USER_KEY_NULL){
		get_key = key_attr[id].long_push;
		xTimerChangePeriod(btn_timer,key_attr[id].long_time*1000/portTICK_RATE_MS,0);
		xTimerReset(btn_timer,0);
		long_key_state = READY_STATE;
		return 0;
	}
	return -1;
}

int ReleaseCombineCheck(KeyName id)
{
	if(get_key == USER_KEY_MUTE){
		if((id == KEY_2)||(id == KEY_3)||(id == KEY_4)){
			wait_clear = true;
			xTimerStop(btn_timer,0);
			return 0;
		}
	}
	return -1;
}

int ReleaseVaildCheck(KeyName id)
{
	int i = 0;
	Commom_msg_t msg = {0};
	ReleaseCombineCheck(id);
	if(wait_clear){
		xTimerStop(btn_timer,0);
		for(i = 0;i < KEY_MAX;i++){
			if(key_sta[i] == BTN_STATE_PUSH)
				return -1;
		}
		wait_clear = false;
		get_key = USER_KEY_NULL;
		vaild_key = KEY_MAX;
	}else{
		if(id == vaild_key){
			xTimerStop(btn_timer,0);
			if(long_key_state == FINISH_STATE){
				LOG(TAG,"************* long_key_state %d ***************",id);
				msg.id = get_key;
				msg.action = BTN_STATE_RELEASE;
				xQueueSend( QueTouchManager,&msg, 0);
				long_key_state = IDLE_STATE;
			}else{
				LOG(TAG,"************* short_key_state %d ***************",id);
				msg.id = key_attr[id].push;
				msg.action = BTN_STATE_RELEASE;
				xQueueSend( QueTouchManager,&msg, 0);
			}
			
#ifdef LED_CHANGE_BUTTON
			HC595SendData((key_led_map[id]&0x0)|get_loopLed_value());
#endif
			vaild_key = KEY_MAX;
		}	
	}
	return 0;
}

static void btnCallback(KeyName id, BtnState state)
{	
	key_sta[id] = state;	
	if(BTN_STATE_PUSH == state){
		if(0 == VaildCheck(id))
		{
			PushVaildCheck(id);
			LongVaildCheck(id);
		}
			
	}else{
		ReleaseVaildCheck(id);
	}			
}

static void led_TimeMode_update()
{
	LIGHT_INFO_S lightInfo;
	
	myNVS_read(&lightInfo,sizeof(lightInfo));
	LOG(TAG,"*************led_TimeMode_update**********");
	switch(lightInfo.timeMode){
		case CLASS:
			lightInfo.timeMode=TIME_MODE_NULL;
			break;
		case STUDY:
			lightInfo.timeMode=EXAM;
			break;
		case EXAM:
			lightInfo.timeMode=CLASS;
			break;
		case TIME_MODE_NULL:
			lightInfo.timeMode=CLASS;
			break;
		default:
			lightInfo.timeMode=TIME_MODE_NULL;
			break;
	}

	myNVS_write(lightInfo);
	
}

static void Factory_Reset(void)
{	
	LIGHT_INFO_S lightInfo;
	myNVS_read(&lightInfo,sizeof(lightInfo));
	if(lightInfo.buttonState)
	{
		TPCSendData(LAMP_OFF);
		loopLed_control(LAMP_OFF,LAMP_OFF);
	}
	lightInfo.buttonState=false;
	lightInfo.autoLight=false;
	lightInfo.timeMode=TIME_MODE_NULL;
	lightInfo.lightness=125;
	autoButton =false;
	myNVS_write(lightInfo);
	

	//关闭定时器
	if( xTimerIsTimerActive(led_timer) != pdFALSE )
	{
		xTimerStop(led_timer,0);
		LOG(TAG,">>>>> STOP led_timer");
	}
	for(uint8_t i=0;i<3;i++)
	{
		if( xTimerIsTimerActive(xTimers[i]) != pdFALSE ) 
		{
			xTimerStop(xTimers[i],0);
			
			LOG(TAG,">>>>> STOP xTimers[%d]",i);
		}
	}
	
}

static void ProcessTouchEvent(DeviceVar_t* device, Commom_msg_t *msg)
{
	LIGHT_INFO_S lightInfo;
   	uint16_t touch_name = msg->id;
   	uint16_t action = msg->action;
   	DeviceNotifyMsg event = DEVICE_NOTIFY_KEY_UNDEFINED;
   	LOG(TAG,"touch _name = %d; event = %d",touch_name,action);
   	switch (touch_name) {
		case USER_KEY_POWER:
			if(action == BTN_STATE_RELEASE)
			{
				event = DEVICE_NOTIFY_KEY_POWER_SWITCH_S;
			}
		break;
		
		case USER_KEY_AUTO_ADJUST:
			if(action == BTN_STATE_RELEASE)
			{
				myNVS_read(&lightInfo,sizeof(lightInfo));
				// 熄灯状态下无法触发自动调节模式
				if(!lightInfo.buttonState)
					break;
				event = DEVICE_NOTIFY_KEY_AUTO_ADJUST;
			}	
		break;

		case USER_KEY_TIME_MODE:
			if(action == BTN_STATE_RELEASE)
			{
				//熄灯状态下无法触发模式管理
				myNVS_read(&lightInfo,sizeof(lightInfo));
				LOG(TAG,">>>>>>>>>>>>>>>>>>>>buttonState: %d",lightInfo.buttonState);
				if(!lightInfo.buttonState)
					break;
				led_TimeMode_update();
				event = DEVICE_NOTIFY_KEY_TIME_S;
			}
		break;

		case USER_KEY_WIFI_SETTING:
				Factory_Reset();
				wifiConfig();
		break;
	}

    if (msg != DEVICE_NOTIFY_KEY_UNDEFINED) {
        device->NotifyServices(device,KEY_EVENT, &event, sizeof(DeviceNotifyMsg));
    }
}


static void TouchManagerEvtTask(void* device)
{
	Commom_msg_t msg = {0};
	KeyDetectVarInit();
	TouchBtnInit(btnCallback);
	myadc_init();
	xSemaphoreTake( xSemaphore, portMAX_DELAY );
	while (1) {
		if(xQueueReceive(QueTouchManager,&msg,portMAX_DELAY)){
			if(msg.id == EXIT_EVENT)
				break;
			
        	ProcessTouchEvent(device, &msg);
		}
    }
	xSemaphoreGive(xSemaphore);
    vTaskDelete(NULL);
}

static int TouchManagerActive(DeviceVar_t* device)
{
    LOG(TAG, "TouchManagerActive");
	if (xTaskCreate(TouchManagerEvtTask, "TouchManagerEvtTask", 1024+512, device, 9, NULL) != pdPASS) {
        printf("TouchManagerActive failed\n");
		return -1;
    }
	QueTouchManager = xQueueCreate(5, sizeof(Commom_msg_t));
	xSemaphore = xSemaphoreCreateMutex();
	return 0;
}

static int TouchManagerDeactive(DeviceVar_t* device)
{
    //TODO
    LOG(TAG, "TouchManagerDeactive");
	Commom_msg_t msg = {0};
	msg.id = EXIT_EVENT;
	xQueueSend( QueTouchManager,&msg, 0);
	xSemaphoreTake(xSemaphore, portMAX_DELAY);
	xSemaphoreGive(xSemaphore);
	vSemaphoreDelete(xSemaphore);
	vQueueDelete(QueTouchManager);
	return 0;
}

ControllerVar_t *TouchManagerCreate()
{
    LOG(TAG, "TouchManagerCreate");
    ControllerVar_t *touch = (ControllerVar_t *) calloc(1, sizeof(ControllerVar_t));
    touch->ControllerActive = TouchManagerActive;
    touch->ControllerDeactive = TouchManagerDeactive;
    return touch;
}

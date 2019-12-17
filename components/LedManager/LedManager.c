#include "LedManager.h"
#include "log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "LedCtrlBy74hc595.h"
#include "LedCtrlByTPC112S1.h"
#include "SystemMemory.h"

#include "driver/adc.h"
#include "driver/hw_timer.h"
#include "freertos/timers.h"
#include "TouchManager.h"

#include "LedCtrlBy74hc595.h"
#include "unistd.h"
#include "LedCtrlBySe8324.h"


static xQueueHandle QueLedManager = NULL;
static xSemaphoreHandle xSemaphore = NULL;

static uint32_t Led_High_Time_Set = 0xffffffff;
static uint32_t Led_High_Time_Cnt = 0;
static uint32_t Led_Low_Time_Set  = 0xffffffff;
static uint32_t Led_Low_Time_Cnt  = 0;
static uint8_t led_old_light;
static uint8_t led_dir_light;

static bool led_loop_dir_f=true;
static uint8_t led_loop_dir=1;

#define TAG		"LedManager"

xTimerHandle led_timer;
bool autoButton=false;

static const uint32_t ledMap[24]={0x00000008,0x00000004,0x00000002,0x00000001,0x00004000,0x00002000,
								  0x00001000,0x00000800,0x00000400,0x00000200,0x00000100,0x00400000,
								  0x00200000,0x00100000,0x00080000,0x00040000,0x00020000,0x00010000,
								  0x04000000,0x02000000,0x01000000,0x00000040,0x00000020,0x00000010
								  };
static uint32_t loopLedValue=0;

extern int utcSeconds;

uint32_t get_loopLed_value()
{
	//LOG(__FUNCTION__,">>>>>>>loopLedValue:%x",loopLedValue);
	return loopLedValue;
}

void update_loopLed_value(uint32_t value,bool bitClearFlag)
{
	//LOG(__FUNCTION__,">>>>>>>loopLedValue:%x",loopLedValue);

	if(bitClearFlag)	
		loopLedValue&=(~value);
	else
		loopLedValue|=value;
	
	//LOG(__FUNCTION__,">>>>>>>loopLedValue:%x",loopLedValue);
}

void loopLed_control(bool sel,uint8_t wheelValue)
{
	uint8_t ledNum;
	uint32_t dataLed=0;

	if(sel)
	{
		if(wheelValue>240)
			wheelValue = 255;
		ledNum = wheelValue/11;

		for(uint8_t i=0;i<=ledNum;i++)
		{
			dataLed |= ledMap[i];
		}
		#ifndef LED_CHANGE_BUTTON
		
		//LOG(TAG,">>>>>>>%d loopLedValue:%x",__LINE__,loopLedValue);
		loopLedValue = ((loopLedValue&key_led_map[0])|(loopLedValue&key_led_map[1])|\
						(loopLedValue&key_led_map[2])|dataLed);
		//LOG(TAG,">>>>>>>%d loopLedValue:%x",__LINE__,loopLedValue);
		
		HC595SendData(loopLedValue);
		#else
		loopLedValue = dataLed;
		
		HC595SendData(dataLed);
		#endif
		
	}else{
		
		loopLedValue = LAMP_OFF;
		HC595SendData(LAMP_OFF);
	}
}
static void ledTimeMode_Set()
{
	LIGHT_INFO_S lightInfo;
	Time_Mode_E timeMode;

	// 获取当前时间管理模式
	myNVS_read(&lightInfo,sizeof(lightInfo));
	timeMode = lightInfo.timeMode;
	switch(timeMode){
		case CLASS:
			LOG(TAG,">>>>>>>>>>>>>timeMode:%d ",lightInfo.timeMode);
			xTimerChangePeriod(led_timer,CLASS_TIME/portTICK_PERIOD_MS,100);
			xTimerReset(led_timer,0);
			
#ifndef LED_CHANGE_BUTTON
			update_loopLed_value(key_led_map[2],false);
			HC595SendData(get_loopLed_value());
#endif
			break;
		case STUDY:
			LOG(TAG,">>>>>>>>>>>>>timeMode:%d ",lightInfo.timeMode);
			xTimerChangePeriod(led_timer,STUDY_TIME/portTICK_PERIOD_MS,100);
			xTimerReset(led_timer,0);
#ifndef LED_CHANGE_BUTTON
			update_loopLed_value(key_led_map[2],false);
			HC595SendData(get_loopLed_value());
#endif
			break;
		case EXAM:
			LOG(TAG,">>>>>>>>>>>>>timeMode:%d ",lightInfo.timeMode);
			xTimerChangePeriod(led_timer,EXAM_TIME/portTICK_PERIOD_MS,100);
			xTimerReset(led_timer,0);
#ifndef LED_CHANGE_BUTTON
			update_loopLed_value(key_led_map[2],false);
			HC595SendData(get_loopLed_value());
#endif
			break;
		default:
			LOG(TAG,">>>>>>>>>>>>>timeMode:%d ",lightInfo.timeMode);
			xTimerStop(led_timer,0);
#ifndef LED_CHANGE_BUTTON
			update_loopLed_value(key_led_map[2],true);
			HC595SendData(get_loopLed_value());
#endif
			break;	
	}
}

void LedCtrl(LED_ID_T id,LED_CMD_T cmd,uint16_t value) 
{
	Commom_msg_t msg = {0};
	msg.id = id;
	msg.action = cmd;
	msg.value = value;
	xQueueSend(QueLedManager,&msg,0);
}
static void led_autoLight_off(LIGHT_INFO_S *lightInfo,DeviceVar_t* device)
{
	DeviceNotifyMsg event = DEVICE_NOTIFY_KEY_UNDEFINED;
	
	if(autoButton)
	{
		autoButton = false;
		event = DEVICE_NOTIFY_KEY_AUTO_BUTTON_M;
		//myNVS_read(&lightInfo,sizeof(lightInfo));
		lightInfo->autoLight = false;
		myNVS_write(*lightInfo);
#ifndef LED_CHANGE_BUTTON
		update_loopLed_value(key_led_map[1],true);
		HC595SendData(get_loopLed_value());
#endif
		device->NotifyServices(device,KEY_EVENT, &event, sizeof(DeviceNotifyMsg));
	}
}

static void led_timeMode_off(LIGHT_INFO_S *lightInfo,DeviceVar_t* device)
{
	
	DeviceNotifyMsg event = DEVICE_NOTIFY_KEY_UNDEFINED;
	if(lightInfo->timeMode!=TIME_MODE_NULL)
	{
		lightInfo->timeMode=TIME_MODE_NULL;
		event = DEVICE_NOTIFY_KEY_TIME_M;		
		myNVS_write(*lightInfo);
		ledTimeMode_Set();
		device->NotifyServices(device,KEY_EVENT, &event, sizeof(DeviceNotifyMsg));
	}
}

static void ledMangerProcess(DeviceVar_t* device,Commom_msg_t msg)
{
	LIGHT_INFO_S lightInfo;
	DeviceNotifyMsg event = DEVICE_NOTIFY_KEY_UNDEFINED;
	
	//LOG(TAG,"ledMsg id:%d action:%d value:%d",msg.id,msg.action,msg.value);

	switch (msg.action)
	{		
		// 开关设置
		case LED_POWER_SWITCH:
		{
			myNVS_read(&lightInfo,sizeof(lightInfo));

			led_autoLight_off(&lightInfo,device);
			
			if(msg.id==LED_LOCAL)
			{
				if(lightInfo.buttonState)
				{
					lightInfo.buttonState=false;
					led_timeMode_off(&lightInfo,device);
					#ifndef SE8324
					TPCSendData(LAMP_OFF);
					#else
					se8324_adjust_light(LAMP_OFF,LAMP_OFF);
					#endif
					loopLed_control(LAMP_OFF,LAMP_OFF);

					#ifndef LED_CHANGE_BUTTON
					update_loopLed_value(key_led_map[0],true);
					HC595SendData(get_loopLed_value());
					#endif
				}else{
					lightInfo.buttonState=true;
					#ifndef SE8324
					TPCCtrlLed(lightInfo.lightness);
					#else
					se8324_adjust_light(LAMP_ON,lightInfo.lightness);
					#endif
					loopLed_control(LAMP_ON,lightInfo.lightness);

					#ifndef LED_CHANGE_BUTTON
					update_loopLed_value(key_led_map[0],false);
					HC595SendData(get_loopLed_value());
					#endif
				}
			}
			else if(msg.id==LED_CLOUD)
			{
				if(msg.value==LAMP_ON)
				{
					lightInfo.buttonState=true;
					#ifndef SE8324
					TPCCtrlLed(lightInfo.lightness);
					#else
					se8324_adjust_light(LAMP_ON,lightInfo.lightness);
					#endif
					loopLed_control(LAMP_ON,lightInfo.lightness);

					#ifndef LED_CHANGE_BUTTON
					update_loopLed_value(key_led_map[0],false);
					HC595SendData(get_loopLed_value());
					#endif
				}
				else if(msg.value==LAMP_OFF)
				{
					lightInfo.buttonState=false;
					led_timeMode_off(&lightInfo,device);
					#ifndef SE8324
					TPCSendData(LAMP_OFF);
					#else
					se8324_adjust_light(LAMP_OFF,LAMP_OFF);
					#endif
					loopLed_control(LAMP_OFF,LAMP_OFF);

					#ifndef LED_CHANGE_BUTTON
					update_loopLed_value(key_led_map[0],true);
					HC595SendData(get_loopLed_value());
					#endif
				}
			}
				
			myNVS_write(lightInfo);
			event = DEVICE_NOTIFY_KEY_POWER_SWITCH_M;
			break;
		}
		// 亮度设置上报
		case LED_LIGHT_SET:
		{	
			#if 0
			myNVS_read(&lightInfo,sizeof(lightInfo));
			
			led_autoLight_off(&lightInfo,device);
			LOG(TAG,">>>>>>> lamp_light: %d",msg.value);
			loopLed_control(LAMP_ON,msg.value);
			if(msg.value>128)
				msg.value = 256-msg.value;
			
			TPCCtrlLed(msg.value);
			
			lightInfo.buttonState=true;
			lightInfo.lightness=msg.value;
			
			myNVS_write(lightInfo);
			event = DEVICE_NOTIFY_KEY_LIGHT_SET_M;
			#endif
			
			if(msg.id==LED_LOCAL)
			{
				myNVS_read(&lightInfo,sizeof(lightInfo));
				led_autoLight_off(&lightInfo,device);
				if(lightInfo.buttonState==false)
				{
					#ifndef LED_CHANGE_BUTTON
					update_loopLed_value(key_led_map[0],false);
					HC595SendData(get_loopLed_value());
					#endif
				}

				//LOG(TAG,">>>>>>> lamp_light: %d  led_old_light :%d led_loop_dir_f:%d led_dir_light:%d led_loop_dir:%d",
				         			//msg.value,led_old_light,led_loop_dir_f,led_dir_light,led_loop_dir);
				if(abs(msg.value-led_old_light)<100&&(led_loop_dir_f))
				{
					
					if(msg.value>led_old_light)
						led_loop_dir=1; // 顺时针
					else 
						led_loop_dir=3;  // 逆时针
				
					led_old_light = msg.value;
					led_dir_light = msg.value;
					lightInfo.buttonState=true;
					lightInfo.lightness=msg.value;
					
					loopLed_control(LAMP_ON,msg.value);
					#ifndef SE8324
					TPCCtrlLed(msg.value);
					#else
					se8324_adjust_light(LAMP_ON,msg.value);
					#endif
					myNVS_write(lightInfo);
					event = DEVICE_NOTIFY_KEY_LIGHT_SET_M;
				}else{
						switch(led_loop_dir){
						case 1:
							led_loop_dir_f=false;
							led_loop_dir = 2;
							led_dir_light = msg.value;	
							break;
						case 2:							
							if(msg.value<led_dir_light&&(abs(msg.value-led_dir_light)<100))
								led_loop_dir_f = true;
							else
								led_dir_light = msg.value;
							break;
						case 3:
							led_dir_light = msg.value;
							led_loop_dir = 4;
							led_loop_dir_f = false;
							break;
						case 4:
							if(msg.value>led_dir_light&&(abs(msg.value-led_dir_light)<100))
								led_loop_dir_f = true;
							else
								led_dir_light = msg.value;
							break;
						default:
							break;
					}
				}
			}
			else if(msg.id==LED_CLOUD){
				
				myNVS_read(&lightInfo,sizeof(lightInfo));

				#ifndef LED_CHANGE_BUTTON
				update_loopLed_value(key_led_map[0],false);
				HC595SendData(get_loopLed_value());
				#endif
				
				led_autoLight_off(&lightInfo,device);
				LOG(TAG,">>>>>>> lamp_light: %d",msg.value);
				loopLed_control(LAMP_ON,msg.value);
				#ifndef SE8324
				TPCCtrlLed(msg.value);
				#else
				se8324_adjust_light(LAMP_ON,msg.value);
				#endif
				lightInfo.buttonState=true;
				lightInfo.lightness=msg.value;
				led_old_light = msg.value;
				myNVS_write(lightInfo);
				event = DEVICE_NOTIFY_KEY_LIGHT_SET_M;
			}
			break;
		} 
		// 自动调光设置
		case LED_AUTO_LIGHT:
		{
			myNVS_read(&lightInfo,sizeof(lightInfo));

			if(lightInfo.buttonState)
			{
				if(msg.value==LAMP_ON)
				{
					autoButton = true;
					
#ifndef LED_CHANGE_BUTTON
					update_loopLed_value(key_led_map[1],false);
					HC595SendData(get_loopLed_value());
#endif
					
				}else{
					autoButton = false;
					
#ifndef LED_CHANGE_BUTTON
					update_loopLed_value(key_led_map[1],true);
					HC595SendData(get_loopLed_value());
#endif
				}
				lightInfo.buttonState=true;
				myNVS_write(lightInfo);
				event = DEVICE_NOTIFY_KEY_AUTO_BUTTON_M;
			}
			
			break;
		}
		// 时间管理设置
		case LED_TIME_MODE:
		{   
			myNVS_read(&lightInfo,sizeof(lightInfo));
			led_autoLight_off(&lightInfo,device);
			if(lightInfo.buttonState)
			{
				ledTimeMode_Set();
				#ifndef SE8324
				TPCCtrlLedBreathe(msg.value);
				#else
				se8324_breath_light(LED_BREATHE_COUNT);
				#endif
				
				event = DEVICE_NOTIFY_KEY_TIME_M;
			}
			break;
		}
		case LED_MODE_RESET:
		{
			event = DEVICE_NOTIFY_KEY_MODE_RESET_M;
		}
		default:
		break;
	}	
	
	//if(msg.id == LED_LOCAL)
	{
		device->NotifyServices(device,KEY_EVENT, &event, sizeof(DeviceNotifyMsg));
	}
}
static void LedManagerEvtTask(void* device)
{
	Commom_msg_t msg = {0};
	xSemaphoreTake( xSemaphore, portMAX_DELAY );
	while (1) {
		if(xQueueReceive(QueLedManager,&msg,portMAX_DELAY)){
			if(msg.id == EXIT_EVENT){
				break;
			}
			ledMangerProcess(device,msg);
		}
	}
	xSemaphoreGive(xSemaphore);
    vTaskDelete(NULL);
}

 static void auto_light_task()
 {
 	uint8_t light=64;
	uint16_t adcValue;
	LIGHT_INFO_S lightInfo;
	
	myNVS_read(&lightInfo,sizeof(lightInfo));

	light = lightInfo.lightness;
	while(1)
	{
		if(autoButton)
		{
			adc_read(&adcValue);
			if(adcValue<200)
			{
				light++;
				#ifndef SE8324
				TPCCtrlLed(light);
				#else
				se8324_adjust_light(LAMP_ON,light);
				#endif
			}
			else if(adcValue>500)
			{
				light--;
				#ifndef SE8324
				TPCCtrlLed(light);
				#else
				se8324_adjust_light(LAMP_ON,light);
				#endif
			}else{
				loopLed_control(LAMP_ON,light);
			}
			LOG(TAG,">>>>>>>>>>adcValue :%d ",adcValue);
			usleep(50000);
		}else{
			usleep(500000);
		}
	}
	 vTaskDelete(NULL);
 }
 void wifi_led_control(uint8_t ledMode)
 {
	if( ledMode == LED_ON ){
		HC595SendData(0xffffffff);
		Led_High_Time_Cnt = 0;
		Led_Low_Time_Cnt  = 0;
		Led_High_Time_Set = 0xffffffff;
		Led_Low_Time_Set  = 0xffffffff;
	}
	else if( ledMode == LED_OFF ) {
		HC595SendData(0x0);

		Led_High_Time_Cnt = 0;
		Led_Low_Time_Cnt  = 0;
		Led_High_Time_Set = 0xffffffff;
		Led_Low_Time_Set  = 0xffffffff;
		
	}
	else if( ledMode == LED_SLOW_BLINK ){
		Led_High_Time_Cnt = 0;
		Led_Low_Time_Cnt  = 0;
		Led_High_Time_Set = 7;
		Led_Low_Time_Set  = 7;
		
	}
	else if( ledMode == LED_FAST_BLINK ){
		Led_High_Time_Cnt = 0;
		Led_Low_Time_Cnt  = 0;
		Led_High_Time_Set = 2;
		Led_Low_Time_Set  = 2;
		
	}
	else if( ledMode == LED_FAST_FAST_BLINK ){
		Led_High_Time_Cnt = 0;
		Led_Low_Time_Cnt  = 0;
		Led_High_Time_Set = 1;
		Led_Low_Time_Set  = 1;
		
	}
 }

static void led_timer_cb(void *arg)
{
	LIGHT_INFO_S lightInfo;
	
	// 时间到， 灯光匀速呼吸两次
	LOG(TAG,"***************led_timer_cb********************");

	#ifndef SE8324
	TPCCtrlLedBreathe(LED_BREATHE_COUNT);
	#else
	se8324_breath_light(LED_BREATHE_COUNT);
	#endif
	myNVS_read(&lightInfo,sizeof(lightInfo));
	lightInfo.timeMode = TIME_MODE_NULL;

	myNVS_write(lightInfo);
	// 通过led 触发模式上报
	LedCtrl(LED_LOCAL,LED_MODE_RESET,0);	
	
	xTimerStop(led_timer,0);
}
 static void hw_timer_cb(void *arg)
 {
	static int led_state = 1;

	if( led_state == 1 ){
		Led_High_Time_Cnt++;

		if(Led_High_Time_Cnt%10==0)
			utcSeconds++;
		if( Led_High_Time_Cnt >= Led_High_Time_Set ){
			HC595SendData(0x0);
			led_state = 0;
			Led_Low_Time_Cnt = 0;
		}
	}
	else{
		Led_Low_Time_Cnt++;

		if(Led_Low_Time_Cnt%10==0)
			utcSeconds++;
		if( Led_Low_Time_Cnt >= Led_Low_Time_Set ){
			HC595SendData(0xffffffff);
			led_state = 1;
			Led_High_Time_Cnt = 0;
		}
	}
	
 }

 static int LedManagerActive(DeviceVar_t* device)
 {
 	LIGHT_INFO_S lightInfo;
	LOG(TAG, "LedManagerActive");
	HC595Init();

	#ifndef SE8324
	TPCInit();
	#else
	se8324_pwm_init();
	#endif
    hw_timer_init(hw_timer_cb, NULL);
    hw_timer_alarm_us(100000, true);
	
	led_timer = xTimerCreate("led_timer", 50 / portTICK_RATE_MS,pdTRUE, NULL, led_timer_cb);
	if(led_timer==NULL)
	{
		printf("LedManagerActive failed\n");
		return -1;
	}
	if (xTaskCreate(LedManagerEvtTask, "LedManagerEvtTask", 1024+512, device, 5, NULL) != pdPASS) {
        printf("LedManagerActive failed\n");
		return -1;
    }

    if(xTaskCreate(auto_light_task, "auto_light_task", 1024, NULL, 5, NULL)!=pdPASS)
    {
		printf("create auto_light_task failed\n");
		return -1;
	}
	
	QueLedManager = xQueueCreate(3, sizeof(Commom_msg_t));
	xSemaphore = xSemaphoreCreateMutex();
	// 恢复掉电之前状态
	myNVS_read(&lightInfo,sizeof(lightInfo));
	if(lightInfo.buttonState)
	{
		#ifndef SE8324
		TPCCtrlLed(lightInfo.lightness);
		#else
		se8324_adjust_light(LAMP_ON,lightInfo.lightness);
		#endif
		loopLed_control(LAMP_ON,lightInfo.lightness);
		led_old_light = lightInfo.lightness;
		#ifndef LED_CHANGE_BUTTON
		update_loopLed_value(key_led_map[0],false);
		HC595SendData(get_loopLed_value());
		#endif
	}else{
		#ifndef SE8324
		TPCSendData(LAMP_OFF);
		#else
		se8324_adjust_light(LAMP_OFF,lightInfo.lightness);
		#endif
		loopLed_control(LAMP_OFF,lightInfo.lightness);
		#ifndef LED_CHANGE_BUTTON
		update_loopLed_value(key_led_map[0],true);
		HC595SendData(get_loopLed_value());
		#endif
	}
	//自动调光处于开启状态下掉电，重新上电默认关闭
	if(lightInfo.autoLight)
	{
		lightInfo.autoLight=false;
		myNVS_write(lightInfo);
	}

	return 0;
 }

 
 static int LedManagerDeactive(DeviceVar_t* device)
 {
	 //TODO
	 LOG(TAG, "LedManagerDeactive");
	 Commom_msg_t msg = {0};
	 msg.id = EXIT_EVENT;
	 xQueueSend( QueLedManager,&msg, 0);
	 xSemaphoreTake(xSemaphore, portMAX_DELAY);
	 xSemaphoreGive(xSemaphore);
	 vSemaphoreDelete(xSemaphore);
	 vQueueDelete(QueLedManager);
	 hw_timer_disarm();
     hw_timer_deinit();
	 return 0;
 }

 
 ControllerVar_t *LedManagerCreate()
 {
	 LOG(TAG, "LedManagerCreate");
	 ControllerVar_t *m_led = (ControllerVar_t *) calloc(1, sizeof(ControllerVar_t));
	 m_led->ControllerActive = LedManagerActive;
	 m_led->ControllerDeactive = LedManagerDeactive;
	 return m_led;
 }
 


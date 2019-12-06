#include "log.h"
#include "TouchService.h"
#include "TouchManager.h"
#include "WifiManager.h"
#include "LedManager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "SystemMemory.h"


#define TAG                   "TouchService"
static xQueueHandle QueTouchSer = NULL;
static xSemaphoreHandle xSemaphore = NULL;
static LIGHT_INFO_S lightInfo;

static void DeviceEvtNotifiedToTouch(DeviceNotification *note)
{
	DeviceNotifyMsg data = *((DeviceNotifyMsg *) note->data);
	Commom_msg_t msg = {0};
	if((note->type == KEY_EVENT)||(note->type == WIFI_EVENT)) {
		msg.id = note->type;
		msg.action = data;
	} 
	if(msg.id != 0){
		xQueueSend(QueTouchSer,&msg, 0);
	}
}

void touchServiceTask()
{
	Commom_msg_t msg = {0};
	xSemaphoreTake( xSemaphore, portMAX_DELAY);
	while(1){
		if(xQueueReceive(QueTouchSer,&msg,portMAX_DELAY)){
			if(msg.id == EXIT_EVENT){
				break;
			}else if(KEY_EVENT == msg.id){
				switch(msg.action){
					case DEVICE_NOTIFY_KEY_POWER_SWITCH_S:
					{
						LedCtrl(LED_LOCAL,LED_POWER_SWITCH,LAMP_UNDEF);
						break;
					}
					case DEVICE_NOTIFY_KEY_AUTO_ADJUST:
					{
						myNVS_read(&lightInfo,sizeof(lightInfo));

						LOG(TAG,">>>>>>>>>>>>>autoLight:%d",lightInfo.autoLight);
						if(lightInfo.autoLight)
						{
							lightInfo.autoLight=false;
							myNVS_write(lightInfo);
							LedCtrl(LED_LOCAL,LED_AUTO_LIGHT,LAMP_OFF);
						}else{
							lightInfo.autoLight=true;
							myNVS_write(lightInfo);
							LedCtrl(LED_LOCAL,LED_AUTO_LIGHT,LAMP_ON);
						}
						break;
					}
					case DEVICE_NOTIFY_KEY_TIME_S:
					{
						
						LedCtrl(LED_LOCAL,LED_TIME_MODE,LED_BREATHE_COUNT);
						break;
					}
					case DEVICE_NOTIFY_KEY_WIFI_SET:
						wifiConfig();
						break;
					default:
						break;
				}
			}
		}  
	}
	xSemaphoreGive(xSemaphore);
    vTaskDelete(NULL);  
}

static void TouchServiceActive(ServiceVar_t *self)
{
    LOG(TAG, "TouchServiceActive");
	QueTouchSer = xQueueCreate(3, sizeof(Commom_msg_t));
	xSemaphore = xSemaphoreCreateMutex();
	if (xTaskCreate(touchServiceTask,"touchServiceTask",(1024),NULL,6,NULL) != pdPASS) {
		LOG(TAG, "Error create touchServiceTask");
    }
	return;
}

static void TouchServiceDeactive(ServiceVar_t *self)
{
    LOG(TAG, "TouchServiceDeactive");
	Commom_msg_t msg = {0};
	msg.id = EXIT_EVENT;
	xQueueSend( QueTouchSer,&msg, 0);
	xSemaphoreTake(xSemaphore, portMAX_DELAY);
	xSemaphoreGive(xSemaphore);
	vSemaphoreDelete(xSemaphore);
	vQueueDelete(QueTouchSer);
	return;
}

ServiceVar_t *TouchServiceCreate()
{
    LOG(TAG, "TouchServiceCreate");
    ServiceVar_t *touch = (ServiceVar_t *) calloc(1, sizeof(ServiceVar_t));
    touch->deviceEvtNotified = DeviceEvtNotifiedToTouch;
    touch->serviceActive = TouchServiceActive;
    touch->serviceDeactive = TouchServiceDeactive;
    return touch;
}

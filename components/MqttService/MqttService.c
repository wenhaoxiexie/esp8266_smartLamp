#include "log.h"
#include "MqttService.h"
#include "WifiManager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "mqtt_client.h"
#include "cJSON.h"
#include "SystemMemory.h"
#include "LedManager.h"
#include "OtaService.h"
#include "board.h"

#include "TouchManager.h"

#include <sys/time.h>
#include <esp_wifi.h>
#include "driver/hw_timer.h"
#include "driver/adc.h"
#include "LedCtrlByTPC112S1.h"

#include "SoftwareRTC.h"
#include "interface_rtc.h"
#include "unistd.h"
#include "LedCtrlBySe8324.h"

#define NUM_TIMERS 4
#define TAG                   "MqttService"

extern bool smartConfigFlag;

esp_mqtt_client_handle_t mqtt_client = NULL;

static xQueueHandle QueMqttSer = NULL;
static xSemaphoreHandle xSemaphore = NULL;

TimerHandle_t xTimers[NUM_TIMERS];
static TimerInfo_t timeInfo[NUM_TIMERS];
//static uint8_t timeIndex=0;

char event_topic[50] = {0};
static char director_topic[50] = {0};
char lwt_topic[50] = {0};
static char lwt_msg[64] = {0};
static char check_topic[50]={0};
char ota_topic[50] = {0};

static char device_mac[18];

static const char* devStateType[STA_TYPE_MAX] = {
	"lamp_power",
	"lamp_light",
	"ota_state",
	"lamp_time_mode",
	"lamp_auto_light",
	"smartConfig"
};

static void ReportDevCheck(esp_mqtt_client_handle_t client)
{
	float tempValue;
	char timeMode[4];
	char level_str[5] = {0};
	LIGHT_INFO_S lightInfo;
	
	cJSON* uploadInfo = cJSON_CreateObject();

	
	cJSON_AddStringToObject(uploadInfo,"mac",device_mac);
	
	myNVS_read(&lightInfo,sizeof(lightInfo));

	// 版本号	
	cJSON_AddStringToObject(uploadInfo,"version",lightInfo.version);
	
	// 自动调光
	if(lightInfo.autoLight)
	{
		cJSON_AddStringToObject(uploadInfo,devStateType[AUTO_BUTTON],"ON");
	}else{
		cJSON_AddStringToObject(uploadInfo,devStateType[AUTO_BUTTON],"OFF");
	}
	// 开关
	if(lightInfo.buttonState)
	{
		cJSON_AddStringToObject(uploadInfo,devStateType[LAMP_POWER],"ON");
	}else{
		cJSON_AddStringToObject(uploadInfo,devStateType[LAMP_POWER],"OFF");
	}
	// 亮度
	tempValue = lightInfo.lightness/2.55;
	if((tempValue-(uint8_t)tempValue)>0)
	{
		lightInfo.lightness = (uint8_t)tempValue+1;
	}else if((tempValue-(uint8_t)tempValue)==0){
		lightInfo.lightness = (uint8_t)tempValue;
	}
	itoa(lightInfo.lightness,level_str,10);
	cJSON_AddStringToObject(uploadInfo,devStateType[LAMP_LEVEL],level_str);
	
	// 模式
	itoa(lightInfo.timeMode,timeMode,10);
	cJSON_AddStringToObject(uploadInfo,devStateType[LAMP_TIMEMANAGE],timeMode);

	// 定时信息
	cJSON_AddStringToObject(uploadInfo,"timer0",timeInfo[0].time);
	/*
	if(timeInfo[0].state)
		cJSON_AddStringToObject(uploadInfo,"timer0_state","ON");
	else	
		cJSON_AddStringToObject(uploadInfo,"timer0_state","OFF");*/
	
	cJSON_AddStringToObject(uploadInfo,"timer1",timeInfo[1].time);
	/*
	if(timeInfo[1].state)
		cJSON_AddStringToObject(uploadInfo,"timer1_state","ON");
	else	
		cJSON_AddStringToObject(uploadInfo,"timer1_state","OFF");*/
	
	// 延时信息
	cJSON_AddStringToObject(uploadInfo,"delay0",timeInfo[2].time);
	
	cJSON_AddStringToObject(uploadInfo,"delay1",timeInfo[3].time);
	/*
	if(timeInfo[2].state)
		cJSON_AddStringToObject(uploadInfo,"delay_state","ON");
	else	
		cJSON_AddStringToObject(uploadInfo,"delay_state","OFF");*/

	char* check_str =cJSON_Print(uploadInfo);
	LOG(TAG,">>>>>>>check_str:%s",check_str);
	esp_mqtt_client_publish(client,check_topic,check_str,strlen(check_str),0,0);

	free(check_str);
	cJSON_Delete(uploadInfo);
	
}

void ReportDevEvent(esp_mqtt_client_handle_t client,char* topic,const char* state_type,char* state)
{

	int reValue = 110;
	cJSON* uploadInfo = cJSON_CreateObject();

	cJSON_AddStringToObject(uploadInfo,"mac",device_mac);
	cJSON_AddStringToObject(uploadInfo,state_type,state);

	char* event_str =cJSON_Print(uploadInfo);
	LOG(TAG,">>>>>>>event_str:%s",event_str);
	reValue = esp_mqtt_client_publish(client,topic,event_str,strlen(event_str),0,0);
	
	LOG(TAG,">>>>>>>reValue:%d",reValue);
	free(event_str);
	cJSON_Delete(uploadInfo);
	
}

static void ReportOnlineStatus(esp_mqtt_client_handle_t client)
{
	cJSON* uploadInfo = cJSON_CreateObject();

	cJSON_AddStringToObject(uploadInfo,"mac",device_mac);
	cJSON_AddStringToObject(uploadInfo,"status","online");

	char* lwt_str =cJSON_Print(uploadInfo);
	LOG(TAG,">>>>>>>lwt_str:%s",lwt_str);
	esp_mqtt_client_publish(client,lwt_topic,lwt_str,strlen(lwt_str),0,0);

	free(lwt_str);
	cJSON_Delete(uploadInfo);
}

static void cJsonOffline()
{
	cJSON* uploadInfo = cJSON_CreateObject();

	cJSON_AddStringToObject(uploadInfo,"mac",device_mac);
	cJSON_AddStringToObject(uploadInfo,"status","offline");

	char* offLine = cJSON_Print(uploadInfo);

	strcpy(lwt_msg,offLine);

	LOG(TAG,">>>>>>>>>>>>>>lwt_msg:%d",strlen(lwt_msg));
	
	free(offLine);
	cJSON_Delete(uploadInfo);
}
static void ReportSmartConfig(esp_mqtt_client_handle_t client)
{
	ReportDevEvent(client,event_topic,devStateType[SMART_CONFIG],"connected");
}
/*
static void ReportSoftVerion(esp_mqtt_client_handle_t client,char* topic)
{
	LIGHT_INFO_S lightInfo;

	myNVS_read(&lightInfo,sizeof(lightInfo));
	LOG(TAG,">>>>>>>>>>>>>soft_ver:%s",lightInfo.version);

	ReportDevEvent(client,topic,"version",lightInfo.version);
}
*/
static void ReportLampPower(esp_mqtt_client_handle_t client,char* topic)
{
	LIGHT_INFO_S lightInfo;

	myNVS_read(&lightInfo,sizeof(lightInfo));

	if(lightInfo.buttonState)
	{
		ReportDevEvent(client,topic,devStateType[LAMP_POWER],"ON");
	}else{
		ReportDevEvent(client,topic,devStateType[LAMP_POWER],"OFF");
	}
}

static void ReportLampLight(esp_mqtt_client_handle_t client,char* topic)
{
	float tempValue;
	char level_str[5] = {0};
	
	LIGHT_INFO_S lightInfo;
	
	myNVS_read(&lightInfo,sizeof(lightInfo));
	#if 0
	tempValue = lightInfo.lightness/1.28;
	if((tempValue-(uint8_t)tempValue)>0)
	{
		lightInfo.lightness = (uint8_t)tempValue+1;
	}else if((tempValue-(uint8_t)tempValue)==0){
		lightInfo.lightness = (uint8_t)tempValue;
	}
	#endif 
	tempValue = lightInfo.lightness/2.55;
	if((tempValue-(uint8_t)tempValue)>0)
	{
		lightInfo.lightness = (uint8_t)tempValue+1;
	}else if((tempValue-(uint8_t)tempValue)==0){
		lightInfo.lightness = (uint8_t)tempValue;
	}
	itoa(lightInfo.lightness,level_str,10);
	
	ReportDevEvent(client,topic,devStateType[LAMP_LEVEL],level_str);
	
}

static void ReportLampAutoButton(esp_mqtt_client_handle_t client,char* topic)
{
	LIGHT_INFO_S lightInfo;

	myNVS_read(&lightInfo,sizeof(lightInfo));

	if(lightInfo.autoLight)
	{
		ReportDevEvent(client,topic,devStateType[AUTO_BUTTON],"ON");
	}else{
		ReportDevEvent(client,topic,devStateType[AUTO_BUTTON],"OFF");
	}
}
static void ReportLampTimeManage(esp_mqtt_client_handle_t client,char* topic)
{
	char timeMode[4];
	LIGHT_INFO_S lightInfo;

	myNVS_read(&lightInfo,sizeof(lightInfo));

	LOG(TAG,">>>>>>>>>timeMode:%d",lightInfo.timeMode);
	itoa(lightInfo.timeMode,timeMode,10);
	
	ReportDevEvent(client,topic,devStateType[LAMP_TIMEMANAGE],timeMode);
}

static void FirstReportDevState(esp_mqtt_client_handle_t client)
{
	LIGHT_INFO_S lightInfo;
	
	ReportOnlineStatus(client);
#if 1
	ReportDevCheck(client);
#else
	ReportSoftVerion(client,check_topic);
	ReportLampPower(client,check_topic);
	ReportLampLight(client,check_topic);
	ReportLampAutoButton(client,check_topic);
	ReportLampTimeManage(client,check_topic);
#endif

	myNVS_read(&lightInfo,sizeof(lightInfo));
	if(lightInfo.buttonState)
	{
		#ifndef SE8324
		TPCCtrlLed(lightInfo.lightness);
		#else
		se8324_adjust_light(LAMP_ON,lightInfo.lightness);
		#endif
		loopLed_control(LAMP_ON,lightInfo.lightness);
		
	}else{
		#ifndef SE8324
		TPCSendData(LAMP_OFF);
		#else
		se8324_adjust_light(LAMP_OFF,lightInfo.lightness);
		#endif
		loopLed_control(LAMP_OFF,lightInfo.lightness);
	}
	
}

static void SubscribeProcess(esp_mqtt_client_handle_t client)
{

	char ota_topic_all[50] = {0};
	
	sprintf(ota_topic_all,"%s/#",ota_topic);
	esp_mqtt_client_subscribe(client, ota_topic_all, 2);
	esp_mqtt_client_subscribe(client, director_topic, 2);
}


static void led_time_mode_parse(Time_Mode_E timeMode)
{
	LIGHT_INFO_S lightInfo;

	myNVS_read(&lightInfo,sizeof(lightInfo));
	lightInfo.timeMode = timeMode;
	myNVS_write(lightInfo);
}

static uint8_t led_step_adjust(bool sel)
{
	uint8_t step = 10;
	LIGHT_INFO_S lightInfo;
	uint8_t lightSet;

	myNVS_read(&lightInfo,sizeof(lightInfo));

	lightSet = lightInfo.lightness;
	if(sel)
	{
		if(lightSet<=118)
		{
			lightSet += step;
			
		}
	}else{
		if(lightSet>10)
		{
			lightSet -= step;
			
		}
	}
	return lightSet;
		
}

// 2019-09-12 12:25:32
static int parsesServerTime(char* time_str,RTC_UTCTimeStruct* serverTime)
{
	
	if(time_str[4]!='-' || time_str[7]!='-' || time_str[10]!=' ' ||
		time_str[13]!=':' || time_str[16]!=':' )
		return -1;

	serverTime->year = (time_str[0] - '0') * 1000 + (time_str[1] - '0') * 100 +
		            (time_str[2] - '0') * 10   + (time_str[3] - '0');

	serverTime->month = (time_str[5] - '0') * 10   + (time_str[6] - '0');

	serverTime->day = (time_str[8] - '0') * 10   + (time_str[9] - '0');

	serverTime->hour = (time_str[11] -'0') * 10   + (time_str[12] -'0');

	serverTime->minutes = (time_str[14] -'0') * 10   + (time_str[15] -'0');

	serverTime->seconds = (time_str[17] -'0') * 10   + (time_str[18] -'0');
	
	return ESP_OK;
}

static void ParseDirector(char* director)
{
	LIGHT_INFO_S lightInfo;
	cJSON *payload_json = NULL;
	cJSON *value = NULL;
	cJSON *operate =NULL;
	int level;

	RTC_UTCTimeStruct serverTime;
	
	//LOG(TAG,">>>>>>>director: %s",director);
	payload_json = cJSON_Parse(director);	

	// 解析系统时间
	value = cJSON_GetObjectItem(payload_json, "server_time");
	if(value != NULL){
		if(parsesServerTime(value->valuestring,&serverTime)==ESP_OK) 
			write_rtc(serverTime.year,serverTime.month,serverTime.day,serverTime.hour,serverTime.minutes,serverTime.seconds);
	}

	// 解析查询
	value = cJSON_GetObjectItem(payload_json, "check");
	if(value != NULL){
		FirstReportDevState(mqtt_client);		
	}

	// 解析灯光开关状态
	value = cJSON_GetObjectItem(payload_json, "lamp_power");
	if(value != NULL){
		if(!strcmp(value->valuestring,"ON")){
			LedCtrl(LED_CLOUD,LED_POWER_SWITCH,LAMP_ON);
		}else if(!strcmp(value->valuestring,"OFF")){
			LedCtrl(LED_CLOUD,LED_POWER_SWITCH,LAMP_OFF);
		}
	}

	// 解析灯光亮度值
	value = cJSON_GetObjectItem(payload_json, "lamp_light");
	if(value != NULL){

		if(!strcmp(value->valuestring,"high")){
			
			LedCtrl(LED_CLOUD,LED_LIGHT_SET,led_step_adjust(true)); 
		}
		else if(!strcmp(value->valuestring,"low")){
			LedCtrl(LED_CLOUD,LED_LIGHT_SET,led_step_adjust(false)); 
		}else{
			level = atoi(value->valuestring);
			#if 0
			if(level>0&&level<=100)
				LedCtrl(LED_CLOUD,LED_LIGHT_SET,(uint8_t)(level*1.28));
			#endif 
			if(level>0&&level<=100)
				LedCtrl(LED_CLOUD,LED_LIGHT_SET,(uint8_t)(level*2.55));
		}
	}

	// 解析时间管理
	value = cJSON_GetObjectItem(payload_json, "lamp_time_mode");
	if(value != NULL){
		LOG(TAG,">>>>>>>>>mode: %s",value->valuestring);
		level = atoi(value->valuestring);
		led_time_mode_parse(level);
		LedCtrl(LED_CLOUD,LED_TIME_MODE,LED_BREATHE_COUNT);
	}
	// 解析自动调光设置
	value = cJSON_GetObjectItem(payload_json, "lamp_auto_light");
	if(value != NULL){
		LOG(TAG,">>>>>>>>lamp_auto_light:%s",value->valuestring);
		myNVS_read(&lightInfo,sizeof(lightInfo));
		if(!strcmp(value->valuestring,"ON")){
			lightInfo.autoLight=true;
			myNVS_write(lightInfo);
			LedCtrl(LED_CLOUD,LED_AUTO_LIGHT,LAMP_ON);
		}else if(!strcmp(value->valuestring,"OFF")){
			lightInfo.autoLight=false;
			myNVS_write(lightInfo);
			LedCtrl(LED_CLOUD,LED_AUTO_LIGHT,LAMP_OFF);
		}
	}

	// 解析定时任务
	value = cJSON_GetObjectItem(payload_json, "lamp_timer");
	if(value!=NULL)
	{
		operate = cJSON_GetObjectItem(payload_json, "operation");
		if(operate!=NULL)
		{
			if(!strcmp(operate->valuestring,"ON"))
			{
				timeInfo[0].seconds=atoi(value->valuestring);
				timeInfo[0].state=true;
				xTimerChangePeriod(xTimers[0],timeInfo[0].seconds*1000/portTICK_PERIOD_MS,100);
				xTimerReset(xTimers[0],0);
				value = cJSON_GetObjectItem(payload_json, "date_time");
				if(value!=NULL)
				{
					strcpy(timeInfo[0].time,value->valuestring);
					LOG(TAG,">>>>>> time[0]:%s",timeInfo[0].time);
				}
			}
			else if(!strcmp(operate->valuestring,"OFF"))
			{
				timeInfo[1].seconds=atoi(value->valuestring);
				timeInfo[1].state=false;
				xTimerChangePeriod(xTimers[1],timeInfo[1].seconds*1000/portTICK_PERIOD_MS,100);
				xTimerReset(xTimers[1],0);
				value = cJSON_GetObjectItem(payload_json, "date_time");
				if(value!=NULL)
				{
					strcpy(timeInfo[1].time,value->valuestring);
					LOG(TAG,">>>>>> time[1]:%s",timeInfo[1].time);
				}
			}
		}
	}

	// 解析延时任务
	value = cJSON_GetObjectItem(payload_json, "lamp_delay");
	if(value!=NULL)
	{
		operate = cJSON_GetObjectItem(payload_json, "operation");
		if(operate!=NULL)
		{
			if(!strcmp(operate->valuestring,"ON"))
			{
				timeInfo[2].seconds=atoi(value->valuestring);
				timeInfo[2].state=true;
				xTimerChangePeriod(xTimers[2],timeInfo[2].seconds*1000/portTICK_PERIOD_MS,100);
				xTimerReset(xTimers[2],0);
				value = cJSON_GetObjectItem(payload_json, "date_time");
				if(value!=NULL)
				{
					strcpy(timeInfo[2].time,value->valuestring);
					LOG(TAG,">>>>>> time[0]:%s",timeInfo[2].time);
				}
			}
			else if(!strcmp(operate->valuestring,"OFF"))
			{
				timeInfo[3].seconds=atoi(value->valuestring);
				timeInfo[3].state=false;
				xTimerChangePeriod(xTimers[3],timeInfo[3].seconds*1000/portTICK_PERIOD_MS,100);
				xTimerReset(xTimers[3],0);
				value = cJSON_GetObjectItem(payload_json, "date_time");
				if(value!=NULL)
				{
					strcpy(timeInfo[3].time,value->valuestring);
					LOG(TAG,">>>>>> time[1]:%s",timeInfo[3].time);
				}
			}
		}
	}
	
	#if 0
	value = cJSON_GetObjectItem(payload_json, "lamp_timer");
	if(value!=NULL){
		timeIndex++;
		LOG(TAG,">>>>>>>>timer:%s,%d",value->valuestring,timeIndex%2);
		timeInfo[timeIndex%2].seconds= atoi(value->valuestring);
		value = cJSON_GetObjectItem(payload_json, "operation");
		if(value!=NULL){
			if(!strcmp(value->valuestring,"ON")){
				timeInfo[timeIndex%2].state=true;
			}else if(!strcmp(value->valuestring,"OFF")){
				timeInfo[timeIndex%2].state=false;
			}
			xTimerChangePeriod(xTimers[timeIndex%2],timeInfo[timeIndex%2].seconds*1000/portTICK_PERIOD_MS,100);
			xTimerReset(xTimers[timeIndex%2],0);
			value = cJSON_GetObjectItem(payload_json, "date_time");
			LOG(TAG,">>>>>>>date_time:%s",value->valuestring)
			if(value!=NULL)
			{
				strcpy(timeInfo[timeIndex%2].time,value->valuestring);
				LOG(TAG,">>>>>> time[%d]:%s",timeIndex%2,timeInfo[timeIndex%2].time);
			}
		}
	}
	// 解析延时任务
	value = cJSON_GetObjectItem(payload_json, "lamp_delay");
	if(value!=NULL){
		LOG(TAG,">>>>>>>>delay:%s",value->valuestring);
		timeInfo[2].seconds= atoi(value->valuestring);
		value = cJSON_GetObjectItem(payload_json, "operation");
		if(value!=NULL){
			if(!strcmp(value->valuestring,"ON")){
				timeInfo[2].state=true;
			}else if(!strcmp(value->valuestring,"OFF")){
				timeInfo[2].state=false;
			}
			xTimerChangePeriod(xTimers[2],timeInfo[2].seconds*1000/portTICK_PERIOD_MS,100);
			xTimerReset(xTimers[2],0);
			value = cJSON_GetObjectItem(payload_json, "date_time");
			LOG(TAG,">>>>>>>date_time:%s",value->valuestring)
			if(value!=NULL)
			{
				strcpy(timeInfo[2].time,value->valuestring);
				LOG(TAG,">>>>>> time:%s",timeInfo[2].time);
			}
		}
	}

	#endif
	cJSON_Delete(payload_json);
}

extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");


static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;	
	static char last_topic[50] = {0};
    
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            LOG(TAG, "MQTT_EVENT_CONNECTED");
			
	        SubscribeProcess(client);
			if(smartConfigFlag)
			{
				smartConfigFlag=false;
				ReportSmartConfig(client);
			}
			FirstReportDevState(client);
			break;
        case MQTT_EVENT_DISCONNECTED:
            LOG(TAG, "MQTT_EVENT_DISCONNECTED");
			
            break;

        case MQTT_EVENT_SUBSCRIBED:
            LOG(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            LOG(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            LOG(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
			LOG(TAG, "MQTT_EVENT_DATA, msg_id=%d", event->msg_id);
			if((NULL == event->topic)&&(strlen(last_topic))){
				event->topic = last_topic;
				event->topic_len = strlen(last_topic);
			}else{
				//LOG(TAG,"TOPIC=%.*s", event->topic_len, event->topic);
				memset(last_topic,0,sizeof(last_topic));
				memcpy(last_topic,event->topic,event->topic_len);
			}
			if(!strncmp(director_topic,event->topic,strlen(director_topic))){
				
				ParseDirector(event->data);
			}else if(!strncmp(ota_topic,event->topic,strlen(ota_topic))){
				
				OtaProcess(event);
			}
            break;
        case MQTT_EVENT_ERROR:
            LOG(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}


static esp_mqtt_client_handle_t mqtt_client_init(void)
{	
	cJsonOffline();
	
    const esp_mqtt_client_config_t mqtt_cfg = {

#if 1
        .uri = "mqtts://ih.giec.cn",
		.port = 8883,
		.lwt_msg = lwt_msg,
        .lwt_topic = lwt_topic,
        .lwt_qos = 0, 
        .keepalive = 10,
        .event_handle = mqtt_event_handler,
        .client_cert_pem = (const char *)client_cert_pem_start,
        .client_key_pem = (const char *)client_key_pem_start,
        //.buffer_size = 2048,
#else
		.uri = "mqtt://192.168.1.106", 
		.port = 1883,
        .lwt_msg = lwt_msg,
        .lwt_topic = lwt_topic,
        .lwt_qos = 0,
        .event_handle = mqtt_event_handler,
        //.client_cert_pem = (const char *)client_cert_pem_start,
        //.client_key_pem = (const char *)client_key_pem_start,
#endif
    };
	
	LOG(TAG,">>>>>>>>>mqtt_cfg.uri:%s",mqtt_cfg.uri);
	LOG(TAG,">>>>>>>>>mqtt_cfg.lwt_topic:%s",mqtt_cfg.lwt_topic);
	LOG(TAG,">>>>>>>>>mqtt_cfg.keepalive:%d",mqtt_cfg.keepalive);
	LOG(TAG,">>>>>>>>>mqtt_cfg.lwt_qos:%d",mqtt_cfg.lwt_qos);
	LOG(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
	return client;
}

void MqttServiceTask(void* param)
{
	Commom_msg_t msg = {0};
	esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)param;
	xSemaphoreTake( xSemaphore, portMAX_DELAY );
	while(1){
		if(xQueueReceive(QueMqttSer,&msg,portMAX_DELAY)){
			if(msg.id == EXIT_EVENT){
				esp_mqtt_client_stop(client);
				break;
			}else if((msg.id == WIFI_EVENT)&&(msg.action == DEVICE_NOTIFY_WIFI_GOT_IP)){
				
				esp_mqtt_client_start(client);
			}else if((msg.id == WIFI_EVENT)&&(msg.action == DEVICE_NOTIFY_WIFI_DISCONNECTED)){
				
				LOG(TAG,"****************WIFI_EVENT disconnected ******************");
				esp_mqtt_client_stop(client);
				
			}else if((msg.id == KEY_EVENT)&&(msg.action == DEVICE_NOTIFY_KEY_POWER_SWITCH_M)){
				// 开关状态上报
				LOG(TAG,"****************UPLOAD BUTTON******************");
				ReportLampPower(client,event_topic);
			}else if((msg.id == KEY_EVENT)&&(msg.action == DEVICE_NOTIFY_KEY_LIGHT_SET_M)){
				// 开关亮度
				LOG(TAG,"****************UPLOAD LIGHT******************");
				ReportLampLight(client,event_topic);
			}else if((msg.id == KEY_EVENT)&&(msg.action == DEVICE_NOTIFY_KEY_AUTO_BUTTON_M)){
				// 自动调光开关
				LOG(TAG,"****************UPLOAD AUTO BUTTON******************");
				ReportLampAutoButton(client,event_topic);
			}else if((msg.id == KEY_EVENT)&&(msg.action == DEVICE_NOTIFY_KEY_TIME_M)){
				// 时间管理上报
				LOG(TAG,"****************UPLOAD TIME-MANGER******************");
				ReportLampTimeManage(client,event_topic);
			}else if((msg.id == KEY_EVENT)&&(msg.action == DEVICE_NOTIFY_KEY_MODE_RESET_M)){
				// 模式复位上报
				ReportLampTimeManage(client,event_topic);
			}

		}  
	}
	xSemaphoreGive(xSemaphore);
    vTaskDelete(NULL);
}

static void DeviceEvtNotifiedToMqtt(DeviceNotification *note)
{
	DeviceNotifyMsg data = *((DeviceNotifyMsg *) note->data);
	Commom_msg_t msg = {0};
	if (note->type == WIFI_EVENT||note->type == KEY_EVENT) {
		msg.id = note->type;
		msg.action = data;
	}
	if(msg.id != 0){
		xQueueSend(QueMqttSer,&msg, 0);
	}
}

static void get_sys_mac(char* device)
{
	uint8_t mac[6];

	char* pStr=NULL;

	pStr = device;

	if(pStr==NULL)
		return ;
	esp_wifi_get_mac(ESP_IF_WIFI_STA,mac);	
	
	sprintf(pStr,"%x:%x:%x:%x:%x:%x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

	LOG(TAG,">>>>>>>>>>>>>mac:%s",pStr);
}
void vTimerCallback(TimerHandle_t xTimer)
{
	uint32_t ulCount;

	configASSERT(xTimer);
	
	ulCount = (uint32_t)pvTimerGetTimerID(xTimer);
	LOG(TAG,">>>>>>>>ulCount:%d",ulCount);

	if(timeInfo[ulCount].state)
		LedCtrl(LED_CLOUD,LED_POWER_SWITCH,LAMP_ON);
	else
		LedCtrl(LED_CLOUD,LED_POWER_SWITCH,LAMP_OFF);	
	xTimerStop(xTimer,0);
}
static void MqttServiceActive(ServiceVar_t *self)
{
	int i=0;
	
    LOG(TAG, "MqttServiceActive");

	get_sys_mac(device_mac);

	sprintf(director_topic,"/topic/%s/%s/%s/director",device_mac,DEV_PID,DEV_DID);
	sprintf(ota_topic,"/topic/%s/%s/%s/ota",device_mac,DEV_PID,DEV_DID);
	
	sprintf(event_topic,"/topic/%s/%s/event",DEV_PID,DEV_DID);
	sprintf(check_topic,"/topic/%s/%s/check",DEV_PID,DEV_DID);
	sprintf(lwt_topic,"/topic/%s/%s/lwt",DEV_PID,DEV_DID);
	
	LOG(TAG,">>>>>>>>>>>>director_topic:%s",director_topic);
	LOG(TAG,">>>>>>>>>>>>ota_topic:%s",ota_topic);
	LOG(TAG,">>>>>>>>>>>>event_topic:%s",event_topic);
	LOG(TAG,">>>>>>>>>>>>check_topic:%s",check_topic);
	LOG(TAG,">>>>>>>>>>>>lwt_topic:%s",lwt_topic);
	
	QueMqttSer = xQueueCreate(3, sizeof(Commom_msg_t));
	xSemaphore = xSemaphoreCreateMutex();
	mqtt_client = mqtt_client_init();
	if (xTaskCreate(MqttServiceTask,"MqttServiceTask",(2048+2048),mqtt_client,8,NULL) != pdPASS) {
		LOG(TAG, "Error create MqttServiceTask");
    }
	
	for(i=0;i<NUM_TIMERS;i++)
	{
		xTimers[i] = xTimerCreate("Timer_delay",50/portTICK_RATE_MS,pdTRUE,(void*)i,vTimerCallback);
		if(xTimers[i]==NULL)
		{
			LOG(TAG,"**************** xTimerCreate %d failed**********************",i);
		}else{
			LOG(TAG,"**************** xTimerCreate %d succeed**********************",i);
		}	
	}	
	return;
}

static void MqttServiceDeactive(ServiceVar_t *self)
{
    LOG(TAG, "MqttServiceDeactive");
	Commom_msg_t msg = {0};
	msg.id = EXIT_EVENT;
	xQueueSend( QueMqttSer,&msg, 0);
	xSemaphoreTake(xSemaphore, portMAX_DELAY);
	xSemaphoreGive(xSemaphore);
	vSemaphoreDelete(xSemaphore);
	vQueueDelete(QueMqttSer);
	esp_mqtt_client_destroy(mqtt_client);
	return;
}

ServiceVar_t *MqttServiceCreate()
{
    LOG(TAG, "MqttServiceCreate");
    ServiceVar_t *mqtt = (ServiceVar_t *) calloc(1, sizeof(ServiceVar_t));
    mqtt->deviceEvtNotified = DeviceEvtNotifiedToMqtt;
    mqtt->serviceActive = MqttServiceActive;
    mqtt->serviceDeactive = MqttServiceDeactive;
    return mqtt;
}

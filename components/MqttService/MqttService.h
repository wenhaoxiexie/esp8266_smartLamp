#ifndef _MQTT_SERVICE_H_
#define _MQTT_SERVICE_H_

#include "DeviceManager.h"
#include "mqtt_client.h"

typedef enum{
	LAMP_POWER = 0,
	LAMP_LEVEL,
	OTA_STATE,
	LAMP_TIMEMANAGE,
	AUTO_BUTTON,
	SMART_CONFIG,
	STA_TYPE_MAX,
}DevState_T;

typedef struct timerInfo{
	bool state;
	uint32_t seconds;
	char time[32];
}TimerInfo_t;
ServiceVar_t *MqttServiceCreate();
void ReportDevEvent(esp_mqtt_client_handle_t client,char*topic,const char* state_type,char* state);
#endif

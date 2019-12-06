#ifndef _OTA_SERVICE_H_
#define _OTA_SERVICE_H_

#include "mqtt_client.h"


typedef struct{
       long firmware_size;
       uint8_t checksum;
       char version[20];
}OTA_PACKAGE_INFO;

typedef enum{
	OTA_IDLE = 0,
	OTA_RUNING,
}OTA_STATE_T;

typedef struct{
	OTA_STATE_T	ota_sta;
	uint8_t rec_checksum;
	uint32_t rec_len;
	uint8_t rec_percent;
	uint8_t last_percent;
	OTA_PACKAGE_INFO pack_info;
}OTA_SER_VAR;

void OtaProcess(esp_mqtt_event_handle_t event);


#endif

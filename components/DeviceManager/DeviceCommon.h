#ifndef __DEVICE_COMMON_H__
#define __DEVICE_COMMON_H__
#include <stdio.h>
#include <stdint.h>

#include "esp_system.h"


typedef int DeviceNotifyMsg;

typedef enum{
	KEY_EVENT = 1,
	WIFI_EVENT,
	EXIT_EVENT=10,
	EVENT_TPYE_MAX,
}EVENT_T;

typedef struct DeviceNotification {
    EVENT_T type;
    void *data;
    int len;
} DeviceNotification;


typedef struct{
	uint16_t id;
	uint16_t action;
	uint16_t value;
}Commom_msg_t;


#endif

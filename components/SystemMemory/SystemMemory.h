#ifndef _SYSTEMMEMORY_H_
#define _SYSTEMMEMORY_H_

#include "DeviceManager.h"
#include "nvs.h"
#include "nvs_flash.h"	

typedef enum{
	CLASS = 1,
	STUDY ,
	EXAM ,
	TIME_MODE_NULL,
}Time_Mode_E;

typedef struct{
	bool autoLight;
	bool buttonState;
	uint8_t lightness;
	Time_Mode_E timeMode;
	char version[64];
}LIGHT_INFO_S;

void myNVS_read(LIGHT_INFO_S *lightInfo,uint8_t len);

void myNVS_write(LIGHT_INFO_S lightInfo);

ServiceVar_t *MemoryServiceCreate();
	
	
#endif
 

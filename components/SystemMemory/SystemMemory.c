#include "SystemMemory.h"
#include "log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "interface_rtc.h"
#include "LedCtrlByTPC112S1.h"
#include "LedManager.h"

#define TAG		"SystemMemory"

static xSemaphoreHandle xSemaphore = NULL;

static const char* LIGHT_NAMESPACE = "light space";

static const char* LIGHT_DATA = "light data";

static void myNVS_init(LIGHT_INFO_S lightInfo)
{
	nvs_handle handle;
	esp_err_t reValue;
	size_t Len;
	LIGHT_INFO_S locLightInfo;

	memset(&locLightInfo,0,sizeof(locLightInfo));
	
	Len = sizeof(lightInfo);

    ESP_ERROR_CHECK( nvs_open( LIGHT_NAMESPACE, NVS_READWRITE, &handle) );

	reValue = nvs_get_blob( handle, LIGHT_DATA, &locLightInfo,&Len);
	
	LOG(TAG,">>>>>>>>>>>>>reValue: %d lightInfo[%d、%d、%d、%d]",reValue,locLightInfo.autoLight,locLightInfo.buttonState,
													 locLightInfo.lightness,locLightInfo.timeMode);

	if(reValue!=ESP_OK)
	{
		ESP_ERROR_CHECK( nvs_set_blob( handle, LIGHT_DATA, &lightInfo, Len) );

    	ESP_ERROR_CHECK( nvs_commit(handle) );
	
    	nvs_close(handle);
	}else{
		ESP_ERROR_CHECK( nvs_set_blob( handle, LIGHT_DATA, &locLightInfo, Len) );
		nvs_close(handle);
	}
    
}

void myNVS_read(LIGHT_INFO_S *lightInfo,uint8_t len)
{
	nvs_handle handle;
	BaseType_t reValue=0;
	size_t locLen = len;

	if(xSemaphore!=NULL)
		reValue = xSemaphoreTake( xSemaphore, portMAX_DELAY );
	
    ESP_ERROR_CHECK( nvs_open( LIGHT_NAMESPACE, NVS_READWRITE, &handle) );
   
    ESP_ERROR_CHECK( nvs_get_blob( handle, LIGHT_DATA, lightInfo,&locLen));
/*
	LOG(TAG,">>>>>>>>>>>>>lightInfo[%d、%d、%d、%d、%s]",lightInfo->autoLight,lightInfo->buttonState,
													 lightInfo->lightness,lightInfo->timeMode,
													 lightInfo->version);*/
    nvs_close(handle);
	if(reValue)
		xSemaphoreGive(xSemaphore);
}

void myNVS_write(LIGHT_INFO_S lightInfo)
{
	nvs_handle handle;
	BaseType_t reValue=0;
	size_t len ;

	len = sizeof(lightInfo);

	if(xSemaphore!=NULL)
		reValue = xSemaphoreTake( xSemaphore, portMAX_DELAY );
	
    ESP_ERROR_CHECK( nvs_open( LIGHT_NAMESPACE, NVS_READWRITE, &handle) );
   
    ESP_ERROR_CHECK( nvs_set_blob( handle, LIGHT_DATA, &lightInfo, len));

    ESP_ERROR_CHECK( nvs_commit(handle) );
	
    nvs_close(handle);
	if(reValue)
		xSemaphoreGive(xSemaphore);
}

static void MemoryServiceActive(ServiceVar_t *self)
{
	LIGHT_INFO_S lightInfo={false,false,125,TIME_MODE_NULL,"ver1.0.0"}; // 第一次上电默认值
	
	myNVS_init(lightInfo);
	myNVS_read(&lightInfo,sizeof(lightInfo));
	rtc_set_default_time();
	xSemaphore = xSemaphoreCreateMutex();
}

static void MemoryServiceDeactive(ServiceVar_t *self)
{
	LOG(TAG,"MemoryServiceDeactive");
	xSemaphoreTake(xSemaphore, portMAX_DELAY);
	xSemaphoreGive(xSemaphore);
	vSemaphoreDelete(xSemaphore);
}


ServiceVar_t *MemoryServiceCreate()
{
	LOG(TAG, "MemoryServiceCreate");
	ServiceVar_t *memory = (ServiceVar_t *) calloc(1, sizeof(ServiceVar_t));
	memory->deviceEvtNotified = NULL;
	memory->serviceActive = MemoryServiceActive;
	memory->serviceDeactive = MemoryServiceDeactive;
	return memory;
}



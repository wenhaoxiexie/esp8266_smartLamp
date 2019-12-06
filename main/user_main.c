#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "lwip/ip_addr.h"
#include "esp_wifi.h"
#include "WifiManager.h"
#include "LedManager.h"
#include "SystemMemory.h"
#include "TouchManager.h"
#include "TouchService.h"
#include "log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "MqttService.h"
#include "board.h"

#define TAG		"MAIN"

#include "driver/adc.h"
#include "sys/time.h"
#include "unistd.h"
#include "LedCtrlBy74hc595.h"
#include "LedCtrlByTPC112S1.h"
#include "interface_rtc.h"
#include "stdio.h"

void app_main(void)
{
	LOG(TAG,"======================VERSION:%s======================",SOFTWARE_VERSION);

	esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );	

	DeviceVar_t* m_dev = DeviceManagerInit();

	ServiceVar_t* MemorySer =  MemoryServiceCreate();
	m_dev->addService(m_dev,MemorySer);
	
	ServiceVar_t* TouchSer =  TouchServiceCreate();
	m_dev->addService(m_dev,TouchSer);
	
	ServiceVar_t* Mqttser =  MqttServiceCreate();
	m_dev->addService(m_dev,Mqttser);
	
	ControllerVar_t * WifiCtrl = WifiManagerCreate();
	m_dev->addController(m_dev,WifiCtrl);
	
	ControllerVar_t * TouchCtrl = TouchManagerCreate();
	m_dev->addController(m_dev,TouchCtrl);
	
	ControllerVar_t * LedCtrl =  LedManagerCreate();
	m_dev->addController(m_dev,LedCtrl);

	// 运行服务链表中注册的所有函数
	m_dev->ServiceActive(m_dev);
	// 运行控制链表中注册的所有函数
	m_dev->ControllerActive(m_dev);
	
}



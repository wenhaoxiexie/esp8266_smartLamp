#include "WifiManager.h"
#include "log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "wifismartconfig.h"

#include "LedManager.h"


#define TAG		"WifiManager"
#define FACTORY_SSID "FACTORY_SL1501"
#define FACTORY_PASS "12345678"
#define WIFI_SSID	"MERCURY_30FF5E" 
#define WIFI_PASS	"smart-home" 
#define SMARTCONFIG_TIMEOUT_TICK   (60000 / portTICK_RATE_MS)
#define WIFI_MANAGER_SETTING        BIT0
#define WIFI_MANAGER_DISCONNECT     BIT1

extern EventGroupHandle_t g_sc_event_group;
bool smartConfigFlag=false;

QueueHandle_t xQueueWifi = NULL;
TaskHandle_t SmartConfigHandle = NULL;
static bool smartConfigLed=false;

static esp_err_t wifiEvtHandlerCb(void* ctx, system_event_t* evt)
{
    WifiState g_WifiState;
    static char lastState = -1;
    if (evt == NULL) {
        return ESP_FAIL;
    }
    switch (evt->event_id) {
        case SYSTEM_EVENT_STA_START:
            LOG(TAG, "+WIFI:STA_START");
            g_WifiState = WifiState_Connecting;
            xQueueSend(xQueueWifi, &g_WifiState, 0);
			if(smartConfigLed)
				wifi_led_control(LED_SLOW_BLINK);
            break;

        case SYSTEM_EVENT_STA_STOP:
            LOG(TAG, "+WIFI:STA_STOP");
            break;

        case SYSTEM_EVENT_STA_CONNECTED:
            LOG(TAG, "+JAP:WIFICONNECTED");
            g_WifiState = WifiState_Connected;
            xQueueSend(xQueueWifi, &g_WifiState, 0);
			wifi_led_control(LED_FAST_BLINK);
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            LOG(TAG, "+JAP:DISCONNECTED,%u,%u", 0, evt->event_info.disconnected.reason);
            if (lastState != SYSTEM_EVENT_STA_DISCONNECTED) {
                g_WifiState = WifiState_Disconnected;
				if (lastState == SYSTEM_EVENT_STA_GOT_IP) {
                	xQueueSend(xQueueWifi, &g_WifiState, 0);
            	}
            }
            lastState = SYSTEM_EVENT_STA_DISCONNECTED;
			wifi_led_control(LED_FAST_FAST_BLINK);
			ESP_ERROR_CHECK( esp_wifi_connect() );
            break;

        case SYSTEM_EVENT_STA_GOT_IP: {
            LOG(TAG, "SYSTEM_EVENT_STA_GOTIP");
            if (lastState != SYSTEM_EVENT_STA_GOT_IP) {
                g_WifiState = WifiState_GotIp;
                xQueueSend(xQueueWifi, &g_WifiState, 0);
            }
            lastState = SYSTEM_EVENT_STA_GOT_IP;
			wifi_led_control(LED_OFF);
            break;
       	}
		case SYSTEM_EVENT_SCAN_DONE:{
				uint16_t apCount;
				esp_wifi_scan_get_ap_num(&apCount);
				
				LOG(TAG,">>>>>>>>>>>>> scan_found: %d ",apCount);
				if(apCount)
				{
					wifi_ap_record_t *list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount);
					ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&apCount, list));
					uint16_t i;
					for (i=0; i<apCount; i++) 
					{
						printf("%26.26s    |	% 4d \r\n",list[i].ssid, list[i].rssi);
						if(!strcmp((const char*)list[i].ssid,FACTORY_SSID))
						{
							wifi_config_t w_config;
							
							LOG(TAG,">>>>>>>>> ITEM SSID FOUND");
							
							ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &w_config));

							strcpy((char*)w_config.sta.ssid,FACTORY_SSID);
							strcpy((char*)w_config.sta.password,FACTORY_PASS);
							LOG(TAG,">>>>>> SSID:%s",w_config.sta.ssid);
							LOG(TAG,">>>>>> password:%s",w_config.sta.password);
							ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &w_config));
							
							ESP_ERROR_CHECK(esp_wifi_connect());
							ESP_ERROR_CHECK(esp_smartconfig_fast_mode(true));
							i= apCount;
						}
					}
					
					free(list);
					list =NULL;
				}
				break;
		}
    default:
        break;
    }
    return ESP_OK;
}

static void ProcessWifiEvent(DeviceVar_t* device, void *evt)
{
    WifiState state = *((WifiState *) evt);
    DeviceNotifyMsg msg = DEVICE_NOTIFY_WIFI_UNDEFINED;
    switch (state) {
    case WifiState_GotIp:
        msg = DEVICE_NOTIFY_WIFI_GOT_IP;
        break;
    case WifiState_Disconnected:
        msg = DEVICE_NOTIFY_WIFI_DISCONNECTED;
        break;
    case WifiState_SC_Disconnected:
        msg = DEVICE_NOTIFY_WIFI_SC_DISCONNECTED;
        break;
    case WifiState_BLE_Disconnected:
        msg = DEVICE_NOTIFY_WIFI_BLE_DISCONNECTED;
        break;
    case WifiState_Config_Timeout:
        msg = DEVICE_NOTIFY_WIFI_SETTING_TIMEOUT;
        break;
    case WifiState_BLE_Stop:
        break;

    case WifiState_Connecting:
    case WifiState_Connected:
    case WifiState_ConnectFailed:
    default:
        break;
    }
	if (msg != DEVICE_NOTIFY_WIFI_UNDEFINED) {

		  // 运行一遍当前所有已经注册的认证函数
		  device->NotifyServices(device,WIFI_EVENT, &msg, sizeof(DeviceNotifyMsg));
	}
}


static void sc_callback(smartconfig_status_t status, void *pdata)
{
    switch (status) {
        case SC_STATUS_WAIT:
            LOG(TAG, "SC_STATUS_WAIT");
            break;
        case SC_STATUS_FIND_CHANNEL:
            LOG(TAG, "SC_STATUS_FINDING_CHANNEL");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            LOG(TAG, "SC_STATUS_GETTING_SSID_PSWD");
			
            break;
        case SC_STATUS_LINK:
            LOG(TAG, "SC_STATUS_LINK");
			
            wifi_config_t *wifi_config = pdata;
            LOG(TAG, "SSID:%s", wifi_config->sta.ssid);
            LOG(TAG, "PASSWORD:%s", wifi_config->sta.password);
            ESP_ERROR_CHECK( esp_wifi_disconnect() );
            ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config) );
            ESP_ERROR_CHECK( esp_wifi_connect() );
            break;
        case SC_STATUS_LINK_OVER:
            LOG(TAG, "SC_STATUS_LINK_OVER");
			
            if (pdata != NULL) {
                uint8_t phone_ip[4] = { 0 };
                memcpy(phone_ip, (uint8_t* )pdata, 4);
                LOG(TAG, "Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);
            }
			
            break;
        default:
            break;
    }
}


void WifiStartUp(void)
{
  	wifi_config_t w_config;

	tcpip_adapter_init();
	
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	
    ESP_ERROR_CHECK(esp_event_loop_init(wifiEvtHandlerCb, NULL));
	
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	
	ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &w_config));

	ESP_ERROR_CHECK(esp_wifi_start());
    
	ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS));

	if(w_config.sta.ssid[0] != 0){
		LOG(TAG, ">>>>>>>>>>Wi-Fi SSID:%s", w_config.sta.ssid);
		ESP_ERROR_CHECK(esp_wifi_connect());
		ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &w_config));
		ESP_ERROR_CHECK(esp_smartconfig_fast_mode(true));
	}else{
		LOG(TAG, "*************** Wi-Fi SSID ***********");
		//smartConfigFlag = true;
		//ESP_ERROR_CHECK(esp_smartconfig_start(sc_callback));
		smartConfigLed=false;
		wifi_scan_config_t config = { 
									  .ssid = NULL,			
 									  .bssid = NULL,
 									  .show_hidden=true,
 									  .channel = 0
								    };
		esp_wifi_scan_start(&config,1);		
	}
	
}

void wifiConfig()
{
	wifi_mode_t mode;

	esp_wifi_restore();
	
	esp_wifi_stop();
	
	esp_smartconfig_stop();

	esp_wifi_get_mode(&mode);
	if(mode!=WIFI_MODE_STA)
		esp_wifi_set_mode(WIFI_MODE_STA);
	
	esp_wifi_start();
	esp_smartconfig_start(sc_callback);
	smartConfigFlag = true;
	smartConfigLed = true;
}

static void WifiManagerEvtTask(void* device)
{
    WifiState state = WifiState_Unknow;	
    while (1) {
        if (xQueueReceive(xQueueWifi, &state, portMAX_DELAY)) {
            ProcessWifiEvent((DeviceVar_t*)device, &state);
        }
    }
    vTaskDelete(NULL);
}


static int WifiManagerActive(DeviceVar_t* device)
{
    LOG(TAG, "WifiManagerActive");
    xQueueWifi = xQueueCreate(4, sizeof(WifiState));
    configASSERT(xQueueWifi);
    WifiStartUp();
    if (xTaskCreate(WifiManagerEvtTask,"WifiManagerEvtTask",(512),device,3,NULL) != pdPASS) {
        LOG(TAG, "Error create WifiManagerTask");
        return -1;
    }
    return 0;
}

static int WifiManagerDeactive(DeviceVar_t* device)
{
    LOG(TAG, "WifiManagerDeactive");
	return 0;
}

ControllerVar_t *WifiManagerCreate()
{
    LOG(TAG, "WifiManagerCreate");
    ControllerVar_t *m_wifi = (ControllerVar_t *) calloc(1, sizeof(ControllerVar_t));
    m_wifi->ControllerActive = WifiManagerActive;
    m_wifi->ControllerDeactive = WifiManagerDeactive;
    return m_wifi;
}



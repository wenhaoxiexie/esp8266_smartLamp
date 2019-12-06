#include "log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "cJSON.h"
#include "OtaService.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "LedManager.h"
#include "MqttService.h"
#include "SystemMemory.h"
#include "unistd.h"

#define TAG                   "OTAService"
extern char ota_topic[50];
extern char event_topic[50];

static OTA_SER_VAR ota_ser_var = {0};
static const esp_partition_t *update_partition = NULL;
static esp_ota_handle_t update_handle = 0 ; 
static uint8_t percentTemp=0;

static void ReportOtaBegain(esp_mqtt_client_handle_t client,char* topic)
{
	ReportDevEvent(client,topic,"ota_start","ok");
}

static void ReportOtaFail(esp_mqtt_client_handle_t client,char* topic)
{
	ReportDevEvent(client,topic,"ota_update","failed");
}

static void ReportOtaSuccess(esp_mqtt_client_handle_t client,char* topic)
{
	ReportDevEvent(client,topic,"ota_update","succeed");
}

static void ReportOtaPercent(esp_mqtt_client_handle_t client,int num)
{
	char str[8];

	itoa(num,str,10);
	ReportDevEvent(client,event_topic,"percent",str);
}

void OtaProcess(esp_mqtt_event_handle_t event)
{
	LIGHT_INFO_S lightInfo;
	char topic_left[10] = {0};
	char* data = event->data;
	cJSON *payload_json = NULL;
	cJSON *value = NULL;
	esp_err_t err = ESP_FAIL;
	int len = strlen(ota_topic)+1;
	
	memcpy(topic_left,event->topic+len,event->topic_len -len);

	LOG(TAG,">>>>>>>>>>topic_left: %s",topic_left);

	if(!strcmp("begin",topic_left) && (ota_ser_var.ota_sta == OTA_IDLE)){
		LOG(TAG,"ota service begin");
		wifi_led_control(LED_ON);
		payload_json = cJSON_Parse(data);
		if(NULL != payload_json){
			value = cJSON_GetObjectItem(payload_json, "version");		
			if(NULL != value){
				strcpy(ota_ser_var.pack_info.version,value->valuestring); 
			}
			value = cJSON_GetObjectItem(payload_json, "firmware_size");
			if(NULL != value){
				ota_ser_var.pack_info.firmware_size = atoi(value->valuestring);
				LOG(TAG,">>>>>>>>>>>>>>valueint: %ld",ota_ser_var.pack_info.firmware_size);
			}
			
			cJSON_Delete(payload_json);
			
			update_partition = esp_ota_get_next_update_partition(NULL);
			if (update_partition != NULL) {
				//LOG(TAG, "OTA Unpack OPS: Writing to partition subtype %d at offset 0x%x",
				//update_partition->subtype, update_partition->address);
				err = esp_ota_begin(update_partition, ota_ser_var.pack_info.firmware_size, &update_handle);
				if (err != ESP_OK) {
					LOG(TAG, "Init OTA failed, error = %d", err);
					ReportOtaFail(event->client,event_topic);
				}else{
					ota_ser_var.ota_sta = OTA_RUNING;
					ReportOtaBegain(event->client,event_topic);
				}
			} else {
				LOG(TAG, "OTA Unpack OPS: Get update partition failed");
				ReportOtaFail(event->client,event_topic);
			}
		}
	}else if(!strcmp("bin",topic_left)&& (ota_ser_var.ota_sta == OTA_RUNING)){
		LOG(TAG,"ota service recive bin %d",ota_ser_var.rec_len);
		ota_ser_var.rec_len += event->data_len;
		if(ota_ser_var.rec_len > ota_ser_var.pack_info.firmware_size){
			LOG(TAG, "ota recive data over size recv_len:%ld firmware size:%ld",(long)ota_ser_var.rec_len,(long)ota_ser_var.pack_info.firmware_size);
			err = esp_ota_end(update_handle);
		}else{
			err = esp_ota_write(update_handle,event->data, event->data_len);
			if (err != ESP_OK) {
				err = esp_ota_end(update_handle);
				LOG(TAG, "OTA Unpack OPS: Write OTA data failed! err: 0x%x", err);
				ReportOtaFail(event->client,event_topic);
			}else{
				ota_ser_var.rec_percent = (uint8_t)(((float)ota_ser_var.rec_len/ota_ser_var.pack_info.firmware_size)*100);
				LOG(TAG, ">>>>>>>>>> OTA percent:%d", ota_ser_var.rec_percent);
				if((ota_ser_var.rec_percent%10==0)&&(ota_ser_var.rec_percent!=percentTemp))
				{
					percentTemp = ota_ser_var.rec_percent;
					ReportOtaPercent(event->client,ota_ser_var.rec_percent);
				}
			}

			if(ota_ser_var.rec_len==ota_ser_var.pack_info.firmware_size)
			{
				LOG(TAG,"ota service end");
				
				usleep(1000000);
				err = esp_ota_end(update_handle);
				ota_ser_var.ota_sta = OTA_IDLE;
				if (err != ESP_OK) {
					LOG(TAG, "OTA Unpack OPS: End OTA failed! err: %d", err);
			    }else{
					err = esp_ota_set_boot_partition(update_partition);
					if (err != ESP_OK) {
						LOG(TAG, "OTA Unpack OPS: Set boot partition failed! err = 0x%x", err);
					}else{
						LOG(TAG, "OTA Successful prepare to restart");
						ReportOtaSuccess(event->client,event_topic);
						myNVS_read(&lightInfo,sizeof(lightInfo));
						strcpy(lightInfo.version,ota_ser_var.pack_info.version);
						LOG(TAG,">>>>>>>>>>>>>>>lightInfo.version:%s",lightInfo.version);
						myNVS_write(lightInfo);
					   	esp_restart();
						wifi_led_control(LED_OFF);
					}	
			  	}
			}
		}
	}
}

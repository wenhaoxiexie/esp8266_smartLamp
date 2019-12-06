#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "MQTTS_EXAMPLE";

static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;

#define CA_CER "-----BEGIN CERTIFICATE-----MIIB2zCCAUQCCQCFcDDdS/oBEjANBgkqhkiG9w0BAQUFADAyMTAwLgYDVQQKEydUTFMgUHJvamVjdCBEb2RneSBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkwHhcNMTkwNTE2MDkwNTMyWhcNMzMwMTIyMDkwNTMyWjAyMTAwLgYDVQQKEydUTFMgUHJvamVjdCBEb2RneSBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkwgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBAJsDBJzwL+VdD5p6d1N5iBkS9JSjuIDyKkxTXGwKgVQs3UcDCTxjIMkSBfJA92JnCX7JlM6Gg7yaMrt5q4nicWnEJUzqkDRX4DHzveSLHmNPNgu6aisMlEX5LcMUN2Qz1++GfH+I7DKgF/Mg1+oSD/AZGGSNq1IBCOFoEPh4kYgrAgMBAAEwDQYJKoZIhvcNAQEFBQADgYEAF2WVHxUQkC1IGLbWoKvHV98AF94FT8JXKoQSn4WYhDHWRc5xwItg+zdefy/+iAuYfAGZflm7AZ5cxzvUczDymqnuLrSwiyg/BcwMfGN/HCEV0eRiReF3s549mFlVASSc+8GMuWhQ82OEh1hTSxueWosSI43CVZjvClInzsZvkXE=-----END CERTIFICATE-----"
#define CLIENT_CRT "-----BEGIN CERTIFICATE-----MIIB5zCCAVACCQCO8vELrdJu/jANBgkqhkiG9w0BAQUFADAyMTAwLgYDVQQKEydUTFMgUHJvamVjdCBEb2RneSBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkwHhcNMTkwNTE2MDkwNTMzWhcNMzMwMTIyMDkwNTMzWjA+MScwJQYDVQQKEx5UTFMgUHJvamVjdCBEZXZpY2UgQ2VydGlmaWNhdGUxEzARBgNVBAMTCmloLmdpZWMuY24wgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBAL7op1bmCU5m5VAeZx/aA5qSoYANq8yx6bfcQrLTfsblkV2hd9lWvdaldNqWhK0E8OyVNfleqmteixX0SlMa5/IbHfD1W1jFnH1CgvaGyeu6dVHVwP3X2RUvBcWK70S7LQl8K7HZJwubfpnaZvV0n2LfJmu2fq7hmUmiJx80jTztAgMBAAEwDQYJKoZIhvcNAQEFBQADgYEAljCJ0y99KnVF+npQZX7pzsjixlzhy/SBy24YQW/n+rWueIMHYXWi5QWa8tWcHGou3vUaZbgKhb+F8QgL+9ZDdr13tDaGc61WMWEIt6OmDKc+8LjVE0uaGRgzTVsMxEB//yRE0JN6DbsSCRsTelLr8QIB4WnYgpTP71plmO3QUwU=-----END CERTIFICATE-----"
#define CLIENT_KEY "-----BEGIN RSA PRIVATE KEY-----MIICXQIBAAKBgQC+6KdW5glOZuVQHmcf2gOakqGADavMsem33EKy037G5ZFdoXfZVr3WpXTaloStBPDslTX5XqprXosV9EpTGufyGx3w9VtYxZx9QoL2hsnrunVR1cD919kVLwXFiu9Euy0JfCux2ScLm36Z2mb1dJ9i3yZrtn6u4ZlJoicfNI087QIDAQABAoGAHMTKktnPhTUUUWKDf9VGvcBi/f0RaqNU6RQUKQaeEDMAGPAAM4xSx0nftiEAlWItPDmwDIgrfkqdAw3xNVzUnvAnByzEQqZM0d0dy9A1pxIOPAl5BE1bmolLTuWva00niQA2OB1qQPAg2h9NkwtYwQxuTgHuovns91XAgXfFfTECQQD8hVJKYylBCXm6vg/Cl7QlsYahLehkj8eZbe19PxktOfg4v+T5KyP1nVj3ez9vT5Aae8Z3ZFTPWGHo0bUqxTnfAkEAwYoEfhaUphQZJoVRaDBekxJv3aiauoVW/xjtfZvSeT4Ue/M0kfNyAbLziGk2q7uK4iFNaycU5fsyGY7gVh76swJBALS7z0wCcPJlj8SCZ9FEtuELkGon616eMaO0s/eig6iFBw3G+mED6XbPqW9nvN00OtVZpZCqNLE6dskl3t6/zk0CQEkKKknfgbfvq9IyzBcy8e41LgyMHeC4g62AHdiintrsx9RDY5qVMnhbrbbUZjKHc5GFme8Pb76ffzfQCO/XfysCQQDD/mwWKjzy5JfCtptYZVetygfTeMtQhtOWbJYuk8YzZDgnBatHq7EVxyktP9J1teC61EMgJ/TW4lS63WvbYpu7-----END RSA PRIVATE KEY-----"

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);

            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void wifi_init(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_LOGI(TAG, "start the WIFI SSID:[%s]", CONFIG_WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Waiting for wifi");
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}

extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

static void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        //.uri = "mqtts://test.mosquitto.org:8884",
        .uri = "mqtts://192.168.1.102",
        .port = 1883,
        .event_handle = mqtt_event_handler,
        .client_cert_pem = (const char *)client_cert_pem_start,
        .client_key_pem = (const char *)client_key_pem_start,
#if 0
       .uri = "mqtts://192.168.1.102",
       .port = 1883,
       .cert_pem = (const char*)CA_CER,
       .client_cert_pem = (const char*)CLIENT_CRT,
       .client_key_pem = (const char*)CLIENT_KEY,
#endif 
    };

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}

void app_main()
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    nvs_flash_init();
    wifi_init();
    mqtt_app_start();

}

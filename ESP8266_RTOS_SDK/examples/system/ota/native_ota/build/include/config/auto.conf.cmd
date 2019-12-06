deps_config := \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/app_update/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/aws_iot/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/console/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/esp8266/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/esp_http_client/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/esp_http_server/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/freertos/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/libsodium/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/log/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/lwip/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/mdns/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/mqtt/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/newlib/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/pthread/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/spiffs/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/ssl/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/tcpip_adapter/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/util/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/vfs/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/wifi_provisioning/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/wpa_supplicant/Kconfig \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/bootloader/Kconfig.projbuild \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/esptool_py/Kconfig.projbuild \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/examples/system/ota/native_ota/main/Kconfig.projbuild \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/components/partition_table/Kconfig.projbuild \
	/mnt/hgfs/work_code/esp/esp8266/esp8266_iot_platform/ESP8266_RTOS_SDK/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;

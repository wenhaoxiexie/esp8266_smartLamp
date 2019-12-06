#ifndef __WIFI_MANAGER_H__
#define __WIFI_MANAGER_H__

#include "DeviceManager.h"
#include "WifiManager.h"
#include <stdbool.h>
 

 typedef enum{
	 DEVICE_NOTIFY_WIFI_UNDEFINED = 0,
	 DEVICE_NOTIFY_WIFI_GOT_IP,
	 DEVICE_NOTIFY_WIFI_DISCONNECTED,
	 DEVICE_NOTIFY_WIFI_SC_DISCONNECTED,
	 DEVICE_NOTIFY_WIFI_BLE_DISCONNECTED,
	 DEVICE_NOTIFY_WIFI_SETTING_TIMEOUT,
 }WIFI_EVENT_T;

 typedef enum {
	 SMARTCONFIG_NO,
	 SMARTCONFIG_YES
 } SmartconfigStatus;
 
 typedef enum {
	 WifiState_Unknow,
	 WifiState_Config_Timeout,
	 WifiState_Connecting,
	 WifiState_Connected,
	 WifiState_Disconnected,
	 WifiState_ConnectFailed,
	 WifiState_GotIp,
	 WifiState_SC_Disconnected,//smart config Disconnected
	 WifiState_BLE_Disconnected,
	 WifiState_BLE_Stop,
 } WifiState;


typedef enum{
	WIFI_IDLE = 0,
	WIFI_SOFTAP,
	WIFI_STA,
}WIFI_MODE_T;


typedef enum{
	SOFTAP = 0,
	SMARTCONFIG,
	AIRKISS,
	SIMPLECONFIG,
}WIFI_CONFIG_METHOD_T;


 typedef enum{
	 WIFI_CONNECT = 0,
	 WIFI_DISCONNECT,
	 WIFI_CONFIG,
	 WIFI_MODE_SWITCH,
 }WIFI_CMD_T;


typedef struct{
	bool connected;
	WIFI_MODE_T mode;
	char local_ip[16];
	char local_mac[18];
}WIFI_MANAGER_INFO;

 ControllerVar_t *WifiManagerCreate();

 void WifiStartUp();
 void wifiConfig(); 
 #endif


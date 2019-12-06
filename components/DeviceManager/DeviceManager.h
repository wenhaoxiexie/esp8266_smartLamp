#ifndef _DEVICE_MANAGER_H_
#define _DEVICE_MANAGER_H_
#include "DeviceCommon.h"

// 单项函数链表
typedef struct ServiceVar_t{
	struct ServiceVar_t* next;
	void (*serviceActive)(struct ServiceVar_t *self);
    void (*serviceDeactive)(struct ServiceVar_t *self);
    void (*deviceEvtNotified)(struct DeviceNotification* evt);
}ServiceVar_t;

typedef struct DeviceVar_t DeviceVar_t;

// 单项函数链表
typedef struct ControllerVar_t{
	struct ControllerVar_t* next;
	int (*ControllerActive)(struct DeviceVar_t* device);
    int (*ControllerDeactive)(struct DeviceVar_t* device);
}ControllerVar_t;

// 初始化函数
typedef struct DeviceVar_t{
	ServiceVar_t* ser_head;
	ControllerVar_t* ctrl_head;
    void (*addService)(struct DeviceVar_t* device,ServiceVar_t* service);
	void (*addController)(struct DeviceVar_t* device,ControllerVar_t* dev_ctrl);
	void (*ServiceActive)(struct DeviceVar_t* device);
    void (*serviceDeactive)(struct DeviceVar_t* device);
	void (*ControllerActive)(struct DeviceVar_t* device);
    void (*ControllerDeactive)(struct DeviceVar_t* device);
	void (*NotifyServices)(struct DeviceVar_t* device,EVENT_T type, void *data, int len);
}DeviceVar_t;

// 公共函数
void NotifyServices(DeviceVar_t* device,EVENT_T type, void *data, int len);		 

DeviceVar_t* DeviceManagerInit();

#endif


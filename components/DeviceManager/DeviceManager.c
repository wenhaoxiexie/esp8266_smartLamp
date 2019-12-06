#include "DeviceManager.h"
#include "log.h"

#define 	TAG 	"DeviceManager"

static void AddService(DeviceVar_t* device,ServiceVar_t* service)
{
	if (!device) {
		return;
	}
	ServiceVar_t** ser = NULL;
	ser = &(device->ser_head);
	while (*ser != NULL) {
		ser = &((*ser)->next);
	}
	*ser = service;
}


static void AddController(DeviceVar_t* device,ControllerVar_t* dev_ctrl)
{
	if (!device) {
		return;
	}
	ControllerVar_t** ctrl = NULL;
	ctrl = &(device->ctrl_head);
	while (*ctrl != NULL) {
		ctrl = &((*ctrl)->next);
	}
	*ctrl = dev_ctrl;
}


static void ServiceActive(DeviceVar_t* device)
{
	if (!device) {
		return;
	}
	ServiceVar_t* ser = NULL;
	ser = device->ser_head;
	while (ser != NULL) {
		ser->serviceActive(ser);
		ser = ser->next;
	}
}


static void serviceDeactive(DeviceVar_t* device)
{
	if (!device) {
		return;
	}
	ServiceVar_t* ser = NULL;
	ser = device->ser_head;
	while (ser != NULL) {
		ser->serviceDeactive(ser);
		ser = ser->next;
	}
}


static void ControllerActive(DeviceVar_t* device)
{
	if (!device) {
    	return;
    }
    ControllerVar_t* ctrl = NULL;
	ctrl = device->ctrl_head;
	while (ctrl != NULL) {
		ctrl->ControllerActive(device);
		ctrl = ctrl->next;
	}
}


static void ControllerDeactive(DeviceVar_t* device)
{
	if (!device) {
    	return;
    }
    ControllerVar_t* ctrl = NULL;
	ctrl = device->ctrl_head;
	while (ctrl != NULL) {
		ctrl->ControllerDeactive(device);
		ctrl = ctrl->next;
	}
}


void NotifyServices(DeviceVar_t* device,EVENT_T type, void *data, int len)
{
	DeviceNotification note;
	ServiceVar_t* service = NULL;
	memset(&note, 0x00, sizeof(DeviceNotification));
	note.type = type;
	note.data = data;
	note.len = len;
	service = device->ser_head; // 从第一个service 开始运行
	
	while (service != NULL) {
		if (service->deviceEvtNotified) {
			service->deviceEvtNotified(&note);
		}
		service = service->next;
		
	}
}
		 

DeviceVar_t* DeviceManagerInit()
{
    LOG(TAG, "DeviceManagerInit");
    DeviceVar_t *m_Device = (DeviceVar_t *) calloc(1, sizeof(DeviceVar_t));
	m_Device->addService = AddService;
	m_Device->ServiceActive = ServiceActive;
	m_Device->serviceDeactive = serviceDeactive;
	m_Device->addController = AddController;
	m_Device->ControllerActive = ControllerActive;
	m_Device->ControllerDeactive = ControllerDeactive;
	m_Device->NotifyServices = NotifyServices;
    return m_Device;
}



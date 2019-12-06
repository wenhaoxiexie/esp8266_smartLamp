#ifndef _APT8L08_BUTTON_H_
#define _APT8L08_BUTTON_H_

#include "esp_types.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

#define APT8L08_ADDR 		 	0xAC

/* APT8L08register */
#define R_APT8L08_SYSCON      	0x3A
#define R_APT8L08_MCON   	 	0x21
#define R_APT8L08_KDR0    		0x23
#define R_APT8L08_GSR			0x20
#define R_APT8L08_KVR0			0x34


int APT8L08Init();

int APT8L08GetKeyValue(uint8_t* key);

#endif

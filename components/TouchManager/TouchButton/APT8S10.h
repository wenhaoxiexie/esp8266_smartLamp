#ifndef _APT8S10_H_
#define _APT8S10_H_

#include "esp_types.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

#define	APT8S10_ADDR	0xAE

int APT8S10Init();

int APT8S10GetKeyValue(uint8_t* key);

int APT8S10GetWheelValue(uint8_t* value);

#endif

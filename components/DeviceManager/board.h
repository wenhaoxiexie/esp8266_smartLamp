#ifndef __BOARD_H__
#define __BOARD_H__
#include "sdkconfig.h"

#define DEV_PID				"lamp"
#define DEV_DID				"A001"
#define SOFTWARE_VERSION	"SMART_LAMP_2019_5_25"


/***************************************GPIO SETUP*********************************************************/

#define GPIO_TOUCH_INTR		GPIO_NUM_MAX

#define TOUCH_INTR_PIN_SEL  (1ULL<<GPIO_TOUCH_INTR)

#define TOUCH_IIC_DATA		GPIO_NUM_2

#define TOUCH_IIC_CLK		GPIO_NUM_14

#define GPIO_74HC595_SCLK	GPIO_NUM_16

#define GPIO_74HC595_RCLK	GPIO_NUM_5

#define GPIO_74HC595_DATA	GPIO_NUM_4

#define GPIO_TPC112S1_SCLK	GPIO_NUM_13

#define GPIO_TPC112S1_DIN	GPIO_NUM_12

#define GPIO_TPC112S1_SYNC	GPIO_NUM_0

#endif


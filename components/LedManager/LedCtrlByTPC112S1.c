#include "log.h"
#include "LedCtrlByTPC112S1.h"
#include "board.h"
#include "driver/gpio.h"
#include "SystemMemory.h"

#include <unistd.h>


#define LIGHT_MAX 3000
#define LIGHT_MIN 1500

#define TAG		"TPC112S1"

#define NOP() 	asm("nop")

//delay 5us
void TPCDelay()
{
	for(uint8_t i = 0;i < 10;i++){
		NOP();
	}
}

void TPCSendData(uint16_t SendVal)
{
	gpio_set_level(GPIO_TPC112S1_SYNC,0);
	
	TPCDelay();
    for(uint8_t i = 0;i<16;i++)
    {
        if((SendVal<<i)&0x8000){
			gpio_set_level(GPIO_TPC112S1_DIN,1);
		}else{
			gpio_set_level(GPIO_TPC112S1_DIN,0);
		}
        gpio_set_level(GPIO_TPC112S1_SCLK,1);
        TPCDelay();
        gpio_set_level(GPIO_TPC112S1_SCLK,0);
    }
	TPCDelay();
	gpio_set_level(GPIO_TPC112S1_SYNC,1);
}


void TPCInit(void)
{
	// Ñ¡ÔñGPIO¹¦ÄÜ
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U,FUNC_GPIO13);

	gpio_set_direction(GPIO_TPC112S1_SCLK,GPIO_MODE_OUTPUT);	
	gpio_set_direction(GPIO_TPC112S1_DIN,GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_TPC112S1_SYNC,GPIO_MODE_OUTPUT);	

	gpio_set_level(GPIO_TPC112S1_SCLK,1);
	gpio_set_level(GPIO_TPC112S1_DIN,0);
	
	gpio_set_level(GPIO_TPC112S1_SYNC,1);
}

void TPCCtrlLed(uint8_t level)
{
	uint16_t SendVal;
	SendVal = LIGHT_CONVER_TOOLS(level);
	
	TPCSendData(SendVal);
}

static void led_breathe_exe(uint16_t defMem,uint16_t defMin,uint16_t max)
{
	uint16_t index;
	
	for(index=defMem;index<max;index++)
	{
		TPCSendData(index);
		usleep(800);
	}
	for(index=max;index>defMin;index--)
	{
		TPCSendData(index);
		usleep(800);
	}
	for(index=defMin;index<defMem;index++)
	{
		TPCSendData(index);
		usleep(800);
	}
}

void TPCCtrlLedBreathe(uint8_t count)
{
	uint16_t defMem;
	LIGHT_INFO_S lightInfo;

	myNVS_read(&lightInfo,sizeof(lightInfo));
	defMem = LIGHT_CONVER_TOOLS(lightInfo.lightness);
	led_breathe_exe(defMem,LIGHT_MIN,LIGHT_MAX);
}



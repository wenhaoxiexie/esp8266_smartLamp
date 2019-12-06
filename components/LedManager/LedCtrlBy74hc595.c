#include "log.h"
#include "LedCtrlBy74hc595.h"
#include "board.h"
#include "driver/gpio.h"

#define TAG		"74HC595"

#define NOP() 	asm("nop")
/*
	0:power_button
	1:auto_button
	2:mode_button
*/
uint32_t key_led_map[3]={0x10000000,0x08000000,0x20000000};

//delay 5us
static void HCDelay()
{
	for(uint8_t i = 0;i < 10;i++){
		NOP();
	}
}

void HC595SendData(uint32_t SendVal)
{
    for(uint8_t i = 0;i<32;i++)
    {
        if((SendVal<<i)&0x80000000){
			gpio_set_level(GPIO_74HC595_DATA,1);			
		}else{
			gpio_set_level(GPIO_74HC595_DATA,0);
		}
        gpio_set_level(GPIO_74HC595_SCLK,0);
        HCDelay();
        gpio_set_level(GPIO_74HC595_SCLK,1);
    }
	gpio_set_level(GPIO_74HC595_RCLK,0);
	HCDelay();
	gpio_set_level(GPIO_74HC595_RCLK,1);
}

void HC595Init()
{
	gpio_set_direction(GPIO_74HC595_DATA,GPIO_MODE_OUTPUT); 
	gpio_set_direction(GPIO_74HC595_SCLK,GPIO_MODE_OUTPUT);	
	gpio_set_direction(GPIO_74HC595_RCLK,GPIO_MODE_OUTPUT);	
	gpio_set_level(GPIO_74HC595_DATA,0);
	gpio_set_level(GPIO_74HC595_SCLK,0);
	gpio_set_level(GPIO_74HC595_RCLK,0);
}


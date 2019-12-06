#include <stdio.h>
#include "esp_log.h"
#include "driver/pwm.h"
#include "driver/gpio.h"
#include "LedCtrlBySe8324.h"
#include "SystemMemory.h"
#include <unistd.h>

// PWM period 500us(2Khz), same as depth
#define PWM_PERIOD    (40) // 25KHZ
#define LIGHT_LOOP_MAX      255

static const char *TAG = "LedCtrlBySe8324";

// pwm pin number
const uint32_t pin_num[1] = {
    GPIO_NUM_12
};

// duties table, real_duty = duties[x]/PERIOD
uint32_t duties[1] = {
    0
};

// phase table, delay = (phase[x]/360)*PERIOD
int16_t phase[1] = {
    0
};


static uint8_t light_duty_tools(uint8_t light)
{
	uint8_t Llight = LIGHT_LOOP_MAX - light;
	//ESP_LOGI(TAG, "Llight:%d \n", Llight);

	uint8_t duty = ((float)Llight/LIGHT_LOOP_MAX)*PWM_PERIOD;
	//ESP_LOGI(TAG, "duty:%d \n", duty);

	return duty;
}
	
void se8324_pwm_init(void)
{
	
	ESP_LOGI(TAG," PWM init... \n");
	pwm_init(PWM_PERIOD, duties, 1, pin_num);
    pwm_set_channel_invert(0x1 << 0);
    pwm_set_phases(phase);
}

void se8324_adjust_light(bool sel, uint8_t light)
{
	uint8_t duty = PWM_PERIOD;

	if(sel==false)
	{
		pwm_set_duty(0,duty);
		pwm_start();
	}else{
		if(light<15)
			light = 15;
		duty = light_duty_tools(light);
	
		ESP_LOGI(TAG, "duty:%d \n", duty);
		pwm_set_duty(0,duty);
		pwm_start();
	}
}

void se8324_breath_light(uint8_t count)
{
	uint16_t defMem;
	uint16_t index;
	LIGHT_INFO_S lightInfo;

	myNVS_read(&lightInfo,sizeof(lightInfo));
	defMem = lightInfo.lightness;

	while(count--)
	{
		for(index=defMem;index<LIGHT_LOOP_MAX;index++)
		{
			se8324_adjust_light(1,index);
			usleep(10);
		}
		
		for(index=LIGHT_LOOP_MAX-1;index>1;index--)
		{
			se8324_adjust_light(1,index);
			usleep(10);
		}
		
		for(index=2;index<=defMem;index++)
		{
			se8324_adjust_light(1,index);
			usleep(10);
		}
	}
}



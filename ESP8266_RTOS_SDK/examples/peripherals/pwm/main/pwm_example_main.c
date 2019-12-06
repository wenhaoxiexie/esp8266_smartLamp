/* pwm example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"

#include "driver/pwm.h"


#define PWM_0_OUT_IO_NUM   12
#define PWM_1_OUT_IO_NUM   13
#define PWM_2_OUT_IO_NUM   14
#define PWM_3_OUT_IO_NUM   15

// PWM period 500us(2Khz), same as depth
#define PWM_PERIOD    (30)

static const char *TAG = "pwm_example";

// pwm pin number
const uint32_t pin_num[1] = {
    PWM_0_OUT_IO_NUM/*,
    PWM_1_OUT_IO_NUM,
    PWM_2_OUT_IO_NUM,
    PWM_3_OUT_IO_NUM*/
};

// duties table, real_duty = duties[x]/PERIOD
uint32_t duties[1] = {
    0/*, 0, 0, 0,*/
};

// phase table, delay = (phase[x]/360)*PERIOD
int16_t phase[1] = {
    0/*, 0, 90, -90,*/
};

void app_main()
{
    pwm_init(PWM_PERIOD, duties, 1, pin_num);
    pwm_set_channel_invert(0x1 << 0);
    pwm_set_phases(phase);
    int16_t count = 0;
	uint32_t duty=0;
	bool flag = true;
	while (1) {
		#if 0
        if (count == 20) {
            // channel0, 1 output hight level.
            // channel2, 3 output low level.
            pwm_stop(0x3);
            ESP_LOGI(TAG, "PWM stop\n");
        } else if (count == 30) {
            pwm_start();
            ESP_LOGI(TAG, "PWM re-start\n");
            count = 0;
        }
		#endif
		ESP_LOGI(TAG, "duty:%d \n", duty);
		duty = count*1;

		pwm_set_duty(0,duty);
		pwm_start();
		if(flag)
		{
			
			count++;
		}else{
			count--;
		}
		
		if(count==34)
		{
			flag=false;
		}
		else if(count==0)
		{
			flag=true;
		}
		
		vTaskDelay(20 / portTICK_RATE_MS);
    }
}


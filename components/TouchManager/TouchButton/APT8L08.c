#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "sdkconfig.h"
#include "APT8L08.h"
#include "log.h"
#include "board.h"

#define TAG	"APT8L08"

typedef struct {
    i2c_port_t i2c_port_num;
    i2c_config_t i2c_cfg;
} APT8L08Config;

extern xQueueHandle toucn_btn_evt_queue;
static const APT8L08Config APT8L08Conf = {
    .i2c_port_num = I2C_NUM_0,
    .i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = TOUCH_IIC_DATA,
        .scl_io_num = TOUCH_IIC_CLK,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
    }
};


static int I2cInit(i2c_config_t *conf, int i2cMasterPort)
{
    int res;
    res = i2c_driver_install(i2cMasterPort, conf->mode);
    res |= i2c_param_config(i2cMasterPort, conf);
    return res;
}


static void IRAM_ATTR APT8L08_isr_handler(void* arg)
{
	static uint8_t gpio_num = GPIO_TOUCH_INTR;
    xQueueSendFromISR(toucn_btn_evt_queue, &gpio_num, NULL);
}


int APT8L08IntrInstall(void)
{
	gpio_config_t io_conf;
	esp_err_t err = ESP_OK;
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = TOUCH_INTR_PIN_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
	toucn_btn_evt_queue = xQueueCreate(5, sizeof(uint8_t));
    err = gpio_isr_handler_add(GPIO_TOUCH_INTR, APT8L08_isr_handler, NULL);
	return err;
}

static int APT8L08WriteReg(uint8_t regAdd, uint8_t data)
{
    int res = 0;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    res |= i2c_master_start(cmd);
    res |= i2c_master_write_byte(cmd, APT8L08_ADDR, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_write_byte(cmd, regAdd, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_write_byte(cmd, data, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_stop(cmd);
    res |= i2c_master_cmd_begin(APT8L08Conf.i2c_port_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return res;
}

static int APT8L08ReadReg(uint8_t regAdd, uint8_t *pData)
{
    int res = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    res |= i2c_master_start(cmd);
    res |= i2c_master_write_byte(cmd, APT8L08_ADDR, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_write_byte(cmd, regAdd, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_stop(cmd);
    res |= i2c_master_cmd_begin(APT8L08Conf.i2c_port_num, cmd, 0);
    i2c_cmd_link_delete(cmd);

    cmd = i2c_cmd_link_create();
    res |= i2c_master_start(cmd);
    res |= i2c_master_write_byte(cmd, APT8L08_ADDR | 0x01, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_read_byte(cmd, pData, I2C_MASTER_NACK/*NACK_VAL*/);
    res |= i2c_master_stop(cmd);
    res |= i2c_master_cmd_begin(APT8L08Conf.i2c_port_num, cmd, 0);
    i2c_cmd_link_delete(cmd);
	if(res)
		*pData = 0;
    return res;
}

static int APT8L08RegInit()
{
	int res = 0;
	res = APT8L08WriteReg(0x3A,0x5a);
	res |= APT8L08WriteReg(0x23,0x00);
	res |= APT8L08WriteReg(0x24,0x00);
	res |= APT8L08WriteReg(0x25,0x00);
	res |= APT8L08WriteReg(0x26,0x08);
	res |= APT8L08WriteReg(0x27,0x02);
	res |= APT8L08WriteReg(0x28,0x02);
	res |= APT8L08WriteReg(0x29,0x10);
	res |= APT8L08WriteReg(0x2A,0x10);
	res |= APT8L08WriteReg(0x2B,0x04);
	res |= APT8L08WriteReg(0x2C,0x00);
	res |= APT8L08WriteReg(0x2D,0x00);
	res |= APT8L08WriteReg(0x3A,0x00);
	return res;
}


static int APT8L08RegSet()
{
	int res = 0;
	vTaskDelay(20 / portTICK_RATE_MS);
	res = APT8L08RegInit();
	vTaskDelay(200 / portTICK_RATE_MS);

	res |= APT8L08WriteReg(0x3A,0x5a);
	res |= APT8L08WriteReg(0x21,0x55);
	res |= APT8L08WriteReg(0x29,0x50);
	
	res |= APT8L08WriteReg(0x23,0xe0);
	//res |= APT8L08WriteReg(0x24,0xf0);
	//res |= APT_WriteReg(0x2A,0xff);
	//res |= APT_WriteReg(0x2D,0x08);


	//res |= APT_WriteReg(0x2C,0x08);
	res |= APT8L08WriteReg(0x20,0x02);
	
	res |= APT8L08WriteReg(0x00,0x12);
	res |= APT8L08WriteReg(0x01,0x12);
	res |= APT8L08WriteReg(0x02,0x0C);
	res |= APT8L08WriteReg(0x03,0x08);
	res |= APT8L08WriteReg(0x04,0x04);
	//res |= APT8L08WriteReg(0x05,0xff);
	//res |= APT8L08WriteReg(0x06,0xff);
	//res |= APT8L08WriteReg(0x07,0xff);	 
	res |= APT8L08WriteReg(0x3A,0x00);
	return res;
	
}


int APT8L08Init()
{
	int res = 0;int cnt = 0;
	APT8L08Config* cfg = (APT8L08Config*)&APT8L08Conf;
    res = I2cInit(&cfg->i2c_cfg, cfg->i2c_port_num); // ESP32 in master mode
RETRY:
	res |= APT8L08RegSet();
	if(0 != res){
		if(cnt < 3){
			LOG(TAG,"Init retry:%d",cnt);
			cnt++;
			goto RETRY;
		}else{
			LOG(TAG,"Init fail!!!");
		}
	}
	//APT8L08IntrInstall();
	return res;
}


int APT8L08GetKeyValue(uint8_t* key)
{
	int ret = APT8L08ReadReg (R_APT8L08_KVR0, key);
	//LOG(TAG,"touch_key value:%x\n",*key);	
	return ret;
}



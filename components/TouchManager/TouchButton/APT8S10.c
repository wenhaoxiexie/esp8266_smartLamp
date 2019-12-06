#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "string.h"
#include "sdkconfig.h"
#include "APT8S10.h"
#include "log.h"
#include "board.h"

#define TAG	"APT8S10"

typedef struct {
    i2c_port_t i2c_port_num;
    i2c_config_t i2c_cfg;
} APT8S10Config;

extern xQueueHandle toucn_btn_evt_queue;
static const APT8S10Config APT8S10CONFIG = {
    .i2c_port_num = I2C_NUM_0,
    .i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = TOUCH_IIC_DATA,
        .scl_io_num = TOUCH_IIC_CLK,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
    }
};
	

static void Delay(uint32_t nCount)
{ 
  for(; nCount != 0; nCount--);
}


static int I2cInit(i2c_config_t *conf, int i2cMasterPort)
{
    int res = 0;

    res = i2c_driver_install(i2cMasterPort, conf->mode);
    res |= i2c_param_config(i2cMasterPort, conf);
    return res;
}

static int APT8S10WriteByte(uint8_t regAdd, uint8_t data)
{
	int res = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    res |= i2c_master_start(cmd);
    res |= i2c_master_write_byte(cmd, APT8S10_ADDR, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_write_byte(cmd, regAdd, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_write_byte(cmd, data, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_stop(cmd);
    res |= i2c_master_cmd_begin(APT8S10CONFIG.i2c_port_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return res;
}

static int APT8S10ReadReg(uint8_t regAdd, uint8_t *pData)
{
    int res = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    res |= i2c_master_start(cmd);
    res |= i2c_master_write_byte(cmd, APT8S10_ADDR, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_write_byte(cmd, regAdd, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_stop(cmd);
    res |= i2c_master_cmd_begin(APT8S10CONFIG.i2c_port_num, cmd, 0);
    i2c_cmd_link_delete(cmd);

    cmd = i2c_cmd_link_create();
    res |= i2c_master_start(cmd);
    res |= i2c_master_write_byte(cmd, APT8S10_ADDR | 0x01, 1 /*ACK_CHECK_EN*/);
    res |= i2c_master_read_byte(cmd, pData, I2C_MASTER_NACK/*NACK_VAL*/);
    res |= i2c_master_stop(cmd);
    res |= i2c_master_cmd_begin(APT8S10CONFIG.i2c_port_num, cmd, 0);
    i2c_cmd_link_delete(cmd);
	if(res)
		*pData = 0;
    return res;
}

static int APT8S10WriteReg(uint8_t regAdd, uint8_t data)
{
	uint8_t read_data = 0;
	uint ret = 0;
	do
	{
		ret = APT8S10WriteByte(regAdd,data);
		Delay(400);	
		ret |= APT8S10ReadReg(regAdd,&read_data);
		if(0 != ret)
			break;
	}
	while(data != read_data);    
	return ret;
}


static int APT8S10RegInit()
{
	int res = 0;
	uint8_t key;
	uint8_t delay=0;
	
	while(delay<100)
	{
		APT8S10ReadReg(0x51, &key);
		if(key==0x55)
			break;
		delay++;
		usleep(1000);
	}
	LOG(TAG,">>>>>>>>>>>>>>>>delay:%d",delay);
	res = APT8S10WriteReg(0x00,0xff);//Trigglevel
	res |= APT8S10WriteReg(0x01,0x70);
	res |= APT8S10WriteReg(0x02,0x78);
	res |= APT8S10WriteReg(0x03,0xff);
	res |= APT8S10WriteReg(0x04,0xff);
	res |= APT8S10WriteReg(0x05,0x78);
	res |= APT8S10WriteReg(0x06,0x78);
	res |= APT8S10WriteReg(0x07,0x78);
	res |= APT8S10WriteReg(0x08,0x70);
	res |= APT8S10WriteReg(0x09,0x50);
	res |= APT8S10WriteReg(0x0a,0x50);
	res |= APT8S10WriteReg(0x0b,0x28);
	res |= APT8S10WriteReg(0x0c,0xff);
	res |= APT8S10WriteReg(0x0d,0xff);
	res |= APT8S10WriteReg(0x0e,0xff);
	
	
	res |= APT8S10WriteReg(0x20,0x03);				//GSR
	
	res |= APT8S10WriteReg(0X21,0X64);				//TK_MCONL
	
	res |= APT8S10WriteReg(0x23,0xe6);				//TK ENABLE 0
	res |= APT8S10WriteReg(0x24,0x1f);				//TK ENABLE 1
	
	res |= APT8S10WriteReg(0X2c,0X02);				//TK MCONH
	
	res |= APT8S10WriteReg(0X10,0X01);				//rebuild

	return res;
}


int APT8S10Init()
{
    int res = 0;int cnt = 0;
	APT8S10Config* cfg = (APT8S10Config*)&APT8S10CONFIG;
    res = I2cInit(&cfg->i2c_cfg, cfg->i2c_port_num); // ESP32 in master mode

	LOG(TAG,">>>>>>> I2cInit:%d",res);
RETRY:
	res |= APT8S10RegInit();
	if(0 != res){
		if(cnt < 3){
			LOG(TAG,"Init retry:%d",cnt);
			cnt++;
			goto RETRY;
		}else{
			LOG(TAG,"Init fail!!!");
		}
	}
	return res;
}

int APT8S10GetKeyValue(uint8_t* key)
{
	int ret;
	 
	ret = APT8S10ReadReg (0x35, key);
	
	//LOG(TAG,"############ touch_key value:%x\n",*key);
	
	return ret;
}



int APT8S10GetWheelValue(uint8_t* value)
{
    int ret = APT8S10ReadReg (0x36, value);
	//LOG(TAG,"WheelValue value:%x\n",*value);
	//usleep(100000);
	return ret;
}



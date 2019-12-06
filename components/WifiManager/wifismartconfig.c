/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "wifismartconfig.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_types.h"
#include "esp_smartconfig.h"
#include "esp_wifi.h"
#include "log.h"

#define SC_DONE_EVT        BIT0
#define SC_STOP_REQ_EVT    BIT1

#define IOT_CHECK(tag, a, ret)  if(!(a)) {                                             \
        LOG(tag,"%s:%d (%s)", __FILE__, __LINE__, __FUNCTION__);      \
        return (ret);                                                                   \
        }

#define ERR_ASSERT(tag, param)  IOT_CHECK(tag, (param) == ESP_OK, ESP_FAIL)

#define WIFI_SC_TAG "WIFI_SMART_CONFIG"

xSemaphoreHandle g_sc_mux = NULL;
EventGroupHandle_t g_sc_event_group = NULL;

/******************************************************************************
 * FunctionName : smartconfig_done
 * Description  : callback function which be called during the samrtconfig process
 * Parameters   : status -- the samrtconfig status
 *                pdata --
 * Returns      : none
*******************************************************************************/

esp_err_t SmartconfigSetup(smartconfig_type_t sc_type, bool fast_mode_en)
{
	 ESP_ERROR_CHECK(esp_smartconfig_set_type(sc_type));
     ERR_ASSERT(WIFI_SC_TAG, esp_smartconfig_fast_mode(fast_mode_en));
	/* 
    if (g_sc_event_group == NULL) {
       	g_sc_event_group = xEventGroupCreate();
        if (g_sc_event_group == NULL) {
            LOG(WIFI_SC_TAG, "g_sc_event_group creat failed!");
            return ESP_FAIL;
        }
        ESP_ERROR_CHECK(esp_smartconfig_set_type(sc_type));
        ERR_ASSERT(WIFI_SC_TAG, esp_smartconfig_fast_mode(fast_mode_en));
    }
   if (g_sc_mux == NULL) {
        g_sc_mux = xSemaphoreCreateMutex();
        if (g_sc_mux == NULL) {
            LOG(WIFI_SC_TAG, "g_sc_mux creat failed!");
            vEventGroupDelete(g_sc_event_group);
            return ESP_FAIL;
        }
    }
   */
    return ESP_OK;
}


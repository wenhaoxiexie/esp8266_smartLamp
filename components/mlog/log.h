#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "esp_system.h"
#include <stdlib.h>
#include "sdkconfig.h"

#define LOG(Tag,format, ...)	printf("==>[%s]: ",Tag);printf(format,##__VA_ARGS__);printf("\n");

#define m_log	LOG("mdebug","%s==>%d",__FUNCTION__,__LINE__)

#endif


# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(PROJECT_PATH)/ESP8266_RTOS_SDK/components/newlib/include $(PROJECT_PATH)/ESP8266_RTOS_SDK/components/newlib/newlib/include $(PROJECT_PATH)/ESP8266_RTOS_SDK/components/newlib/newlib/port/include
COMPONENT_LDFLAGS += -L$(BUILD_DIR_BASE)/newlib $(PROJECT_PATH)/ESP8266_RTOS_SDK/components/newlib/newlib/lib/libc_nano.a $(PROJECT_PATH)/ESP8266_RTOS_SDK/components/newlib/newlib/lib/libm.a -lnewlib
COMPONENT_LINKER_DEPS += $(PROJECT_PATH)/ESP8266_RTOS_SDK/components/newlib/newlib/lib/libc_nano.a $(PROJECT_PATH)/ESP8266_RTOS_SDK/components/newlib/newlib/lib/libm.a
COMPONENT_SUBMODULES += 
COMPONENT_LIBRARIES += newlib
component-newlib-build: 

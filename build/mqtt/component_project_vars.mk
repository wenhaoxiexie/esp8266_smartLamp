# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(PROJECT_PATH)/ESP8266_RTOS_SDK/components/mqtt/esp-mqtt/include
COMPONENT_LDFLAGS += -L$(BUILD_DIR_BASE)/mqtt -lmqtt
COMPONENT_LINKER_DEPS += 
COMPONENT_SUBMODULES += $(PROJECT_PATH)/ESP8266_RTOS_SDK/components/mqtt/esp-mqtt
COMPONENT_LIBRARIES += mqtt
component-mqtt-build: 

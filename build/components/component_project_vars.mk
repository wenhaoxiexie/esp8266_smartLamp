# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(PROJECT_PATH)/components/include $(PROJECT_PATH)/components/DeviceManager $(PROJECT_PATH)/components/LedManager $(PROJECT_PATH)/components/mlog $(PROJECT_PATH)/components/SystemMemory $(PROJECT_PATH)/components/TouchManager $(PROJECT_PATH)/components/TouchService $(PROJECT_PATH)/components/WifiManager $(PROJECT_PATH)/components/MqttService $(PROJECT_PATH)/components/OtaService $(PROJECT_PATH)/components/SysClock
COMPONENT_LDFLAGS += -L$(BUILD_DIR_BASE)/components -lcomponents
COMPONENT_LINKER_DEPS += 
COMPONENT_SUBMODULES += 
COMPONENT_LIBRARIES += components
component-components-build: 

#pragma once

/* 
* Optional
*/
//#define LAYOUT_IN_RAM

/*
* Main consts
*/
#define FIRMWARE_VERSION "2.3.5-dev"
#define LED_PIN 2
#define FLASH_4MB true
#define MQTT_RECONNECT_INTERVAL 20000
// 1000 * 60 * 60 * 2
#define TELEMETRY_UPDATE_INTERVAL 0

enum SchedulerTask { WIFI_SCAN,
                     MQTT_CONNECTION,
                     SENSORS,
                     STEPPER1,
                     STEPPER2,
                     LOGGER,
                     TIMERS,
                     TIME,
                     TIME_SYNC,
                     STATISTICS,
                     UPTIME,
                     ANNOUNCE,
                     UDP_DB,
                     SYS_TIMINGS,
                     SYS_MEMORY };

enum ErrorType_t {
    ET_NONE,
    ET_FUNCTION,
    ET_MODULE,
    ET_SYSTEM
};

enum ErrorLevel_t {
    EL_INFO,
    EL_WARNING,
    EL_ERROR
};

enum LedStatus_t {
    LED_OFF,
    LED_ON,
    LED_SLOW,
    LED_FAST
};

enum ConfigType_t {
    CT_CONFIG,
    CT_SCENARIO
};

enum BusScanner_t {
    BS_I2C,
    BS_ONE_WIRE
};

enum BroadcastMessage_t {
    BM_ANNOUNCE,
    BM_MQTT_SETTINGS,
    NUM_BROADCAST_MESSAGES
};

extern const char* DEVICE_LAYOUT_FILE;
extern const char* DEVICE_RUNTIME_FILE;
extern const char* DEVICE_CONFIG_FILE;
extern const char* DEVICE_COMMAND_FILE;
extern const char* DEVICE_SCENARIO_FILE;

extern const char* TAG_INIT;
extern const char* TAG_OPTIONS;
extern const char* TAG_RUNTIME;
extern const char* TAG_LAST_VERSION;

#define TAG_ONE_WIRE "oneWire"
#define TAG_I2C "i2c"
#define TAG_SHARE_MQTT "share_mqtt"
#define TAG_CHECK_MQTT "check_mqtt"

extern const char* getMessageType(BroadcastMessage_t message_type);
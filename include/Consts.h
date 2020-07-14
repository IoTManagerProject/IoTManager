#pragma once
/*
* Main consts
*/
#define FIRMWARE_VERSION "2.3.5-dev"
#define LED_PIN 2
#define FLASH_4MB true
// 1000 * 60 * 60 * 2
#define TELEMETRY_UPDATE_INTERVAL 0

enum SchedulerTask { WIFI_SCAN,
                     NETWORK_CONNECTION,
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

enum LoopItems {
    LI_CLOCK,
    LT_FLAG_ACTION,
    LI_MQTT_CLIENT,
    LI_CMD,
    LI_ITEMS,
    LI_SCENARIO,
    LI_BROADCAST,
    LI_SERIAL,
    LT_TASKS,
    LT_LOGGER,
    NUM_LOOP_ITEMS
};


extern const char* getMessageType(BroadcastMessage_t message_type);
#pragma once
/*
* Main consts
*/
#define FIRMWARE_VERSION "2.3.5-dev"
#define LED_PIN 2
#define FLASH_4MB true
#define MQTT_RECONNECT_INTERVAL 20000
// 1000 * 60 * 60 * 2
#define TELEMETRY_UPDATE_INTERVAL 0

#define DEFAULT_PRESET 100
#define DEFAULT_SCENARIO 100

#define TAG_ONE_WIRE "oneWire"
#define TAG_I2C "i2c"
#define TAG_ONE_WIRE_PIN "oneWirePin"

/* 
* Optional
*/
//#define OTA_UPDATES_ENABLED
//#define WEBSOCKET_ENABLED
//#define LAYOUT_IN_RAM

/* 
* Sensor 
*/

#define LEVEL_ENABLED
#define ANALOG_ENABLED
#define DALLAS_ENABLED
#define DHT_ENABLED
#define BMP_ENABLED
#define BME_ENABLED

/* 
* Gears 
*/
#define STEPPER_ENABLED
#define SERVO_ENABLED

/* 
* Other
*/
#define LOGGING_ENABLED
#define SERIAL_ENABLED
#define PUSH_ENABLED

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

extern const char* DEVICE_LAYOUT_FILE;
extern const char* DEVICE_RUNTIME_FILE;
extern const char* DEVICE_CONFIG_FILE;
extern const char* DEVICE_COMMAND_FILE;
extern const char* DEVICE_SCENARIO_FILE;
extern const char* KNOWN_DEVICE_FILE;

extern const char* HEADER_ANNOUNCE;
extern const char* HEADER_MQTT_SETTINGS;

extern const char* TAG_INIT;
extern const char* TAG_OPTIONS;
extern const char* TAG_RUNTIME;

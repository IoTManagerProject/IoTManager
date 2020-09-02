#pragma once
/*
* Main consts
*/
#define FIRMWARE_VERSION "2.4.0"
#define NUM_BUTTONS 6
#define LED_PIN 2
#define FLASH_4MB true

#define MQTT_RECONNECT_INTERVAL 20000

#define TELEMETRY_UPDATE_INTERVAL 7200000

#define DEVICE_CONFIG_FILE "s.conf.csv"
#define DEVICE_SCENARIO_FILE "s.scen.txt"

#define DEFAULT_PRESET 100
#define DEFAULT_SCENARIO 100

#define TAG_ONE_WIRE "oneWire"
#define TAG_I2C "i2c"
#define TAG_ONE_WIRE_PIN "oneWirePin"

/* 
* Optional
*/
//#define OTA_UPDATES_ENABLED
//#define MDNS_ENABLED
//#define WEBSOCKET_ENABLED
//#define LAYOUT_IN_RAM
//#define UDP_ENABLED

/* 
* Sensor 
*/
#define TANK_LEVEL_SAMPLES 10
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

struct Time_t {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day_of_week;
    uint8_t day_of_month;
    uint8_t month;
    uint16_t day_of_year;
    uint16_t year;
    unsigned long days;
    unsigned long valid;
};

enum TimerTask_t { WIFI_SCAN,
                   WIFI_MQTT_CONNECTION_CHECK,
                   SENSORS,
                   STEPPER1,
                   STEPPER2,
                   LOG1,
                   LOG2,
                   LOG3,
                   LOG4,
                   LOG5,
                   TIMER_COUNTDOWN,
                   TIME,
                   TIME_SYNC,
                   STATISTICS,
                   UPTIME,
                   UDP,
                   UDP_DB,
                   TEST };

enum notAsincActions {
   do_ZERO, 
   do_UPGRADE,
   do_GETLASTVERSION,
   do_UDPDATAPARSE,
   do_MQTTUDP,
   do_BUSSCAN,
   do_MQTTPARAMSCHANGED,
   do_LAST,
};                   

enum ErrorType_t {
    ET_NONE,
    ET_FUNCTION,
    ET_MODULE,
    ET_SYSTEM
};

enum ErrorLevel_t {
    EL_NONE,
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

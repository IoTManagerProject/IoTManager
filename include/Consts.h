#pragma once

#define FIRMWARE_VERSION 285

#ifdef esp8266_4mb
#define FIRMWARE_NAME "esp8266_4mb"
#define USE_LITTLEFS true
#define USE_OTA true
#define LED_PIN 2
#define ESP_MODE
#endif

#ifdef esp8266_1mb
#define FIRMWARE_NAME "esp8266_1mb"
#define USE_LITTLEFS false
#define USE_OTA false
#define LED_PIN 2
#define ESP_MODE
#endif

#ifdef esp32_4mb
#define FIRMWARE_NAME "esp32_4mb"
#define USE_LITTLEFS false
#define USE_OTA true
#define LED_PIN 22
#define ESP_MODE
#endif

#ifdef esp8266_mysensors_4mb
#define FIRMWARE_NAME "esp8266_mysensors_4mb"
#define USE_LITTLEFS true
#define USE_OTA true
#define LED_PIN 2
#define GATE_MODE
#define MYSENSORS
#define NODE_RED_COLOR_TIMEOUT 120
#define NODE_ORANGE_COLOR_TIMEOUT 60
#endif

#ifdef esp32_mysensors_4mb
#define FIRMWARE_NAME "esp32_mysensors_4mb"
#define USE_LITTLEFS false
#define USE_OTA true
#define LED_PIN 22
#define GATE_MODE
#define MYSENSORS
#define NODE_RED_COLOR_TIMEOUT 120
#define NODE_ORANGE_COLOR_TIMEOUT 60
#endif

#define NUM_BUTTONS 6
#define MQTT_RECONNECT_INTERVAL 20000
#define TELEMETRY_UPDATE_INTERVAL_MIN 60
#define DEVICE_CONFIG_FILE "s.conf.csv"
#define DEVICE_SCENARIO_FILE "s.scen.txt"
//#define OTA_UPDATES_ENABLED
//#define MDNS_ENABLED
//#define WEBSOCKET_ENABLED
//#define LAYOUT_IN_RAM
//#define UDP_ENABLED
//#define SSDP_ENABLED

#ifdef ESP_MODE
#define EnableButtonIn
#define EnableButtonOut
#define EnableCountDown
#define EnableImpulsOut
#define EnableInput
#define EnableLogging
#define EnableOutput
#define EnablePwmOut
#define EnableSensorAnalog
#define EnableSensorBme280
#define EnableSensorBmp280
#define EnableSensorCcs811
#define EnableSensorDallas
#define EnableSensorDht
#define EnableSensorPzem
#define EnableSensorUltrasonic
#define EnableSensorUptime
#define EnableTelegram
#define EnableUart
#endif

#ifdef GATE_MODE
#define EnableSensorGate
#define EnableButtonOut
#define EnableCountDown
#define EnableInput
#define EnableLogging
#define EnableOutput
#define EnableSensorUptime
#define EnableTelegram
#define EnableUart
#endif

//================================================================================================================================================================

enum TimerTask_t { WIFI_SCAN,
                   WIFI_MQTT_CONNECTION_CHECK,
                   TIME,
                   TIME_SYNC,
                   STATISTICS,
                   UPTIME,
                   UDP,
                   SYGNAL,
                   TIMES };

enum NotAsyncActions {
    do_ZERO,
    do_UPGRADE,
    do_GETLASTVERSION,
    do_BUSSCAN,
    do_MQTTPARAMSCHANGED,
    do_deviceInit,
    do_delChoosingItems,
    do_addItem,
    do_addPreset,
    do_sendScenUDP,
    do_sendScenMQTT,
    do_LAST,
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

//history
//07.11.2020 (SSDP OFF, UDP OFF)
//RAM:   [=====     ]  46.8% (used 38376 bytes from 81920 bytes)
//Flash: [=====     ]  54.2% (used 566004 bytes from 1044464 bytes)

//13.11.2020 (SSDP OFF, UDP OFF)
//RAM:   [=====     ]  46.6% (used 38208 bytes from 81920 bytes)
//Flash: [=====     ]  54.2% (used 566388 bytes from 1044464 bytes)

//15.11.2020 (SSDP OFF, UDP OFF)
//RAM:   [=====     ]  46.1% (used 37780 bytes from 81920 bytes)
//Flash: [=====     ]  54.3% (used 566656 bytes from 1044464 bytes)

//17.11.2020 (SSDP OFF, UDP OFF)
//RAM:   [=====     ]  45.7% (used 37476 bytes from 81920 bytes)
//Flash: [=====     ]  54.5% (used 569296 bytes from 1044464 bytes)

//RAM:   [=====     ]  45.6% (used 37336 bytes from 81920 bytes)
//Flash: [======    ]  55.3% (used 577396 bytes from 1044464 bytes)

//eventBuf - буфер событий которые проверяются в сценариях,
//и если событие удовлетворяет какому нибудь условию то выполняются указанные команды

//orderBuf - буфер команд которые выполняются сейчас же

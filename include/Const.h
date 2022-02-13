#pragma once

//Это версия прошивки
#define FIRMWARE_VERSION 401

//Размер буфера json
#define JSON_BUFFER_SIZE 1024

//выбор сервера
//#define ASYNC_WEB_SERVER
//#define ASYNC_WEB_SOCKETS
#define STANDARD_WEB_SERVER
#define STANDARD_WEB_SOCKETS

#define UDP_ENABLED

//#define REST_FILE_OPERATIONS

#define MQTT_RECONNECT_INTERVAL 20000
#define CHANGE_BROKER_AFTER 5

#ifdef esp8266_4mb
#define USE_LITTLEFS true
#endif

#ifdef esp32_4mb
#define USE_LITTLEFS false
#endif

//задачи таскера
enum TimerTask_t { WIFI_SCAN,
                   WIFI_MQTT_CONNECTION_CHECK,
                   TIME,
                   TIME_SYNC,
                   UPTIME,
                   UDP,
                   TIMES,
                   PTASK };

//задачи которые надо протащить через loop
enum NotAsyncActions {
    do_ZERO,
    do_MQTTPARAMSCHANGED,
    do_LAST,
};
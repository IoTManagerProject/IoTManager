#pragma once

//Версия прошивки
#define FIRMWARE_VERSION 408

#ifdef esp8266_4mb
#define FIRMWARE_NAME "esp8266_4mb"
#endif

#ifdef esp32_4mb
#define FIRMWARE_NAME "esp32_4mb"
#endif

//Размер буфера json
#define JSON_BUFFER_SIZE 2048

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
                   UDP, //UDPP
                   TIMES,
                   PTASK };

//задачи которые надо протащить через loop
enum NotAsyncActions {
    do_ZERO,
    do_MQTTPARAMSCHANGED,
    do_LAST,
};

//состояния обновления
enum UpdateStates { NOT_STARTED,
                    UPDATE_FS_IN_PROGRESS,
                    UPDATE_FS_COMPLETED,
                    UPDATE_FS_FAILED,
                    UPDATE_BUILD_IN_PROGRESS,
                    UPDATE_BUILD_COMPLETED,
                    UPDATE_BUILD_FAILED,
                    PATH_ERROR
};
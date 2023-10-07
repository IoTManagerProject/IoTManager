#pragma once
#include "BuildTime.h"

// Версия прошивки
#define FIRMWARE_VERSION 452

#ifdef esp8266_1mb_ota
#define FIRMWARE_NAME "esp8266_1mb_ota"
#endif

#ifdef esp8266_1mb
#define FIRMWARE_NAME "esp8266_1mb"
#endif

#ifdef esp8266_2mb
#define FIRMWARE_NAME "esp8266_2mb"
#endif

#ifdef esp8266_2mb_ota
#define FIRMWARE_NAME "esp8266_2mb_ota"
#endif

#ifdef esp8266_4mb
#define FIRMWARE_NAME "esp8266_4mb"
#endif

#ifdef esp8266_16mb
#define FIRMWARE_NAME "esp8266_16mb"
#endif

#ifdef esp32_4mb
#define FIRMWARE_NAME "esp32_4mb"
#endif

#ifdef esp32cam_4mb
#define FIRMWARE_NAME "esp32cam_4mb"
#endif

#ifdef esp32_16mb
#define FIRMWARE_NAME "esp32_16mb"
#endif

#ifdef esp32s2_4mb
#define FIRMWARE_NAME "esp32s2_4mb"
#endif

// Размер буфера json
#define JSON_BUFFER_SIZE 4096 // держим 2 кб не меняем

/*
WEB_SOCKETS_FRAME_SIZE создан для того что бы не загружать оперативку.
Эта технология передаёт в сокеты большие файлы по частям.
Чем меньше этот фрейм тем теоретически лучше.
Но и сильно малый он тоже быть не должен.
Я опытным путём установил что размер 1024 является оптимальным. Можно так же поставить 2048
*/
#define WEB_SOCKETS_FRAME_SIZE 1024

// #define LOOP_DEBUG

// выбор сервера
// #define ASYNC_WEB_SERVER
// #define ASYNC_WEB_SOCKETS
#define STANDARD_WEB_SERVER
#define STANDARD_WEB_SOCKETS

#define UDP_ENABLED

// #define REST_FILE_OPERATIONS

#define MQTT_RECONNECT_INTERVAL 20000

#define TELEMETRY_UPDATE_INTERVAL_MIN 60

#define USE_LITTLEFS true

#define START_DATETIME 1661990400 // 01.09.2022 00:00:00 константа для сокращения unix time

#define MIN_DATETIME 1575158400
#define LEAP_YEAR(Y) (((1970 + Y) > 0) && !((1970 + Y) % 4) && (((1970 + Y) % 100) || !((1970 + Y) % 400)))

// задачи таскера
enum TimerTask_t { WIFI_SCAN,
    WIFI_MQTT_CONNECTION_CHECK,
    TIME,
    TIME_SYNC,
    UPTIME,
    UDP, // UDPP
    TIMES, // периодические секундные проверки
    PTASK,
    ST,
    END };

// задачи которые надо протащить через loop
enum NotAsyncActions {
    do_ZERO,
    do_MQTTPARAMSCHANGED,
    do_LAST,
};

// состояния обновления
enum UpdateStates { NOT_STARTED,
    UPDATE_FS_IN_PROGRESS,
    UPDATE_FS_COMPLETED,
    UPDATE_FS_FAILED,
    UPDATE_BUILD_IN_PROGRESS,
    UPDATE_BUILD_COMPLETED,
    UPDATE_BUILD_FAILED,
    PATH_ERROR
};

enum distination {
    TO_MQTT,
    TO_WS,
    TO_MQTT_WS,
};

#define WS_BROADCAST -1

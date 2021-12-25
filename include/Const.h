#pragma once

//Это версия прошивки
#define FIRMWARE_VERSION 400

//Размер буфера json
#define JSON_BUFFER_SIZE 1024

//выбор сервера
//#define ASYNC_WEB_SERVER
//#define ASYNC_WEB_SOCKETS
#define STANDARD_WEB_SERVER
#define STANDARD_WEB_SOCKETS

//#define REST_FILE_OPERATIONS

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
                   SYGNAL,
                   TIMES,
                   MYTEST };
#pragma once

//Это версия прошивки
#define FIRMWARE_VERSION 400

//Размер буфера json
#define JSON_BUFFER_SIZE 4096

//выбор сервера (или асинхронный, или обычный)
//#define ASYNC_WEB_SERVER
#define STANDARD_WEB_SERVER

//если мы используем стандартный веб сервер то нужна библиотека веб сокетов
//если асинхронный то плагин сокетов включен в него
#ifdef STANDARD_WEB_SERVER
#define STANDARD_WEB_SOCKETS
#endif

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
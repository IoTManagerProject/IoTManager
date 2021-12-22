#pragma once

#define FIRMWARE_VERSION 400

#define JSON_BUFFER_SIZE 4096

//задачи таскера
enum TimerTask_t { WIFI_SCAN,
                   WIFI_MQTT_CONNECTION_CHECK,
                   TIME,
                   TIME_SYNC,
                   UPTIME,
                   SYGNAL,
                   TIMES,
                   MYTEST };
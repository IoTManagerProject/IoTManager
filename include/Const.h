#pragma once
#include "BuildTime.h"

// Версия прошивки "<ручная часть>.<счётчик коммитов git>", например "463.2225".
// Первое число поднимается вручную при значимых изменениях, второе перезаписывается
// автоматически pre-скриптом tools/version_bump.py при каждой сборке прошивки —
// править вручную нужно только первое число (и то не обязательно).
#define FIRMWARE_VERSION "463.2240"

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

#ifdef esp32_4mb3f
#define FIRMWARE_NAME "esp32_4mb3f"
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

#ifdef esp32c3m_4mb
#define FIRMWARE_NAME "esp32c3m_4mb"
#endif

#ifdef esp32s3_16mb
#define FIRMWARE_NAME "esp32s3_16mb"
#endif

#ifdef bk7231n
#define FIRMWARE_NAME "bk7231n"
#endif

#ifdef esp32c6_4mb
#define FIRMWARE_NAME "esp32c6_4mb"
#endif

#ifdef esp32c6_8mb
#define FIRMWARE_NAME "esp32c6_8mb"
#endif

#ifdef esp32_wifirep
#define FIRMWARE_NAME "esp32_wifirep"
#endif
// Размер буфера json
#define JSON_BUFFER_SIZE 4096  // держим 2 кб не меняем

/*
WEB_SOCKETS_FRAME_SIZE создан для того что бы не загружать оперативку.
Эта технология передаёт в сокеты большие файлы по частям.
Чем меньше этот фрейм тем теоретически лучше.
Но и сильно малый он тоже быть не должен.
Я опытным путём установил что размер 1024 является оптимальным. Можно так же поставить 2048
*/
#define WEB_SOCKETS_FRAME_SIZE 1024

// #define LOOP_DEBUG

// выбор сервера и веб-сокетов
//   асинхронный вариант: ESPAsyncWebServer + AsyncWebSocket
//   стандартный вариант: WebServer/ESP8266WebServer + WebSocketsServer — резервный
//
// Единый переключатель для всех окружений: раскомментирован — асинхронный,
// закомментирован — стандартный. Флагов -DASYNC_* в platformio.ini больше нет;
// точечно вариант можно переопределить флагами конкретного окружения
// (-DASYNC_WEB_SERVER или -DSTANDARD_WEB_SERVER, пара *_SOCKETS) — тогда
// ветка ниже не участвует. Для LibreTiny асинхронный вариант требует esphome-форки
// библиотек в lib_deps окружения (см. docs/articles/async-web-server.md, раздел 6.1).
#define LT_ASYNC_WEB_SERVER    // ← закомментировать для стандартного стека (LT_WebSockets)

// если вариант сервера задан флагами -D окружения, значения по умолчанию не применяются
#if !defined(ASYNC_WEB_SERVER) && !defined(STANDARD_WEB_SERVER)
#ifdef LT_ASYNC_WEB_SERVER
#define ASYNC_WEB_SERVER
#define ASYNC_WEB_SOCKETS
#else
#define STANDARD_WEB_SERVER
#define STANDARD_WEB_SOCKETS
#endif
#endif

// взаимозависимости вариантов: асинхронные сокеты работают на объекте асинхронного сервера
#if defined(ASYNC_WEB_SOCKETS) && !defined(ASYNC_WEB_SERVER)
#define ASYNC_WEB_SERVER
#endif

// проверка недопустимых сочетаний (в коде поддержаны оба варианта, но вместе они не собираются)
// [DEPRECATED] Раньше здесь стоял безусловный запрет «ASYNC не поддерживается для LIBRETINY»;
// он снят 2026-10: поддержка появилась в ESPAsyncWebServer 3.7.8 (PR ESP32Async/187),
// см. docs/articles/async-web-server.md (раздел 6.1).
// Гвард ловит случай, когда на LIBRETINY ASYNC пришёл флагом -D из ESP-окружения,
// а переключатель LT_ASYNC_WEB_SERVER выключен: esphome-форки в lib_deps, скорее всего,
// не выставлены — и сборка упала бы невнятной ошибкой literals.h/emptyString.
#if defined(LIBRETINY) && defined(ASYNC_WEB_SERVER) && !defined(LT_ASYNC_WEB_SERVER)
#error "ASYNC на LIBRETINY: не копируйте -DASYNC_* из ESP-окружений — включайте переключателем LT_ASYNC_WEB_SERVER в include/Const.h (нужны esphome-форки, см. docs/articles/async-web-server.md)"
#endif
#if defined(ASYNC_WEB_SERVER) && defined(STANDARD_WEB_SERVER)
#error "Выберите один веб-сервер: ASYNC_WEB_SERVER или STANDARD_WEB_SERVER (оба занимают порт 80)"
#endif
#if defined(ASYNC_WEB_SOCKETS) && defined(STANDARD_WEB_SOCKETS)
#error "Выберите один вариант веб-сокетов: ASYNC_WEB_SOCKETS или STANDARD_WEB_SOCKETS (оба занимают порт 81)"
#endif
#if !defined(ASYNC_WEB_SOCKETS) && !defined(STANDARD_WEB_SOCKETS)
#error "Нужен один из вариантов веб-сокетов: STANDARD_WEB_SOCKETS или ASYNC_WEB_SOCKETS (веб-интерфейс работает только через веб-сокеты)"
#endif

// [DEPRECATED] Старый таймаут (3000 мс) ожидания окна TCP при отправке фрейма веб-сокета:
// синхронное ожидание в loop() замораживало сценарии и таймеры на секунды, когда клиент
// терял WiFi (окно не освобождается без ACK — см. лог "frame DROP: canSend timeout" и
// «взрыв» событий после дисконнекта). Заменён на короткий бюджет отправки +
// автозакрытие «застрявших» клиентов (см. src/AsyncWebServer.cpp).
#define ASYNC_WEB_SOCKETS_SEND_TIMEOUT 3000

// максимальное суммарное время (мс) ожидания освобождения TCP-окна на один фрейм
// веб-сокета: по истечении бюджета фрейм дропается (статусы волатильны), а клиент
// помечается «застрявшим» и закрывается через ASYNC_WEB_SOCKETS_STUCK_CLOSE_MS.
// 100 мс — компромисс: короткая пауза на переполненном TCP-окне (поток файлов /config|)
// не роняет мелкие файлы одним фреймом (scenario.txt ~157 Б), но и не замораживает loop()
#define ASYNC_WEB_SOCKETS_SEND_BUDGET_MS 100
// фреймы файлов и крупных JSON (>= этого размера в байтах) получают расширенный бюджет,
// чтобы здоровый клиент успел подтвердить приём при потоковой отправке
#define ASYNC_WEB_SOCKETS_SEND_BUDGET_FILES_BYTES 256
#define ASYNC_WEB_SOCKETS_SEND_BUDGET_FILES_MS 200
// время (мс) «застревания» клиента, после которого asyncWebSocketsLoop() принудительно
// закрывает его соединение (клиент не отдаёт ACK — потерял WiFi)
#define ASYNC_WEB_SOCKETS_STUCK_CLOSE_MS 2000

//#ifndef LIBRETINY
#define UDP_ENABLED
//#endif
// #define REST_FILE_OPERATIONS

#define MQTT_RECONNECT_INTERVAL 20000

#define TELEMETRY_UPDATE_INTERVAL_MIN 60

#define USE_LITTLEFS true

#define START_DATETIME 1661990400  // 01.09.2022 00:00:00 константа для сокращения unix time

#define MIN_DATETIME 1575158400
#define LEAP_YEAR(Y) (((1970 + Y) > 0) && !((1970 + Y) % 4) && (((1970 + Y) % 100) || !((1970 + Y) % 400)))

#ifdef LIBRETINY
//#define WIFI_ASYNC
#endif

#if defined(ESP32) && !defined(esp32_wifirep)
#define WIFI_ASYNC
#endif

// задачи таскера
enum TimerTask_t {
    WIFI_SCAN,
    WIFI_MQTT_CONNECTION_CHECK,
#ifdef WIFI_ASYNC    
    WIFI_CONN,
#endif    
    TIME,
    // TIME_SYNC, // не используется
    // UPTIME, // не используется
    UDPt,    // UDPP
    TIMES,  // периодические секундные проверки
    PTASK,
    ST,
    PiWS,
    END
};

// задачи которые надо протащить через loop // не используется
// enum NotAsyncActions {
//     do_ZERO,
//     do_MQTTPARAMSCHANGED,
//     do_LAST,
// };

// состояния обновления
enum UpdateStates { UPDATE_COMPLETED, UPDATE_FAILED, PATH_ERROR };

enum distination {
    TO_MQTT,
    TO_WS,
    TO_MQTT_WS
};

// #define WS_BROADCAST -1 // не используется

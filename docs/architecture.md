# Архитектура IoTManager

> Объяснение (**Explanation**): как устроена система. Инструкции — в `docs/guides/`, справочники — в `docs/reference/`.

## 1. Обзор

**IoTManager** — система «умного дома» на микроконтроллерах **ESP8266 / ESP32**
(поддерживаются также ESP8285, ESP32-S2/S3/C3/C6/CAM, чип **BK7231N**).

Микроконтроллер подключается к домашнему Wi-Fi, к его пинам — периферия: сенсоры,
реле, шаговые двигатели, сервоприводы и др. Управление двумя параллельными и
полностью синхронизированными способами:

1. **Мобильное приложение** (iOS/Android) — через MQTT-брокер; работает из любой точки мира;
2. **Веб-браузер** — все устройства на одной странице; работает локально, без интернета.

Логика каждого устройства настраивается **сценариями** (`scenario.txt`): на любое
действие можно назначить любую реакцию.

Сопутствующие инструменты репозитория:

- **magicIoTm** — локальная веб-панель (Python Flask): конфигурация проектов, сборка, прошивка, менеджер устройств;
- **measure_size** — замер размера прошивки (flash) по модулям;
- **PrepareProject / PrepareServer** — подготовка проекта к компиляции.

## 2. Сеть и протоколы

| Назначение | Протокол / адрес |
|---|---|
| Веб-интерфейс устройства | HTTP, порт 80 |
| Управление в реальном времени | WebSocket, порт 81 |
| Встроенный MQTT-брокер (модуль `BrokerMQTT`) | MQTT, порт 1883 |
| Обнаружение устройств | multicast-UDP, группа `239.255.255.255`, порт 4210, пакеты каждые 60 сек |
| Веб-панель magicIoTm | HTTP, `127.0.0.1:5005` |
| Облако / мобильное приложение | MQTT (внешний или встроенный брокер) |
| OTA-обновление | HTTP(S) |

Прошивка раз в 60 секунд рассылает broadcast-презентацию в multicast-группу в
формате JSON-массива, например `[{"ip":"192.168.1.50","wg":"main","name":"..."}]`.
Панель magicIoTm слушает эту группу для обнаружения устройств.

## 3. Прошивка (`src/`)

### Основные компоненты

| Файл | Роль |
|---|---|
| `Main.cpp` | Точка входа, `setup()` / `loop()`, цикл `elementsLoop()` |
| `Global.cpp` | Глобальные объекты: `WiFiClient`, `PubSubClient`, `AsyncWebServer`, `WebSocketsServer` |
| `AsyncWebServer.cpp` | Асинхронный веб-сервер и веб-сокеты (`ASYNC_WEB_SERVER`/`ASYNC_WEB_SOCKETS`) |
| `StandWebServer.cpp` | Регистрация и обработчики HTTP — общие для обоих вариантов сервера |
| `WsServer.cpp` | Обработчики WS-команд и транспорт веб-сокетов — общие для обоих вариантов |
| `MqttClient.cpp` | MQTT-клиент (подключение к брокеру) |
| `DeviceList.cpp` | Список устройств, discovery (HomeAssistant, HomeKit) |
| `EspFileSystem.cpp` | LittleFS — чтение/запись файловой системы |
| `EventsAndOrders.cpp` | Обработка событий и команд |
| `PeriodicTasks.cpp` | Периодические задачи (обновление, NTP) |
| `NTP.cpp` | Синхронизация времени |
| `UpgradeFirm.cpp` | OTA-обновление прошивки |
| `Buffers.cpp` | Буферы данных |
| `DebugTrace.cpp` | Отладочная трассировка |

### Варианты веб-сервера и веб-сокетов

Веб-интерфейс обслуживают два взаимозаменяемых стека (выбор — флагами сборки в `platformio.ini`
→ `[common_env_data].build_flags`, значения по умолчанию и проверки — в `include/Const.h`).
Вариант веб-сервера задаётся единым переключателем `LT_ASYNC_WEB_SERVER` в
`include/Const.h` (по умолчанию async; `-D`-флагов варианта в окружениях нет,
точечно — `-DASYNC_*`/`-DSTANDARD_*` в `build_flags` конкретного env). Подробнее —
[guides/build-firmware.md](guides/build-firmware.md#вариант-веб-сервера-асинхронный-по-умолчанию-или-стандартный);
для `bk7231n` особые библиотеки — esphome-форки в `[env:bk7231n]` (см.
[articles/async-web-server.md](articles/async-web-server.md), раздел 6.1).

| | Стандартный (резервный) | Асинхронный (все сборки) |
|---|---|---|
| Флаги | `STANDARD_WEB_SERVER` + `STANDARD_WEB_SOCKETS` | `ASYNC_WEB_SERVER` + `ASYNC_WEB_SOCKETS` |
| HTTP, порт 80 | `WebServer` (ESP32) / `ESP8266WebServer` | `ESPAsyncWebServer` (обёртка `AsyncWebServerCompat`) |
| WS, порт 81 | `WebSocketsServer` (объект `standWebSocket`) | `AsyncWebSocket` (объект `ws`) |
| Библиотеки | `lib/WebSockets`, `lib/LT_WebSockets` (LibreTiny) | `ESP32Async/ESPAsyncWebServer` (общий `lib_deps_external`), `AsyncTCP` (ESP32, зависимость) / `ESP32Async/ESPAsyncTCP` (ESP8266, прописан в каждой env); для `bk7231n` — `esphome/AsyncTCP-esphome` + `esphome/ESPAsyncWebServer-esphome` |
| Обработчики | `StandWebServer.cpp`, `WsServer.cpp` — одни и те же | `StandWebServer.cpp`, `WsServer.cpp` |
| Готовые окружения | нет: включается закомментированием переключателя `LT_ASYNC_WEB_SERVER` в `Const.h` (глобально) либо `-DSTANDARD_*` флагами env | все env `platformio.ini` по умолчанию (переключатель в `Const.h`) |

`AsyncWebServerCompat` (`include/WebServerCompat.h`) повторяет интерфейс синхронного сервера
(`HTTP.on/serveStatic/hasArg/arg/send/send_P/sendHeader/streamFile/upload`), поэтому
`StandWebServer.cpp`, `UpgradeFirm.cpp` и модуль `EspCam` собираются без изменений в обоих
вариантах. Веб-сокеты сохраняют протокол `standWebSocket`: заголовки сообщений вида
`xxxxxx|0012|`, ограничение в 3 клиента, отправка файлов фреймами RFC 6455 (fin/continuation).
На `bk7231n` асинхронный вариант включён переключателем `LT_ASYNC_WEB_SERVER`
в `include/Const.h` и использует форки ESPHome вместо `ESP32Async` (см.
[articles/async-web-server.md](articles/async-web-server.md), раздел 6.1).

Особенности платформ:

- **ESP8266**: библиотека `ESPAsyncWebServer` включает `lwip/tcpbase.h`, чей `enum tcp_state`
  (`CLOSED`, `LISTEN`, …) конфликтует с `enum wl_tcp_state` из `wl_definitions.h` esp8266-core
  (`error: 'CLOSED' conflicts with a previous declaration`). Поэтому в `include/Global.h` блок
  `#ifdef ASYNC_WEB_SERVER` стоит **до** `#include <ESP8266WiFi.h>` — порядок менять нельзя.
- **Область видимости**: вариант решает переключатель `LT_ASYNC_WEB_SERVER` в `Const.h` —
  он вложен в ветку «ни один вариант не задан флагами», поэтому на окружения с явными
  `-DASYNC_*`/`-DSTANDARD_*` не влияет. Безусловное `#define ASYNC_WEB_SERVER` в `Const.h`
  обошло бы эту ветку и конфликтовало с явными STANDARD-флагами — так делать нельзя.

### Классы (`src/classes/`)

| Класс | Назначение |
|---|---|
| `IoTItem` | Базовый класс элемента конфигурации (каждого подключённого устройства/элемента) |
| `IoTDB` | База данных элементов/конфигурации |
| `IoTGpio` | Работа с GPIO (digital/analog/PWM) |
| `IoTScenario` | Управление сценариями (логика «событие → реакция») |
| `IoTDiscovery` | Обнаружение устройств в сети |
| `IoTBench` | Измерения производительности задач элементов |
| `IoTUart` | Работа с UART |
| `NotAsync` | Вспомогательный класс (синхронный слой) |

Модель: в `Main.cpp` функция `elementsLoop()` последовательно передаёт управление
каждому элементу конфигурации, обрабатывает заказы (`handleOrder()`) и события (`handleEvent()`).

### Модульная система (`src/modules/`)

| Каталог | Тип | Примеры |
|---|---|---|
| `exec/` | Исполнительные | Кнопки, энкодеры, PWM, зуммер, мультитач, Telegram, термостат, FTP, ИК-приёмник |
| `sensors/` | Сенсоры | BME280, DHT11/22, DS18B20, AHTxx, PZEM-004T, UART, NTC, Impulse, MH-Z19, SDS011 |
| `virtual/` | Виртуальные | Переменные, таймеры, Cron, логгеры, Math, погода OWM |
| `display/` | Дисплеи | Nextion и др. |

Каждый модуль — папка с `modinfo.json` (категория меню, конфигурируемый элемент,
автор, версия, `usedLibs` по платформам, размеры flash/RAM по окружениям) и `.cpp`.
Модули подключаются через `build_src_filter` в секциях `[env:*_fromitems]`
`platformio.ini` — включение/выключение без пересборки ядра. Совместимость модуля
с платформой определяется по `usedLibs`.

## 4. Веб-конфигуратор magicIoTm (`tools/magicIoTm/`)

**Назначение**: управление проектами, конфигурация прошивки, сборка (PlatformIO),
замер размеров, прошивка по USB/OTA, менеджер устройств. Сервер — локальный, порт `5005`.

### Структура панели

```
magicIoTm/
├── app.py                  # Точка входа: Flask, CORS, регистрация Blueprint'ов, init()
├── state/globals.py        # Глобальное состояние (проект, кэши, устройства, блокировки)
├── core/                   # Бизнес-логика
│   ├── config.py           # Платформы, modinfo, размеры FLASH/RAM/FS, совместимость
│   ├── devices.py          # Multicast, пинг, статусы, папки устройств, сканирование сети
│   ├── builder.py          # Сборка (обёртка над utils/build.py)
│   ├── measurer.py         # Замер размеров (обёртка над utils/measure_run.py)
│   ├── flasher.py          # USB-прошивка / esptool (обёртка)
│   └── ota.py              # OTA-прошивка (обёртка над utils/ota.py)
├── routes/                 # Flask Blueprint'ы (12): projects, config, modules, build,
│                           #   measure, platforms, tools, validation, upload, ota, scenario, devices
├── utils/                  # Реализация
│   ├── projects.py         # Категории/проекты, валидация, история, бэкапы
│   ├── build.py            # Сборка через PlatformIO (subprocess), парсинг размеров
│   ├── PrepareProject.py   # Подготовка проекта: platformio.ini, API.cpp, data_svelte
│   ├── measure_run.py      # Запуск measure.py, прогресс, abort
│   ├── ota.py              # OTA: локальный HTTP-сервер .bin, шаги, копирование data_svelte
│   ├── flash.py            # USB-прошивка через pio upload / uploadfs
│   ├── esptool_tools.py    # Проверка/установка/обновление esptool
│   └── ws_client.py        # WebSocket/HTTP-клиент к устройствам (RAM/FS, запись)
├── static/index.html       # Frontend (SPA, vanilla JS); favicon.png
├── projects/               # Проекты пользователя (gitignored)
├── devices/                # Папки устройств (gitignored)
└── requirements.txt        # flask>=3.0, flask-cors>=4.0
```

Ранее панель была монолитом `app.py` (~1700 строк); рефакторинг на
`state/ + core/ + routes/ (Blueprints)` — см. [decisions/0001](decisions/0001-magiciotm-blueprint-architecture.md).

### Менеджер устройств

- Обнаружение по multicast-UDP; папка устройства: `devices/<имя> <id>/` с разделами `RAM/` и `FS/`.
- **RAM**: скачивание по WebSocket-командам `/config|` и `/profile|` (порт 81) файлов
  `items.json`, `widgets.json`, `config.json`, `scenario.txt`, `settings.json`, `ota.json`, `profile.json`.
  Запись обратно — только для `config.json`, `scenario.txt`, `settings.json` (обратные
  команды `/gifnoc|`, `/oiranecs|`, `/sgnittes|`), плюс `layout.json` (`/tuoyal|`) и `devlist.json` (`/tsil|`).
- **FS**: рекурсивное скачивание по HTTP (порт 80): `/list?dir=` и `/<path>?download=1`.
  Обратной записи на устройство для FS нет (только локально).
- Сканирование сети: до 64 пингов параллельно, прогресс по SSE, ручное добавление по IP.
- Статусы устройств — конечный автомат, подробно: [reference/device-status-fsm.md](reference/device-status-fsm.md).

### Сборка и прошивка

- Сборка: подготовка профиля (`PrepareProject.py`) → сборка FS (`buildfs`) → сборка прошивки (`build`); поэтапный SSE-лог, расчёт Flash/RAM/FS.
- Замер размеров: `measure_size/measure.py` в фоне; режимы: все модули / отдельный / baseline / профиль / без размера; обновляет кэши `modinfo`/`platforms`. Базовый замер (baseline) дописывает запись в историю платформы в `platforms.json` (версия прошивки из `Const.h` + время замера); потребительские кэши берут последнюю запись.
- USB: esptool (автоустановка/обновление), определение чипа, режимы FS/прошивка/full, защита от нехватки flash.
- OTA: режимы firmware/fs/full; запись FS образом (`flash` — `littlefs.bin`/`spiffs.bin`) или пофайлово (`copy`); локальный HTTP-сервер `.bin`; проверка совместимости платформы.
- Ёмкость FS для расчёта размера берётся из `total_fs` в `platforms.json`, иначе — из размера собранного образа: `.pio/build/<env>/littlefs.bin` (ESP) либо `lt_littlefs.bin` в корне проекта (LibreTiny/bk7231n, таргет `tools/lt_fsbuild.py`); env с LibreTiny-ФС определяется по `tools/lt_fsbuild.py` в `extra_scripts` секции `[env:<env>]` `platformio.ini`.
- Проверка `scenario.txt`: скобки, if/then/else, циклические зависимости, неизвестные переменные относительно `config.json`, встроенные функции `ID.функция()` по `modinfo.json`/`sceninfo.json`.

### Ограничения панели

- Локальный инструмент **без аутентификации**, CORS открыт, возможен path traversal — использовать на доверенной машине и в доверенной сети.
- Работает на **Windows** (`.bat`, `netsh` для SSID, `ping -n`); на Unix сборка/прошивка работают, часть утилит адаптирована под Windows.
- Замер и сборка не могут выполняться одновременно (взаимная блокировка в API).
- Лимит записи файла на устройство — 50 КБ (один WebSocket-фрейм ESP).

## 5. Связь с мобильным приложением

Прошивка отправляет данные через MQTT на брокер (встроенный или облачный, например
wqtt.ru). Мобильное приложение (iOS/Android) подключается к тому же брокеру для
отображения данных и управления.

## 6. Ключевые зависимости

| Компонент | Версия | Назначение |
|---|---|---|
| `ArduinoJson` | 6.18.0 | Парсинг JSON |
| `PubSubClient` | — | MQTT |
| `ESPAsyncUDP` | — | Асинхронный UDP |
| `AsyncWebServer` | — | Асинхронный веб-сервер |
| `WebSocketsServer` | — | WebSocket-сервер |
| `LT_WebSockets` | — | STANDARD-вариант для BK7231N (резерв) |

Для BK7231N используется кастомная ветка `libretiny`; асинхронный стек — форки ESPHome
(раздел 6.1 [articles/async-web-server.md](articles/async-web-server.md)). Имя прошивки —
`iotm_tiny`. Flask-панель: `flask>=3.0`, `flask-cors>=4.0`.

Подробнее о форматах — [reference/data-formats.md](reference/data-formats.md).
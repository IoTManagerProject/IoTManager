# Руководство: сборка прошивки и запуск панели

> How-to документ. Справочник платформ — [reference/platforms.md](../reference/platforms.md), порты/протоколы — [architecture.md](../architecture.md).

## Требования

- **Прошивка**: PlatformIO (VSCode + расширение или CLI).
- **Панель magicIoTm**: Python (в корне — `.venv/`), зависимости из `tools/magicIoTm/requirements.txt` (`flask>=3.0`, `flask-cors>=4.0`).

## Сборка прошивки (PlatformIO)

Конфигурация окружений — в `platformio.ini`; `default_envs` задаёт профиль проекта
(`myProfile.json` → `projectProp.platformio.default_envs`, в эталонном профиле —
`esp32c3m_4mb`, `PrepareProject.py` переносит значение в `platformio.ini`).
Полный список: [reference/platforms.md](../reference/platforms.md).

```bash
# Установить зависимости
pio pkg install

# Сборка окружения по умолчанию (значение default_envs из профиля проекта)
pio run

# Сборка конкретной платы
pio run -e esp8266_1mb_ota
pio run -e esp32_4mb
pio run -e esp32c3m_4mb
pio run -e bk7231n

# Загрузка на устройство
pio run -e esp32s2_4mb --target upload

# Мониторинг порта
pio device monitor
```

### Вариант веб-сервера: асинхронный (по умолчанию) или стандартный

У всех плат собирается **асинхронный** вариант — `ASYNC_WEB_SERVER` + `ASYNC_WEB_SOCKETS`
(`ESPAsyncWebServer` + `AsyncTCP`/`ESPAsyncTCP`). Вариант задаётся единым переключателем
в `include/Const.h`; флагов `-DASYNC_*` в `platformio.ini` больше нет:

```cpp
// include/Const.h
#define LT_ASYNC_WEB_SERVER    // ← закомментировать для стандартного стека
```

У `bk7231n` переключатель тот же, но асинхронный вариант там требует esphome-форки
библиотек в `lib_deps` его секции
(см. [articles/async-web-server.md](../articles/async-web-server.md), раздел 6.1).
Поэтому ничего включать вручную не нужно — `pio run -e esp8266_4mb` уже собирает
асинхронный вариант, отдельных окружений `esp8266_4mb_async` / `esp32s2_4mb_async` в
`platformio.ini` нет (коммит `f0d80db6`).

```bash
# Обычная сборка и загрузка — вариант уже асинхронный
pio run -e esp8266_4mb
pio run -e esp32s2_4mb --target upload
pio run -e esp32s2_4mb -t size   # проверить запас по flash/RAM
```

Библиотеки по платформам:

| Платформа | Библиотеки |
|---|---|
| ESP8266 / ESP8285 | `ESP32Async/ESPAsyncWebServer` (из `lib_deps_external`) + `ESP32Async/ESPAsyncTCP` — прописан в `lib_deps` каждой env |
| ESP32 / ESP32-S2 / S3 / C3 / C6 | `ESP32Async/ESPAsyncWebServer`; `AsyncTCP` приходит его зависимостью |
| BK7231N (LibreTiny) | `esphome/AsyncTCP-esphome` + `esphome/ESPAsyncWebServer-esphome` (форки; `ESP32Async` на форке LibreTiny не собирается) |

**Стандартный вариант** (`STANDARD_WEB_SERVER` + `STANDARD_WEB_SOCKETS`: `WebServer` /
`ESP8266WebServer` + `WebSocketsServer`) остался в коде как **резервный**: он включается
закомментированием переключателя `LT_ASYNC_WEB_SERVER` в `include/Const.h` (глобально)
либо точечно — флагами `-DSTANDARD_WEB_SERVER -DSTANDARD_WEB_SOCKETS` в `build_flags`
нужного окружения (тогда переключатель для него не участвует). Порты и протокол у
вариантов одинаковые (HTTP 80, WS 81), поэтому веб-интерфейс и приложение работают с
обоими.

Нельзя включать оба варианта одновременно (оба занимают порты 80 и 81); недопустимые
сочетания ловит препроцессор (`#error` в `include/Const.h`). Гвард там же отсекает
копирование `-DASYNC_WEB_SERVER` из ESP-окружения в LibreTiny-окружение: для `bk7231n`
асинхронный вариант включается только переключателем в `Const.h` (и требует esphome-форки
библиотек в `lib_deps`).

**Чего делать нельзя:** держать в `include/Const.h` раскомментированными одновременно и
переключатель `LT_ASYNC_WEB_SERVER`, и `#define STANDARD_*` — компилятор поймает конфликт
на `#error` «Выберите один веб-сервер». Переключатель — единственный источник варианта
для окружений без `-D`-флагов.

Особенности ESP8266:

- зависимость `ESPAsyncTCP` прописана в `lib_deps` каждой ESP8266/ESP8285-env явно (видно,
  что именно собирается), а не подтягивается автоматически;
- в `include/Global.h` блок `#ifdef ASYNC_WEB_SERVER` подключается **до** `#include <ESP8266WiFi.h>`:
  `ESPAsyncWebServer.h` включает `lwip/tcpbase.h` (`enum tcp_state`: `CLOSED`, `LISTEN`, …), а
  esp8266-core объявляет такой же `enum wl_tcp_state` в `wl_definitions.h` только если
  `lwip/tcpbase.h` ещё не подключён. Иначе ошибка `error: 'CLOSED' conflicts with a previous
  declaration`. Порядок include в `Global.h` менять нельзя;
- асинхронный вариант «дороже» по flash и RAM. Замеры на одном и том же наборе модулей
  (LittleFS, ldscript `4m1m`):

  | Сборка `esp8266_4mb` | RAM | Flash |
  |---|---|---|
  | стандартный вариант | 38 552 Б (47.1 %) | 475 663 Б (45.5 %) |
  | асинхронный вариант (текущая сборка) | 38 820 Б (47.4 %) | 496 207 Б (47.5 %) |

  То есть ~+20 КБ flash и ~+0.3 КБ RAM. Стандартный замер снимался на временном окружении
  `esp8266_4mb_async` (в `platformio.ini` не сохранено). Платы с 1 МБ flash собираются в
  асинхронном варианте так же, как остальные: например `esp8266_1mb` (ldscript `1m256`) —
  38 660 Б RAM (47.2 %) и 476 059 Б flash (62.5 %), сборка успешна, но запас меньше, чем у
  4 МБ плат, поэтому проверяйте `pio run -e <env> -t size`.

Ручные шаги для конкретного окружения (например, `esp32_4mb`):

1. Подготовить проект: `python PrepareProject.py -p myProfile.json`.
2. Собрать файловую систему: `pio run -t buildfs -e esp32_4mb`.
3. Собрать прошивку: `pio run -e esp32_4mb`.
4. Записать на устройство (по USB/сети).

## Запуск веб-панели magicIoTm

Из каталога `tools/magicIoTm/`:

```
run.bat        # запуск сервера (python app.py)
restart.bat    # перезапуск
stop.bat       # остановка
stop.ps1       # остановка (PowerShell)
```

Либо вручную:

```bat
cd tools\magicIoTm
pip install -r requirements.txt
python app.py
```

Сервер доступен: **http://127.0.0.1:5005**

## Обычный рабочий цикл в панели

1. Открыть проект (автоматически открывается последний открытый).
2. Подготовить проект к сборке — этап «Подготовка профиля (PrepareProject)».
3. **Сборка**: сначала «Сборка файловой системы (buildfs)», затем «Сборка прошивки (build)».
4. Подключиться к устройству (раздел «Устройства»).
5. Записать файловую систему (FS) и/или прошивку (кнопка «🚀 Upload»).

## Замер размеров прошивки

```bash
# из папки measure_size с venv:
python measure.py --dry-run

# или из корня репозитория:
measure_size/venv/Scripts/python measure_size/measure.py --env esp32_4mb
```

Скрипт замеряет прирост размера прошивки (flash) для каждого модуля и пишет
результат в `about.size` файла `modinfo.json` (КБ по окружениям).
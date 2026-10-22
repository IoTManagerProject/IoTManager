# Справочник поддерживаемых платформ

> **Reference**. Окружения заданы в секциях `[env:*]` файла `platformio.ini`.
> `default_envs` не хранится в репозитории как договорённость: его задаёт профиль проекта —
> `myProfile.json` → `projectProp.platformio.default_envs` (в эталонном профиле —
> `esp32c3m_4mb`), `PrepareProject.py` переносит значение в `platformio.ini`.

## Платы

| Плата | Платформа | Flash | Примечания |
|---|---|---|---|
| `esp8266_1mb_ota` | NodeMCU v2 | 1MB | OTA, ldscript `1m64` (64 КБ FS), espressif8266 @4.0.1 |
| `esp8266_1mb` | NodeMCU v2 | 1MB | ldscript `1m256` (256 КБ FS) |
| `esp8285_1mb_ota` | ESP8285 | 1MB | OTA, ldscript `1m64` (64 КБ FS), espressif8266 @4.0.1 (макрос имени прошивки — `esp8266_1mb_ota`) |
| `esp8285_1mb` | ESP8285 | 1MB | Встроенная flash, ldscript `1m256` |
| `esp8266_2mb` | D1 WROOM | 2MB | 2MB flash / 1MB FS (`2m1m`) |
| `esp8266_2mb_ota` | D1 WROOM | 2MB | OTA, ldscript `2m256`; espressif8266 @4.2.0 |
| `esp8266_4mb` | NodeMCU v2 | 4MB | 4MB flash / 1MB FS (`4m1m`) |
| `esp8266_16mb` | NodeMCU v2 | 16MB | 16MB flash / 14MB FS (`16m14m`) |
| `esp32_4mb` | ESP32 Dev | 4MB | `debug_tool = esp-prog` |
| `esp32_4mb3f` | ESP32 Dev | 4MB | Кастомные партиции (`tools/partitions_custom.csv`) |
| `esp32_16mb` | ESP32 Dev | 16MB | `tools/large_spiffs_16MB.csv` |
| `esp32cam_4mb` | ESP32-CAM | 4MB | PSRAM |
| `esp32s2_4mb` | LOLIN S2 Mini | 4MB | USB CDC |
| `esp32s3_16mb` | ESP32-S3 DevKitC-1 | 16MB | Кастомные партиции (`tools/large_spiffs_16MB.csv`) |
| `esp32c3m_4mb` | LOLIN C3 Mini | 4MB | Flash-режим принудительно `dio` (см. Примечания) |
| `esp32c6_4mb` | ESP32-C6 DevKitM-1 | 4MB | espressif32 @6.9.0 |
| `esp32c6_8mb` | ESP32-C6 DevKitM-1 | 8MB | Кастомные партиции (`tools/partitions_custom_8mb.csv`) |
| `esp32_wifirep` | ESP32 Dev | 4MB | Tasmota platform (`platform-espressif32` 2.0.5.3) |
| `bk7231n` | BK7231N QFN32 | — | LibreTiny, имя прошивки `iotm_tiny`, только стандартный веб-сервер; OTA прошивки поддерживается, OTA образа ФС — нет (только copy/USB), см. [TODO.md](../../TODO.md) |

## По семействам

- **ESP8266 / ESP8285**: `esp8266_1mb`, `esp8266_1mb_ota`, `esp8266_2mb`, `esp8266_2mb_ota`,
  `esp8266_4mb`, `esp8266_16mb`, `esp8285_1mb`, `esp8285_1mb_ota`.
- **ESP32**: `esp32_4mb`, `esp32_4mb3f`, `esp32_16mb`, `esp32cam_4mb`, `esp32s2_4mb`,
  `esp32c3m_4mb`, `esp32s3_16mb`, `esp32_wifirep`, `esp32c6_4mb`, `esp32c6_8mb`.
- **BK7231N**: `bk7231n` (LibreTiny, имя прошивки `iotm_tiny`), кастомная ветка `libretiny` и `LT_WebSockets`.


## Примечания

- Сборка: `pio run -e <плата>`; загрузка: `pio run -e <плата> --target upload`.
- **ESP32-C3 (`esp32c3m_4mb`): обязателен `board_build.flash_mode = dio`.** Плата
  `lolin_c3_mini` объявляет `build.flash_mode = qio`, поэтому сборщик Arduino берёт набор
  предкомпилированного SDK/библиотек `tools/sdk/esp32c3/qio_qspi`
  (`CONFIG_ESPTOOLPY_FLASHMODE_QIO`), а образ и загрузчик PlatformIO пишет в режиме `dio`
  (`_get_board_flash_mode` в `builder/main.py` заменяет `qio`/`qout` на `dio`); каталог SDK
  выбирается как `build.flash_mode + "_qspi"` (`tools/platformio-build-esp32c3.py`).
  Рассинхрон виден только на железе: паника загрузки
  `assert failed: do_core_init startup.c:328 (flash_ret == ESP_OK)` — падение
  `esp_flash_init()` в приложении. С `board_build.flash_mode = dio` вариант SDK становится
  `tools/sdk/esp32c3/dio_qspi` и совпадает с режимом образа.
- **ESP32-C3: после смены flash-режима прошивать с полным стиранием** —
  `pio run -e esp32c3m_4mb -t erase`, затем `-t upload` (bootloader + партиции + приложение)
  и `-t uploadfs` (LittleFS стирается при `erase`).
- **Веб-сервер: асинхронный у всех плат, включая `bk7231n`.** Вариант задаётся
  единым переключателем `LT_ASYNC_WEB_SERVER` в `include/Const.h` (по умолчанию
  раскомментирован, т.е. async); флагов `-DASYNC_*` в `platformio.ini` больше нет.
  Точечно для одного окружения вариант переопределяется `-DASYNC_*`/`-DSTANDARD_*`
  флагами его `build_flags` (тогда переключатель для него не участвует).
  Библиотеки ESP-плат: `ESP32Async/ESPAsyncWebServer`
  (общий `lib_deps_external`) + `ESP32Async/ESPAsyncTCP` в `lib_deps` каждой
  ESP8266/ESP8285-env; у ESP32/ESP32-S2/S3/C3/C6 `AsyncTCP` приходит зависимостью
  `ESPAsyncWebServer`. Для `bk7231n` — esphome-форки в `lib_deps` его секции
  (см. [articles/async-web-server.md](../articles/async-web-server.md), раздел 6.1).
  Стандартный вариант (`STANDARD_WEB_SERVER` + `STANDARD_WEB_SOCKETS`,
  `WebServer`/`ESP8266WebServer` + `WebSocketsServer`) сохранён в коде как **резервный**:
  включается закомментированием переключателя (глобально) либо `-DSTANDARD_*` флагами
  окружения. Готовых окружений `esp8266_4mb_async` / `esp32s2_4mb_async` в `platformio.ini`
  **нет** — вариант общий для всех сборок (коммит `f0d80db6`). Подробнее —
  [guides/build-firmware.md](../guides/build-firmware.md#вариант-веб-сервера-асинхронный-по-умолчанию-или-стандартный).
- Файловая система: LittleFS (`platformio.ini`: `filesystem = littlefs`, `data_dir = data_svelte`).
- После успешной сборки в `iotm/` остаётся только одна папка с прошивками (текущей платформы) — см. [ADR-0002](../decisions/0002-data-svelte-canonical-path.md).
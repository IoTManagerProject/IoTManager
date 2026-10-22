# Справочник API веб-панели magicIoTm

> **Reference**. Все маршруты — с префиксом `/api`. SSE-потоки отдают `text/event-stream`.
> Панель — локальный инструмент без аутентификации.

## Проекты и конфигурация

| Метод | Путь | Описание |
|---|---|---|
| GET | `/projects` | Дерево проектов + описания |
| GET | `/projects/data-svelte` | Проекты, у которых есть `data_svelte` (кроме PlatformIO) |
| POST | `/projects/category` | Создать категорию |
| DELETE | `/projects/category/<name>` | Удалить категорию |
| POST | `/projects/category/<name>/rename` | Переименовать категорию |
| POST | `/projects/create` | Создать проект |
| DELETE | `/projects/<cat>/<name>` | Удалить проект |
| POST | `/projects/<cat>/<name>/rename` | Переименовать проект |
| POST | `/projects/copy` | Копировать проект |
| POST | `/projects/move` | Перенести проект |
| POST | `/projects/<cat>/<name>/open` | Открыть проект |
| POST | `/platformio/open` | Открыть защищённый проект PlatformIO (корневой `myProfile.json`) |
| GET | `/projects/last` | Последний открытый |
| GET | `/projects/list-all` | Список всех (для копирования) |
| GET | `/projects/<cat>/<name>/settings` | `iotmSettings` проекта |
| POST | `/projects/copy-settings` | Копировать iotmSettings |
| POST | `/projects/copy-modules` | Копировать modules |
| POST | `/config/import-root` | Импорт из корневого `myProfile.json` в текущий проект |
| POST | `/projects/repair-data-dir` | Перезаписать `[platformio] data_dir` проекта на `<проект>/data_svelte` (`{category, name}`) |
| GET | `/projects/<cat>/<name>/tree/fs` | Дерево файлов FS проекта (`data_svelte`) |
| GET | `/projects/<cat>/<name>/file/fs?path=` | Содержимое файла из `data_svelte` проекта (`?meta=1` — только имя/размер, в т.ч. для бинарных `*.gz`/`favicon.ico`) |
| POST | `/projects/<cat>/<name>/file/fs` | Сохранить файл в `data_svelte` проекта (`{path, content}`; бинарные `*.gz`/`favicon.ico` не записываются) |
| GET | `/projects/<cat>/<name>/file/sources?mode=file\|all&path=` | Доступные источники «получить из устройства»: по каждому устройству — есть ли сохранённые RAM/FS (`ram_saved`/`fs_saved`) и в сети ли оно (`ram_live`/`fs_live`, можно тянуть напрямую; `ram_live` — только для файлов, которые прошивка отдаёт по WS из набора RAM: `config.json`, `items.json`, `widgets.json`, `scenario.txt`, `settings.json`, `ota.json`, `profile.json`) |
| POST | `/projects/<cat>/<name>/file/from-device` | Получить файл/файлы из устройства в `data_svelte` проекта (`{device_key, section, live, all, path}`) |
| GET | `/config` | Текущая конфигурация |
| POST | `/config/save` | Сохранить конфигурацию |
| POST | `/config/settings` | Сохранить настройки |
| POST | `/config/about` | Сохранить описание |
| GET | `/config/export` | Экспорт в JSON (имя файла `myProfile_<ts>.json`) |

## Платформы, модули, размеры

| Метод | Путь | Описание |
|---|---|---|
| GET | `/platforms` | Список платформ (env) + baseline_flash |
| POST | `/platform/change` | Смена платформы (автоотключение несовместимых) |
| GET | `/size` | Размеры FLASH/RAM/FS (проценты и байты) |
| POST | `/modules/toggle` | Включить/выключить модуль |
| POST | `/modules/sync` | Пакетное обновление активных модулей |
| GET | `/modules/compatibility` | Карта совместимости и статусов размеров модулей для текущей платформы (`sizeState`: `ok` / `error_platform` / `error_all` / `missing`) |
| POST | `/modules/reload` | Перезагрузить кэши modinfo и platforms (после замера размеров) |
| POST | `/modules/info` | Информация о модуле (about, usedLibs, usedFLASH, usedRAM) |

## Сборка и замер

| Метод | Путь | Описание |
|---|---|---|
| POST | `/build/start` | Запустить сборку (сохраняет конфиг) |
| GET | `/build/stream` | SSE-поток событий сборки |
| POST | `/measure/start` | Запустить замер размера (scope: all/module/baseline/profile/without; для baseline — platforms_scope: current/all) |
| GET | `/measure/stream` | SSE-поток событий замера |
| POST | `/measure/abort` | Мягко прервать замер |

## Валидация и инструменты

| Метод | Путь | Описание |
|---|---|---|
| GET | `/validate/name` | Валидация имени устройства |
| GET | `/validate/apssid` | Валидация AP SSID |
| GET | `/tools/esptool` | Статус esptool (установлен/версия/актуальность) |
| POST | `/tools/esptool/install` | Автоустановка esptool |
| POST | `/tools/esptool/update` | Обновление esptool |
| GET | `/tools/platformio` | Статус PlatformIO (найден ли `pio`, путь, версия, актуальность) |
| POST | `/tools/platformio/install` | Установка PlatformIO, если `pio` не найден (в окружение панели) |
| POST | `/tools/platformio/update` | Обновление PlatformIO |
| GET | `/tools/ports` | COM-порты с определённым ESP-чипом |
| GET | `/scenario/functions` | Списки функций для проверки scenario.txt (`reserved` + `bySubtype`) |

## Прошивка (USB и OTA)

| Метод | Путь | Описание |
|---|---|---|
| GET | `/upload/status` | Готовность к USB-прошивке (собрана ли прошивка, семейство чипа) |
| POST | `/upload/detect` | Определить подключённый ESP-чип, проверить соответствие платформе |
| POST | `/upload/start` | Запустить USB-прошивку (`mode`: fs/firmware/full, `port`: COM) |
| GET | `/upload/stream` | SSE-поток USB-прошивки |
| GET | `/ota/candidates` | Список онлайн-устройств для OTA (совместимость платформ) |
| POST | `/ota/start` | Запустить OTA (`mode`: fs/firmware/full, `fs_method`: flash/copy) |
| GET | `/ota/stream` | SSE-поток OTA |

Особенности `POST /upload/start`:

- `mode`: `fs` | `firmware` | `full`; при `fs`/`full` образ LittleFS собирается из
  `[platformio] data_dir` проекта (`mklittlefs -c $PROJECT_DATA_DIR`).
- Если в `platformio.ini` указан устаревший/несуществующий `data_dir` (например, проект
  перенесли в другую категорию), ответ — `409` с `code: "data_dir"`, полями `ini_value`,
  `resolved`, `expected` и `project`. UI показывает модалку «Исправить и прошить» и
  вызывает `POST /projects/repair-data-dir`.

## Устройства

| Метод | Путь | Описание |
|---|---|---|
| GET | `/devices` | Список устройств со статусами (online/offline, цвет) |
| GET | `/devices/list-all` | Все папки устройств + наличие settings/profile в RAM/FS |
| GET | `/devices/stream` | SSE-поток изменений списка устройств |
| GET | `/devices/interfaces` | Сетевые интерфейсы для выбора подсети |
| POST | `/devices/scan` | Запустить сканирование сети (опц. `{"subnet": "..."}` или `"__all__"`) |
| GET | `/devices/scan/stream` | SSE-поток прогресса сканирования |
| POST | `/devices/add` | Добавить устройство по IP |
| GET | `/devices/wifi` | Состояние WiFi-антенны: все сети эфира (`all`, сети модулей `iotm*` — впереди), AP-сети модулей (`networks`), счётчик новых, статус подключения |
| GET | `/devices/wifi/stream` | SSE-поток состояния WiFi-антенны (кадры — только при изменениях) |
| POST | `/devices/wifi/scan` | Скан эфира по требованию (не ждать очередного цикла потока) |
| POST | `/devices/wifi/seen` | Погасить счётчик «новых» сетей: список показан пользователю (`{}` или `{"ssids": [...]}`) |
| POST | `/devices/wifi/connect` | Подключить панель к AP-сети модуля и найти его: `{ssid, password?}`; фон, ход — в `connect.stage` |
| POST | `/devices/wifi/return` | Вернуть панель в «домашнюю» сеть (сохранённый профиль обычной сети) после работы в AP модуля |
| GET | `/devices/wifi/passwords` | Пароли AP для подстановки: `platformio` — корневой `myProfile.json` («PlatformIO», показывается первой строкой до групп), `projects` — проекты (`myProfile.json` → `iotmSettings.apssid/appass`), `devices` — устройства (`settings.json` из RAM/FS → `apssid/appass`); элементы `{label, ssid, pass}` |
| DELETE | `/device/<device_key>` | Удалить устройство (папка + записи) |
| POST | `/device/<device_key>/copy-to-project` | Копировать устройство в проект (существующий или создаётся из шаблона): `{dst_cat, dst_name, section: ram\|fs}`; правила кнопок «⤵ Из устройства»: settings.json → iotmSettings (кроме id/ip/root), profile.json/flashProfile.json → платформа из default_envs (несовместимые модули отключаются) и активность модулей по path |
| GET | `/device/<device_key>/info` | Инфо устройства и путь к папке |
| POST | `/device/<device_key>/fetch/<ram\|fs>` | Скачать раздел RAM/FS в фоне |
| GET | `/device/<device_key>/fetch/<section>/status` | Статус/прогресс скачивания |
| GET | `/device/<device_key>/tree/<ram\|fs>` | Дерево файлов раздела |
| GET | `/device/<device_key>/file/<ram\|fs>?path=` | Содержимое файла (`?meta=1` — только имя/размер) |
| POST | `/device/<device_key>/file/<ram\|fs>` | Сохранить файл локально |
| POST | `/device/<device_key>/write/ram` | Записать файл обратно на устройство (обратные WS-команды; только поддерживаемые прошивкой файлы) |
| POST | `/device/<device_key>/fetch/<ram\|fs>/file` | Скачать один файл с устройства в папку устройства (`{path}`; RAM — по WS, FS — по HTTP) |
| POST | `/device/<device_key>/ping` | Ручной пинг устройства: `{success, online}` по ICMP (для устройств «не в сети» перед получением файлов); FSM-статус не меняет |
| GET | `/device/settings?key=&section=` | settings.json устройства (RAM/FS) |
| GET | `/device/profile?key=&section=` | Профиль устройства: modules + default_envs (RAM: profile.json, FS: flashProfile.json) |
| POST | `/device/<device_key>/reboot` | Перезагрузить устройство (WS `/reboot\|`) |

### WiFi-антенна: точки доступа модулей (`iotm*`)

Реализация — `tools/magicIoTm/core/wifi.py`, запуск потока — в `init()` (`app.py`).
Модуль в режиме точки доступа поднимает сеть с именем из `apssid` (у
пользовательских сборок префикс `iotm`) и адресом `192.168.4.1`
(`src/utils/WiFiUtils.cpp`).

1. Поток `wifi-scan-worker` сразу после старта панели (и далее каждые
   `WIFI_SCAN_INTERVAL`, 60 с) выполняет `netsh wlan show networks mode=bssid`
   и складывает в `_wifi_scan_state` две выборки: `networks` — только сети с
   префиксом `iotm` (регистр не важен, AP-режим модулей), и `all` — **все** сети
   эфира, где сети `iotm*` идут впереди прочих (внутри групп — по убыванию
   сигнала, у каждой записи флаг `iotm`). Модалка показывает `all`, поэтому под
   рукой сначала точки доступа модулей, а ниже — остальные сети (к любой можно
   подключиться). Первый скан запускается без ожидания очередного цикла:
   поток делает его через `WIFI_FIRST_SCAN_DELAY` (1 с), а если запрос состояния
   пришёл раньше — его выполнит `ensure_first_scan()` (один раз, идемпотентно) на
   `GET /devices/wifi` / первом кадре SSE. Появление новой сети (`new`) — сигнал
   для UI: счётчик и подсветка кнопки «!», которая стоит в правом углу шапки списка
   устройств и видима всегда (чтобы список AP-сетей был доступен независимо от
   состояния сканера). Повторные сканы не чаще
   `MIN_RESCAN_GAP` (5 с) и никогда во время смены WiFi-сети.
2. Открытие модалки (`POST /devices/wifi/seen`) гасит счётчик: SSID уходит в
   историю `seen` (не более `SEEN_LIMIT` записей).
3. `POST /devices/wifi/connect {ssid, password?}` в отдельном потоке
   `wifi-connect`: создаёт WiFi-профиль (открытая сеть — `open`, иначе
   `WPA2PSK`/`AES`, ключ — элементом `<sharedKey><keyType>passPhrase</keyType>`;
   не-ASCII SSID дополнительно задаётся байтами в `<hex>`) → `netsh wlan add profile` → `netsh wlan connect` →
   ожидание ассоциации (`CONNECT_WAIT`, 25 с; лимит не расходуется, если журнал
   `Microsoft-Windows-WLAN-AutoConfig/Operational` раньше сообщает о сбое —
   событие `8002` с SSID сети, опрос каждые `WLAN_FAIL_POLL` через `wevtutil`,
   причина сразу уходит в `connect.error`) → проверка, что адрес панели попал
   в `192.168.4.0/24` (иначе ошибка называет и ожидаемую подсеть модуля, и
   адреса панели) → `ping 192.168.4.1` (до `PING_ATTEMPTS` попыток) →
   `add_device_by_ip()` → устройство появляется в дереве
   по SSE `/devices/stream`. `add_device_by_ip()` берёт имя и id из `settings.json`
   (`fetch_identity_with_retry` → `_fetch_settings_identity`, WebSocket-порт 81) с
   повторами (`AP_IDENTITY_ATTEMPTS` × `AP_IDENTITY_DELAY`): сразу после ассоциации
   модуль отвечает не с первого раза. Прошивка всегда пишет в `settings.json`
   `id` (= `getChipId`) и `name`, поэтому папка формируется по ключу `<name> <id>` —
   как у устройств, найденных multicast-пакетом; папка «по IP» не создаётся.
   `/devlist.json` — запасной источник имени/id. Если панель подключилась к обычной
   сети (адрес вне `192.168.4.0/24`) — это не ошибка: устройство просто не
   добавляется. Ход стадий — в `connect.stage`, итог — в
   `connect.error` / `connect.result`.
4. Сканирование приостанавливается на время подключения (состояние
   `connect.running`), чтобы не мешать ассоциации.
5. **Возврат в домашнюю сеть.** Перед подключением к AP (и при каждом скане)
   панель запоминает последнюю обычную сеть — не с префиксом `iotm` — в поле
   `home` (`{ssid, profile}`). Пока текущее подключение не совпадает с `home`,
   кнопка «!» подсвечивается, а в модалке появляется кнопка
   «🏠 <домашняя сеть>»: `POST /devices/wifi/return` выполняет
   `netsh wlan disconnect` + `netsh wlan connect` по сохранённому профилю
   (пароль не нужен, повторный профиль не создаётся). Если панель ни разу не
   работала в обычной сети, эндпоинт отвечает `400`.

Формат кадра SSE (`/devices/wifi` и `/devices/wifi/stream` одинаковы):

```json
{"supported": true, "backend": "netsh", "networks": [
   {"ssid": "iotm-24ABCD", "signal": 94, "channel": 6,
    "secured": false, "auth": "open", "bssid": "aa:bb:cc:dd:ee:ff", "iotm": true}],
 "all": [
   {"ssid": "iotm-24ABCD", "signal": 94, "channel": 6,
    "secured": false, "auth": "open", "bssid": "aa:bb:cc:dd:ee:ff", "iotm": true},
   {"ssid": "HomeWiFi", "signal": 71, "channel": 11,
    "secured": true, "auth": "WPA2-Personal", "bssid": "11:22:33:44:55:66", "iotm": false}],
 "new": ["iotm-24ABCD"], "new_count": 1, "seen_count": 0, "alert": true,
 "total": 2,
 "connected": "HomeWiFi", "home": {"ssid": "HomeWiFi", "profile": "HomeWiFi", "ts": 1789000000.0},
 "last_scan": 1789000000.0, "scans": 12,
 "interval": 60.0, "next_scan": 1789000060.0, "error": null,
 "connect": {"running": false, "ssid": "", "mode": "connect", "stage": "",
             "error": null, "result": null, "started": 0.0}}
```

Замечания:

- `networks` — только AP-сети модулей (`iotm*`), `all` — все сети эфира, где
  `iotm*` идут впереди (у каждой записи флаг `iotm`). Счётчики `new`/`alert` и
  прогрев кнопки «!» считаются по `networks`. Модалка показывает `all`.
- **UI модалки.** Список сетей открывается кнопкой «!» и не закрывается кликом мимо
  окна (только кнопкой «Закрыть»). Поле пароля появляется лишь для выбранной
  защищённой сети (`secured`), для открытой пароль не отправляется. Если сетей в
  эфире меньше двух, под списком выводится подсказка обновить список средствами ОС.

- Вывод `netsh` локализован, поэтому разбор не зависит от языка: ключи `SSID` /
  `BSSID` не переводятся, сигнал ищется по значению «NN%», канал и тип
  аутентификации — по списку синонимов; декодирование — UTF-8 → OEM-кодировка
  консоли (`GetOEMCP`, для ru — cp866) → cp1251.
- Поле `total` в кадре — **общее число сетей в эфире**. Оно нужно, чтобы отличать две
  внешне одинаковые ситуации («списка iotm* нет»): модуль не в режиме точки доступа
  (выключен или уже в домашней сети — `total > 0`, `networks` пуст) и сам скан не
  работает (WiFi выключен, netsh недоступен — `total = 0`). В журнал при этом пишется
  `WiFi-скан: AP-сетей iotm* не найдено (в эфире всего сетей: N, например: …)`.
- `netsh wlan show networks` сам по себе поиск **не инициирует** — печатает кэш
  драйвера. Перед чтением вывода панель пытается запустить активный поиск через
  `WlanScan` (`core/wifi.py`, `trigger_scan`): `WlanOpenHandle` →
  `WlanEnumInterfaces` (интерфейс в состоянии `connected`) → `WlanScan` → пауза
  `SCAN_SETTLE` (2.5 с) на обновление кэша → чтение `netsh`. Между вызовами
  выдерживается пауза `SCAN_API_MIN_GAP` (8 с), кнопка «Обновить» её игнорирует
  (`scan_once(force=True)`). Если WlanScan недоступен (нет `wlanapi.dll`, не
  Windows, драйвер/WlanSvc/policy отказали — типичный код `87`,
  `ERROR_INVALID_PARAMETER`), то после `SCAN_API_GIVE_UP` (3) отказов попытки
  прекращаются, в журнал пишется `Активный скан эфира (WlanScan) недоступен …`,
  и панель читает кэш netsh — то есть работает как раньше, просто без
  принудительного обновления эфира.
- **Если список сетей пуст минутами подряд — перезапустите службу WLAN.** Практический
  признак «мёртвой» службы автонастройки: `netsh wlan show networks` возвращает
  ровно одну сеть (текущую) и не меняет этого десятки секунд; `WlanEnumInterfaces` при
  этом не отдаёт размер буфера, и WLAN API отказывает целиком (`WlanScan` → 87,
  `WlanQueryInterface` → 87). Это не права и не настройки панели: тот же симптом
  воспроизводился и в Python, и в PowerShell, и с правами администратора. Лечение —
  PowerShell от администратора:
  `Restart-Service WlanSvc -Force; Start-Service NlaSvc`
  (Wi-Fi на пару секунд пропадает, затем поднимается по сохранённому профилю). После
  перезапуска `netsh` снова показывает весь эфир, и панель видит AP-сети модулей.
- Linux-резерв — `nmcli` (только просмотр сетей): автоподключение реализовано
  для Windows, на других ОС `POST /devices/wifi/connect` возвращает ошибку.
- Переход в сеть модуля означает, что панель на время теряет обычную сеть
  (адрес проекта с телефона/другой машины недоступен) — модалка предупреждает об
  этом, а кнопка «🏠 Домашняя сеть» возвращает панель в сохранённую обычную сеть
  (`POST /devices/wifi/return`).
- `connect.mode` отличает операцию: `connect` — переход в AP модуля (с созданием
  профиля и поиском устройства), `return` — возврат в домашнюю сеть.

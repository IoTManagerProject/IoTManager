# MagicIoTm — конфигуратор прошивок IoTManager

Веб-приложение на Python **Flask** для управления проектами и конфигурациями
IoT-устройств (**ESP8266 / ESP32** и совместимых платформ). Запустите `run.bat`
и откройте http://127.0.0.1:5005 — панель найдёт устройства с IoTManager-прошивкой
в локальной сети, позволит настроить и собрать прошивку (**PlatformIO**), записать её
по **USB** или **OTA** и редактировать конфигурацию устройства.

## Возможности

- **Управление проектами**: категории и проекты — создание/удаление/переименование/
  копирование/перенос. Защищённый проект PlatformIO работает напрямую с корневым
  `myProfile.json` (его нельзя переименовать, перенести или удалить). Автобэкап
  в `projects/.backups`, автооткрытие последнего проекта.
- **Конфигурация**: редактирование `iotmSettings`, модули через чекбоксы (`active`),
  копирование настроек/модулей из других проектов, импорт базового конфига из
  корневого `myProfile.json`, экспорт в JSON.
- **Валидация**: имя устройства — ≤ 64 символов, без пробелов, уникально в пределах
  всех проектов; AP SSID — уникален (может быть пустым).
- **Платформы**: выбор из `[env:*]` секций `platformio.ini`; при смене платформы
  несовместимые модули отключаются автоматически (по `usedLibs` в `modinfo.json`).
- **Индикаторы памяти**: FLASH / RAM / FS на основе `modinfo.json` и `platforms.json`.
- **Сборка**: PrepareProject → buildfs → build с поэтапным SSE-логом и расчётом размеров.
- **Замер размеров**: `measure_size/measure.py` в фоне (все/отдельный/baseline/профиль/без),
  SSE-прогресс, обновление кэшей `modinfo`/`platforms`.
- **Прошивка по USB**: esptool (проверка/установка/обновление), определение чипа,
  режимы FS/прошивка/full, защита при нехватке flash.
- **OTA**: режимы `firmware`/`fs`/`full`; запись FS образом (`flash`) или пофайлово
  (`copy`); локальный HTTP-сервер `.bin`; проверка совместимости платформы.
- **Менеджер устройств**: обнаружение по UDP-multicast (239.255.255.255:4210),
  фоновый пинг, сканирование сети, ручное добавление по IP, разделы **RAM** и **FS**
  (дерево + редактор с подсветкой), скачивание файлов с прогрессом, копирование
  настроек с устройства в проект.
- **Статусы устройств**: 🟢 зелёный / ⚪ серый / 🟡 жёлтый / 🔴 красный — конечный
  автомат (подробно: `docs/reference/device-status-fsm.md`).
- **Проверка scenario.txt**: скобки, if/then/else, циклические зависимости,
  неизвестные переменные относительно `config.json`, встроенные функции `ID.функция()`.

## Структура проекта

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
├── run.bat / restart.bat / stop.bat / stop.ps1
└── requirements.txt        # flask>=3.0, flask-cors>=4.0
```

## Быстрый старт

```
run.bat        # Windows: запуск сервера (python app.py)
restart.bat    # перезапуск
stop.bat       # остановка
# вручную: pip install -r requirements.txt && python app.py
```

Сервер: **http://127.0.0.1:5005** (см. `docs/guides/build-firmware.md`).

## Ограничения

- Панель — локальный инструмент **без аутентификации**; CORS открыт; возможен path
  traversal. Используйте на доверенной машине и в доверенной сети.
- Работает на **Windows** (`.bat`, `netsh` для SSID, `ping -n`); на Unix сборка/прошивка
  работают, часть утилит адаптирована под Windows.
- Замер и сборка не могут выполняться одновременно (взаимная блокировка в API).
- Размер файла для записи на устройство ограничен 50 КБ (один WebSocket-фрейм ESP).

## Документация

- `docs/reference/magiciotm-api.md` — справочник API (все маршруты `/api/*`, SSE-потоки)
- `docs/reference/device-status-fsm.md` — конечный автомат статусов устройств
- `docs/guides/use-magiciotm.md` — руководство по разделам панели
- `docs/architecture.md` — архитектура панели и прошивки
- `../AGENTS.md` — контракт репозитория (правила, риски, команды)
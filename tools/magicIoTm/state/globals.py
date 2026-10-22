"""
Глобальное состояние приложения (синглтоны).

Все изменяемые глобальные переменные вынесены сюда для удобства
и предотвращения рассинхрона между модулями.
"""

import threading

# ==================== Проектное состояние ====================
current_project = None          # {"category": ..., "name": ...}
current_config = None           # dict — полная конфигурация проекта
current_platform = "esp8266_4mb"  # текущая платформа (env)

# ==================== Кэши ====================
modinfo_cache = {}              # module_name -> {usedFLASH, usedRAM, usedLibs, about}
platforms_cache = {}            # env -> {baseline_flash, total_flash, baseline_ram, total_ram, total_fs}

# ==================== Общеприменимые блокировки ====================
_lock = threading.Lock()        # для записи current_config

# ==================== Состояние сканирования ====================
_scan_lock = threading.Lock()
_scan_state = {
    "running": False,
    "total": 0,
    "done": 0,
    "subnet": "",
    "alive": [],
    "found": [],
    "failed": [],
    "error": None,
}

# ==================== Состояние WiFi-сканера ====================
# Периодический поиск точек доступа IoTManager (SSID с префиксом iotm):
# панели достаточно одного раза в минуту, чаще — только по кнопке «Обновить».
_wifi_scan_lock = threading.Lock()
_wifi_scan_state = {
    "supported": True,        # доступна ли WiFi-подсистема ОС (netsh/nmcli)
    "backend": None,          # 'netsh' | 'nmcli' | None
    "networks": [],           # найденные сети iotm*: {ssid, signal, channel, secured, auth, bssid}
    "total": 0,               # сетей в эфире всего (диагностика: скан работает ли)
    "new": [],                # SSID, ещё не показанные пользователю (счётчик для кнопки «!»)
    "seen": [],               # SSID, уже показанные в модалке (история, ограничена SEEN_LIMIT)
    "connected": "",          # SSID текущего подключения панели
    # «домашняя» сеть (не AP модуля) — для возврата кнопкой «🏠»
    "home": {"ssid": "", "profile": "", "ts": 0.0},
    "last_scan": 0.0,         # время последнего сканирования
    "scans": 0,               # число выполненных сканирований
    "error": None,            # последняя ошибка сканирования
}

# Подключение к выбранной сети выполняется в отдельном потоке (netsh + ожидание
# ассоциации + пинг 192.168.4.1 — занимает несколько десятков секунд).
_wifi_connect_lock = threading.Lock()
_wifi_connect_state = {
    "running": False,
    "ssid": "",
    "mode": "connect",        # 'connect' — к AP модуля, 'return' — назад в домашнюю сеть
    "stage": "",              # текущая стадия (текст для UI)
    "error": None,
    "result": None,           # добавленное устройство (device из add_device_by_ip)
    "started": 0.0,
}

# ==================== Состояние устройств ====================
_devices_lock = threading.Lock()
_devices = {}                   # ip -> {ip, name, wg, id, status, fv, last_seen}

_device_folders_lock = threading.Lock()
_device_folders = {}            # key -> {folder, ram_dir, fs_dir, name, ip}

_device_states_lock = threading.Lock()
_device_states = {}             # key -> {state, fails, last_confirm}

# ==================== Потоки ====================
_device_thread = None           # multicast listener
_ping_thread = None             # ping worker
_scan_thread = None             # network scanner
_wifi_thread = None             # wifi scan worker (поиск AP-сетей iotm*)
_wifi_connect_thread = None     # подключение к выбранной WiFi-сети

# ==================== Скачивание разделов ====================
_fetch_progress = {}            # (device_key, section) -> {stage, done, total, name, files, error, ip}

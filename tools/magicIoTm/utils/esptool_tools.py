"""
esptool_tools — проверка установки и актуальности esptool для magicIoTm.

Модуль самодостаточен: не требует циклических импортов из app.py.
На старте приложения вызывается `startup_check()` (неблокирующий фоновый
поток), который:
  1. определяет, установлен ли esptool (и его версию);
  2. запрашивает актуальную версию с PyPI (официальный источник pip-пакета);
  3. кэширует результат и пишет сводку в лог.

Состояние доступно через `status()` для показа в API/UI.

Замечание: сетевые запросы и импорт esptool обёрнуты в try/except, поэтому
любые ошибки (нет сети, пакет не установлен) не блокируют запуск сервера.
"""

import importlib
import json
import logging
import re
import subprocess
import sys
import threading
import time
import urllib.request

logger = logging.getLogger(__name__)

# Официальный источник версии: PyPI JSON API (канал обновления pip-пакета)
PYPI_URL = "https://pypi.org/pypi/esptool/json"
# Минимальная таргетная версия (для сообщения об установке)
ESPTool_VERSION = "4.7.1"
CHECK_TIMEOUT = 5  # секунд на сетевой запрос


# ==================== Кэш состояния ====================

_status = {
    "available": False,
    "installed_version": None,
    "latest_version": None,
    "needs_update": None,
    "source": PYPI_URL,
    "last_checked": None,
    "error": None,
}
_lock = threading.Lock()


def get_installed():
    """Определяет, установлен ли esptool, и возвращает его версию.

    Используется lazy-импорт через importlib, чтобы отсутствие пакета
    не приводило к падению при старте. Версия берётся из атрибута
    `esptool.__version__` (есть в официальном пакете).

    Returns:
        dict: {"available": bool, "version": str | None}
    """
    try:
        esptool = importlib.import_module("esptool")
        version = getattr(esptool, "__version__", None)
        return {"available": True, "version": version}
    except Exception as e:  # ImportError и любые ошибки инициализации
        logger.debug(f"esptool не доступен: {e}")
        return {"available": False, "version": None}


def check_latest():
    """Запрашивает актуальную версию esptool с PyPI.

    Returns:
        dict: {"latest_version": str | None, "error": str | None}
    """
    try:
        req = urllib.request.Request(
            PYPI_URL,
            headers={"User-Agent": "magicIoTm/1.0"},
        )
        with urllib.request.urlopen(req, timeout=CHECK_TIMEOUT) as resp:
            data = json.loads(resp.read().decode("utf-8"))
        latest = data.get("info", {}).get("version")
        if not latest:
            return {"latest_version": None, "error": "Поле info.version отсутствует в ответе PyPI"}
        return {"latest_version": latest, "error": None}
    except Exception as e:
        return {"latest_version": None, "error": str(e)}


def _version_tuple(v):
    """Приводит строку версии к кортежу int для сравнения (или None, если некорректна)."""
    if not v:
        return None
    try:
        parts = re.split(r"[^\d]+", v.strip())
        nums = [int(p) for p in parts if p]
        return tuple(nums) if nums else None
    except Exception:
        return None


def _run_check():
    """Полный цикл проверки: установлен ли esptool и актуальность версии."""
    global _status

    installed = get_installed()
    latest_res = check_latest()

    installed_version = installed["version"]
    latest_version = latest_res["latest_version"]

    needs_update = None
    if installed_version and latest_version:
        inst_t = _version_tuple(installed_version)
        lat_t = _version_tuple(latest_version)
        if inst_t and lat_t:
            needs_update = inst_t < lat_t

    with _lock:
        _status.update({
            "available": installed["available"],
            "installed_version": installed_version,
            "latest_version": latest_version,
            "needs_update": needs_update,
            "last_checked": time.time(),
            "error": latest_res["error"],
        })

    _log_summary()


def _log_summary():
    """Пишет итог проверки в лог."""
    with _lock:
        s = dict(_status)

    if not s["available"]:
        logger.warning(
            "esptool не установлен. Установите: pip install esptool>=%s "
            "(см. tools/magicIoTm/requirements.txt)",
            ESPTool_VERSION,
        )
        return

    base = f"esptool {s['installed_version'] or '?':<6} "
    if s["latest_version"] is None:
        logger.info(f"{base}| последнюю версию проверить не удалось (нет сети?)")
    elif s["needs_update"]:
        logger.info(f"{base}| доступна новая версия {s['latest_version']} (pip install -U esptool)")
    else:
        logger.info(f"{base}| актуальная версия")


def startup_check():
    """Запускает проверку esptool в фоновом потоке (неблокирующе).

    Если esptool отсутствует — автоматически скачивает его БЕЗ подтверждения
    (требование ТЗ). Обновление до новой версии остаётся только по согласию
    пользователя (через модальное окно в UI).
    """
    def _worker():
        if not get_installed()["available"]:
            _install_and_check()
        else:
            _run_check()
    thread = threading.Thread(target=_worker, daemon=True, name="esptool-check")
    thread.start()
    logger.info("Проверка esptool запущена в фоне...")


def status():
    """Возвращает снимок состояния проверки esptool для API/UI.

    Returns:
        dict: available, installed_version, latest_version, needs_update,
              source, last_checked, error
    """
    with _lock:
        return dict(_status)


# ==================== Установка / обновление ====================

def _pip(*args):
    """Запускает pip через sys.executable. Возвращает (rc, полный вывод)."""
    cmd = [sys.executable, "-m", "pip"] + list(args)
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=300,
                           encoding="utf-8", errors="replace")
        return p.returncode, (p.stdout or "") + (p.stderr or "")
    except Exception as e:
        logger.error(f"Не удалось запустить pip {args}: {e}")
        return -1, str(e)


def _install_and_check():
    """Устанавливает esptool (без подтверждения) и обновляет кэш состояния."""
    logger.info("esptool не найден — автоматическая установка (без подтверждения)...")
    rc, out = _pip("install", f"esptool>={ESPTool_VERSION}")
    if rc != 0:
        logger.error("Автоустановка esptool не удалась: %s", out[-2000:])
    else:
        logger.info("esptool установлен автоматически")
    _run_check()
    return status()


def ensure_installed():
    """Автоматически устанавливает esptool, если он отсутствует.

    По ТЗ установка выполняется без подтверждения пользователя (тихо).
    Обновление же требует согласия — см. ensure_updated().

    Returns:
        dict: результат status() после установки/проверки.
    """
    if get_installed()["available"]:
        return status()
    return _install_and_check()


def ensure_updated():
    """Обновляет esptool до актуальной версии (вызывается ПОСЛЕ согласия пользователя)."""
    logger.info("Обновление esptool по запросу пользователя...")
    rc, out = _pip("install", "-U", "esptool")
    if rc != 0:
        return {"success": False, "error": out[-2000:] or "неизвестная ошибка pip"}
    _run_check()
    return {"success": True}


# ==================== Определение чипов на COM-портах ====================

def _comports():
    """Возвращает список COM-портов или [] при недоступности pyserial."""
    try:
        from serial.tools import list_ports
        return list(list_ports.comports())
    except Exception as e:
        logger.debug(f"Не удалось перечислить порты (нет pyserial?): {e}")
        return []


def flash_label_to_bytes(label):
    """Преобразует строку объёма из esptool (например '4MB', '512KB') в байты.

    Returns:
        int | None: размер в байтах или None, если строка не распознана.
    """
    m = re.match(r"(\d+)\s*(MB|KB)", (label or "").strip(), re.IGNORECASE)
    if not m:
        return None
    n = int(m.group(1))
    unit = m.group(2).upper()
    return n * (1024 * 1024 if unit == "MB" else 1024)


def _port_identity(comp):
    """Стабильная «топология» USB‑устройства для отслеживания re‑enumeration.

    ESP32‑S2 с нативным USB переключается между режимом приложения (например
    COM6, PID 80C2) и режимом загрузки (COM7, PID 0002): меняются именно
    имя порта и PID, а VID, серийный номер и location остаются теми же.
    Именно по ним сопоставляем «старый» и «новый» порт одного устройства.
    """
    return (
        getattr(comp, "vid", None),
        getattr(comp, "serial_number", "") or "",
        getattr(comp, "location", "") or "",
    )


def _same_usb_device(a, b):
    """True, если это одно и то же USB‑устройство (игнорируем переключение PID)."""
    if a is None or b is None:
        return a is b
    if a is b:
        return True
    return _port_identity(a) == _port_identity(b)


def _flash_id_output(port):
    """Запускает `esptool --port <port> flash_id`, возвращает (stdout+stderr, rc).

    Оборачивает subprocess.run в try/except, чтобы таймаут/исключения esptool
    не падали наружу.
    """
    cmd = [sys.executable, "-m", "esptool", "--port", port, "flash_id"]
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=40,
                           encoding="utf-8", errors="replace")
        out = (p.stdout or "") + (p.stderr or "")
        return out, p.returncode
    except Exception as e:
        logger.debug(f"_flash_id_output({port}) ошибка: {e}")
        return "", -1


def _parse_flash_id(out):
    """Определяет модель/family/flash из вывода esptool `flash_id`.

    Returns:
        dict | None: {"model", "family", "flash_bytes", "flash_label"} или None.
    """
    model = None
    m = re.search(r"Chip is\s+([^\r\n]+)", out, re.IGNORECASE)
    if m:
        model = m.group(1).strip()
    else:
        m2 = re.search(r"Detecting chip type\.\.\.\s*([A-Za-z0-9\-]+)", out, re.IGNORECASE)
        if m2:
            model = m2.group(1).strip()

    if not model:
        return None

    family = family_of_model(model)
    if family is None:
        return None

    # Реальный объём флеш-памяти
    flash_label = None
    flash_bytes = None
    fm = re.search(r"Detected flash size:\s*([^\r\n]+)", out, re.IGNORECASE)
    if fm:
        flash_label = fm.group(1).strip()
        flash_bytes = flash_label_to_bytes(flash_label)

    return {"model": model, "family": family,
            "flash_bytes": flash_bytes, "flash_label": flash_label}


def detect_device(port):
    """Определяет модель ESP-чипа и реальный объём флеш-памяти через esptool.

    Устраняет flaky detection для ESP32‑S2 с нативным USB: в режиме приложения
    `esptool --port COM6 flash_id` сбрасывает чип (1200 baud) → порт переименовывается
    в COM7 (режим загрузки) → старый дескриптор становится недействительным, и esptool
    падает с OSError(22 'Invalid argument')/'A serial exception error occurred'.
    Поэтому после неудачи перечитываем COM‑порты и повторяем flash_id на порте того же
    USB‑устройства (COM6 → COM7), пока не получим ответ чипа.

    Returns:
        dict | None: {"port", "model", "family", "flash_bytes", "flash_label"}.
    """
    # Запоминаем «топологию» устройства ДО сброса: esptool flash_id на портe
    # приложения (COM6) сбрасывает ESP32‑S2 в режим загрузки — порт переименовывается
    # (COM6 -> COM7), и старый дескриптор становится недействительным.
    base = next((c for c in _comports() if c.device == port), None)

    # 1) прямой вызов на указанном порте
    out, _rc = _flash_id_output(port)
    info = _parse_flash_id(out)
    if info:
        info["port"] = port
        return info

    # 2) порт исчез / переименовался — ищем переименованный порт того же устройства
    if base is None:
        # вызывали с уже переименованного порта — берём любой доступный
        base = next(iter(_comports()), None)
    if base is None:
        return None

    last = port
    for _ in range(5):
        time.sleep(0.7)
        for comp in _comports():
            if not _same_usb_device(comp, base):
                continue
            p = comp.device
            if p == last:
                continue
            out, _rc = _flash_id_output(p)
            info = _parse_flash_id(out)
            if info:
                # Возвращаем ИСХОДНЫЙ порт приложения (COM6), а не загрузчика (COM7):
                # PlatformIO ожидает порт в режиме приложения и сам делает 1200bps‑сброс
                # для перехода в загрузчик. Отдавать порт загрузчика нельзя — тогда
                # PlatformIO сбросит чип ОБРАТНО в приложение (COM6) и не сможет
                # отследить переименование COM7→COM6 на Windows.
                info["port"] = port
                logger.debug(f"detect_device: чип найден, порт приложения {port} (загрузчик на {p})")
                return info
            last = p

    logger.debug(f"detect_device({port}): чип не опознан после переименования порта. "
                 f"Последний вывод:\\n{out[-1500:]}")
    return None


def detect_chip(port):
    """Определяет только модель ESP-чипа (без объёма флеш-памяти).

    Returns:
        dict | None: {"port", "model", "family"} или None.
    """
    info = detect_device(port)
    if not info:
        return None
    return {k: info[k] for k in ("port", "model", "family")}


def list_esp_ports():
    """Перебирает COM-порты и возвращает те, на которых есть ESP-чип.

    Returns:
        list[dict]: [{"port", "model", "family", "flash_bytes", "flash_label"}, ...]
    """
    found = []
    ports = _comports()
    if not ports:
        # Порт может быть в момент переименования (native USB ESP32: COM6 -> COM7).
        time.sleep(0.5)
        ports = _comports()
    for comp in ports:
        port = comp.device
        info = detect_device(port)
        if info:
            found.append(info)
    return found


def expected_flash_from_env(env):
    """Определяет ожидаемый физический объём флеш-памяти по имени env.

    Объём кодируется суффиксом вида '4mb'/'1mb'/'16mb' в имени env, например:
    esp8266_4mb -> 4 МБ, esp8285_1mb_ota -> 1 МБ.

    Returns:
        int | None: объём в байтах или None, если не найден.
    """
    m = re.search(r"(\d+)mb", (env or "").lower())
    if not m:
        return None
    return int(m.group(1)) * 1024 * 1024


def list_raw_ports():
    """Список ВСЕХ COM-портов без определения чипа (для ручного выбора).

    Returns:
        list[dict]: [{"port", "description"}, ...]
    """
    result = []
    ports = _comports()
    if not ports:
        # Порт может быть в момент переименования (native USB ESP32: COM6 -> COM7).
        time.sleep(0.5)
        ports = _comports()
    for comp in ports:
        result.append({"port": comp.device, "description": comp.description or ""})
    return result


def family_of_model(model):
    """Сопоставляет строку модели чипа (из esptool) с семейством платформы.

    ESP8266 и ESP8285 — РАЗНЫЕ чипы и разделяются строго:
      ESP8285 -> 'esp8285', ESP8266/ESP8266EX -> 'esp8266'.
    """
    m = (model or "").upper()
    if not m:
        return None
    if "ESP8285" in m:
        return "esp8285"
    if "ESP8266" in m:
        return "esp8266"
    if "ESP32" in m:
        if "S2" in m:
            return "esp32s2"
        if "S3" in m:
            return "esp32s3"
        if "C3" in m:
            return "esp32c3"
        return "esp32"
    return None


def family_of_env(env):
    """Определяет ожидаемое семейство платформы по имени env (platformio.ini).

    ESP8266 и ESP8285 — разные чипы, поэтому префиксы 'esp8266*' и 'esp8285*'
    сопоставляются с разными семействами.
    """
    e = (env or "").lower()
    if e.startswith("esp8285"):
        return "esp8285"
    if e.startswith("esp8266"):
        return "esp8266"
    if e.startswith("esp32"):
        if "s2" in e:
            return "esp32s2"
        if "s3" in e:
            return "esp32s3"
        if "c3" in e:
            return "esp32c3"
        return "esp32"
    return None
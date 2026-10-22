"""
platformio_tools — проверка наличия/установки PlatformIO для magicIoTm.

Сделан по образцу esptool_tools.py. Отличия от esptool:

  - PlatformIO на старте НЕ устанавливается автоматически: это тяжёлый пакет,
    поэтому отсутствие pio фиксируется, а установка предлагается пользователю
    модальным окном в UI (кнопка -> POST /api/tools/platformio/install);
  - pio может лежать в трёх местах: в PATH, в ~/.platformio/penv (официальный
    установщик) и в окружении самой панели (после автоустановки панелью);
    этот же порядок использует core.builder.get_platformio_path().

На старте приложения вызывается startup_check() (фоновый поток), который:
  1. определяет, найден ли исполняемый файл pio/platformio;
  2. получает версию (`pio --version`);
  3. запрашивает актуальную версию с PyPI;
  4. кэширует результат и пишет сводку в лог.

Состояние доступно через status() для API/UI. Все ошибки (нет сети, pio не
найден) не блокируют запуск сервера.
"""

import json
import logging
import os
import re
import shutil
import subprocess
import sys
import threading
import time
import urllib.request

logger = logging.getLogger(__name__)

# Официальный источник версии: PyPI JSON API (канал обновления pip-пакета)
PYPI_URL = "https://pypi.org/pypi/platformio/json"
# Минимальная версия для установки панелью
PIO_VERSION = "6.1"
CHECK_TIMEOUT = 5         # секунд на сетевой запрос
PIO_VERSION_TIMEOUT = 10  # секунд на `pio --version`


# ==================== Кэш состояния ====================

_status = {
    "available": False,
    "path": None,
    "installed_version": None,
    "latest_version": None,
    "needs_update": None,
    "source": PYPI_URL,
    "last_checked": None,
    "error": None,
}
_lock = threading.Lock()


# ==================== Поиск pio ====================

def _candidate_dirs():
    """Каталоги резервного поиска pio (после PATH).

    Returns:
        list[str]: venv панели (каталог python) и ~/.platformio/penv.
    """
    dirs = [os.path.dirname(sys.executable)]
    if os.name == "nt":
        dirs.append(os.path.join(os.environ.get("USERPROFILE", ""),
                                 ".platformio", "penv", "Scripts"))
    else:
        dirs.append(os.path.join(os.environ.get("HOME", ""),
                                 ".platformio", "penv", "bin"))
    return dirs


def get_path():
    """Ищет исполняемый файл pio/platformio: PATH -> venv панели -> penv.

    Returns:
        str | None: путь к исполняемому файлу или None, если не найден.
    """
    # 1. PATH (pio или platformio)
    for name in ("pio", "platformio"):
        found = shutil.which(name)
        if found:
            return found
    # 2. venv панели / ~/.platformio/penv
    exe_names = ("pio.exe", "platformio.exe") if os.name == "nt" else ("pio", "platformio")
    for d in _candidate_dirs():
        for name in exe_names:
            candidate = os.path.join(d, name)
            if os.path.isfile(candidate):
                return candidate
    return None


def get_installed():
    """Определяет, найден ли pio, и запрашивает его версию (`pio --version`).

    Returns:
        dict: {"available": bool, "path": str | None, "version": str | None}
    """
    path = get_path()
    if not path:
        return {"available": False, "path": None, "version": None}
    try:
        p = subprocess.run([path, "--version"], capture_output=True, text=True,
                           timeout=PIO_VERSION_TIMEOUT, encoding="utf-8",
                           errors="replace")
        out = (p.stdout or "") + (p.stderr or "")
        m = re.search(r"version\s+(\d[\w.+\-]*)", out)
        return {"available": True, "path": path, "version": m.group(1) if m else None}
    except Exception as e:  # FileNotFoundError и прочие ошибки запуска
        logger.debug(f"Не удалось получить версию pio ({path}): {e}")
        # Файл найден, но запустить не удалось — считаем pio «сломанным»
        return {"available": False, "path": path, "version": None}


# ==================== Актуальность версии ====================

def check_latest():
    """Запрашивает актуальную версию PlatformIO с PyPI.

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


# ==================== Фоновая проверка ====================

def _run_check():
    """Полный цикл проверки: найден ли pio и актуальность версии."""
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
            "path": installed["path"],
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
            "PlatformIO не найден. Установите: pip install platformio>=%s "
            "или предложите установку из панели (/api/tools/platformio/install)",
            PIO_VERSION,
        )
        return

    base = f"PlatformIO {s['installed_version'] or '?':<6} ({s['path']}) "
    if s["latest_version"] is None:
        logger.info(f"{base}| последнюю версию проверить не удалось (нет сети?)")
    elif s["needs_update"]:
        logger.info(f"{base}| доступна новая версия {s['latest_version']} (pip install -U platformio)")
    else:
        logger.info(f"{base}| актуальная версия")


def startup_check():
    """Запускает проверку PlatformIO в фоновом потоке (неблокирующе).

    В отличие от esptool, pio НЕ устанавливается автоматически: отсутствие
    фиксируется в кэше/логе, установку предлагает UI (модальное окно).
    """
    thread = threading.Thread(target=_run_check, daemon=True, name="platformio-check")
    thread.start()
    logger.info("Проверка PlatformIO запущена в фоне...")


def status():
    """Возвращает снимок состояния проверки PlatformIO для API/UI.

    Returns:
        dict: available, path, installed_version, latest_version,
              needs_update, source, last_checked, error
    """
    with _lock:
        return dict(_status)


# ==================== Установка ====================

def _pip(*args):
    """Запускает pip через sys.executable. Возвращает (rc, полный вывод)."""
    cmd = [sys.executable, "-m", "pip"] + list(args)
    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=600,
                           encoding="utf-8", errors="replace")
        return p.returncode, (p.stdout or "") + (p.stderr or "")
    except Exception as e:
        logger.error(f"Не удалось запустить pip {args}: {e}")
        return -1, str(e)


def install():
    """Устанавливает PlatformIO в окружение панели (sys.executable).

    Вызывается по действию пользователя (модальное окно в UI): pio ставится
    в тот же интерпретатор, где работает панель, поэтому дальше находится
    резервным поиском в каталоге sys.executable (см. get_path()).

    Returns:
        dict: результат status() после установки/проверки.
    """
    logger.info(f"Установка PlatformIO (pip install platformio>={PIO_VERSION})...")
    rc, out = _pip("install", f"platformio>={PIO_VERSION}")
    if rc != 0:
        logger.error("Установка PlatformIO не удалась: %s", out[-2000:])
    else:
        logger.info("PlatformIO установлен")
    _run_check()
    return status()


def ensure_installed():
    """Устанавливает PlatformIO, если он не найден; иначе просто возвращает статус.

    Returns:
        dict: результат status().
    """
    if get_installed()["available"]:
        return status()
    return install()


def ensure_updated():
    """Обновляет PlatformIO до актуальной версии (только по запросу пользователя).

    Returns:
        dict: {"success": bool, "error": str | None}
    """
    logger.info("Обновление PlatformIO по запросу пользователя...")
    rc, out = _pip("install", "-U", "platformio")
    if rc != 0:
        return {"success": False, "error": out[-2000:] or "неизвестная ошибка pip"}
    _run_check()
    return {"success": True}


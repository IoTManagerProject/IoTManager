"""
OTA (Over-The-Air firmware upload) logic for MagicIoTm.

Extracted from app.py to allow other modules to use OTA functions
without importing the Flask app.
"""

import json
import os
import logging

from utils.ota import start, is_running, event_stream, build_steps, busy_ip
from utils import ota as _ota_utils, ws_client
from core.devices import get_device_folder
from state.globals import current_project, current_config
from core.config import PROJECT_ROOT, ROOT_CONFIG_FILE, PLATFORMIO_INI_FILE, BASE_DIR, _project_dir

logger = logging.getLogger(__name__)


def resolve_ota_files(cfg):
    """Пути собранных .bin для OTA: firmware.bin и littlefs.bin (что есть).

    Приоритет — папка iotm/<env>/400/ конкретного проекта (файлы там стабильны
    после сборки); корневой .pio/build/<env> — запасной вариант.
    """
    env = cfg.get("env", "")
    cwd = cfg.get("cwd", "")
    build_dir = os.path.join(cwd, ".pio", "build", env)
    dist_dir = os.path.join(os.path.dirname(cfg.get("profile", "")), "iotm", env, "400")

    files = {}
    for name in ("firmware.bin", "littlefs.bin"):
        for c in (os.path.join(dist_dir, name), os.path.join(build_dir, name)):
            if os.path.isfile(c):
                files[name] = c
                break
        else:
            # альтернатива — spiffs (если littlefs нет)
            if name == "littlefs.bin":
                for s in (os.path.join(dist_dir, "spiffs.bin"),
                          os.path.join(build_dir, "spiffs.bin")):
                    if os.path.isfile(s):
                        files[name] = s
                        break
    return files


def profile_env(data):
    """Извлекает env платформы из profile.json устройства."""
    if not isinstance(data, dict):
        return None
    try:
        return data["projectProp"]["platformio"]["default_envs"]
    except Exception:
        return None


def device_platform(device_key):
    """Платформа (env) устройства по profile.json.

    1) читает локальный кэш <папка>/RAM/profile.json (если уже скачан);
    2) если нет — запрашивает профиль по WebSocket (/profile|) и кладёт в кэш.

    Возвращает строку env либо None (профиль недоступен).
    """
    entry = get_device_folder(device_key)
    if not entry:
        return None
    prof_path = os.path.join(entry.get("ram_dir", ""), "profile.json")
    data = None
    if os.path.isfile(prof_path):
        try:
            with open(prof_path, "r", encoding="utf-8") as f:
                data = json.load(f)
        except Exception:
            data = None
    if not data or not profile_env(data):
        ip = entry.get("ip")
        if ip:
            try:
                live = ws_client.fetch_profile(ip)
                if live and profile_env(live):
                    data = live
                    try:
                        os.makedirs(entry["ram_dir"], exist_ok=True)
                        with open(prof_path, "w", encoding="utf-8") as f:
                            json.dump(data, f, ensure_ascii=False, indent=2)
                    except Exception:
                        pass
            except Exception as e:
                logger.warning(f"Не удалось получить profile устройства {device_key}: {e}")
    return profile_env(data)


__all__ = [
    "start",
    "is_running",
    "event_stream",
    "build_steps",
    "busy_ip",
    "resolve_ota_files",
    "profile_env",
    "device_platform",
]

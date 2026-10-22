"""
OTA (Over-The-Air firmware upload) routes for MagicIoTm.

Extracted from app.py to allow modular routing via Flask Blueprint.
"""

import json
import os
import logging
import time
import socket
import socket as socket_mod

import state.globals as globals_
from flask import Blueprint, request, jsonify, Response

from core.ota import start as ota_start, is_running as ota_is_running, event_stream as ota_event_stream, build_steps, busy_ip, device_platform
from core.devices import _build_devices_payload, get_device_folder
from core.flasher import is_running as flash_is_running
from core.builder import resolve_build_config, is_running as build_is_running
from utils import ws_client

bp = Blueprint('ota', __name__)

logger = logging.getLogger(__name__)


def _local_ip_for_device(dev_ip):
    """IP компьютера, достижимый устройством (для локального OTA-сервера)."""
    import ipaddress
    try:
        net = ipaddress.ip_network(f"{dev_ip}/24", strict=False)
    except Exception:
        return _get_local_ip()
    for cand in _local_ips():
        try:
            if ipaddress.ip_address(cand) in net:
                return cand
        except Exception:
            continue
    return _get_local_ip()


def _local_ips():
    """Список полезных локальных IPv4 интерфейсов (для OTA-сервера)."""
    ips = []
    try:
        import psutil
        for _iface, addr_list in psutil.net_if_addrs().items():
            for a in addr_list:
                if a.family == socket_mod.AF_INET and _is_useful_interface(a.address):
                    ips.append(a.address)
    except Exception:
        pass
    if not ips:
        ip = _get_local_ip()
        if ip and not ip.startswith("127."):
            ips.append(ip)
    return ips


def _get_local_ip():
    """Локальный IPv4 ноутбука."""
    s = socket_mod.socket(socket_mod.AF_INET, socket_mod.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        return s.getsockname()[0]
    except Exception:
        return "127.0.0.1"
    finally:
        s.close()


def _is_useful_interface(ip):
    """Проверяет, что интерфейс может содержать IoT-устройства."""
    import ipaddress
    try:
        addr = ipaddress.ip_address(ip)
    except ValueError:
        return False
    if addr.is_loopback:
        return False
    if addr.is_link_local:
        return False
    return True


def _resolve_upload_config():
    """Формирует cfg для flash.start() по текущему проекту (или None)."""
    if not globals_.current_project:
        return None
    return resolve_build_config(globals_.current_project, globals_.current_config or {})


def _has_built_firmware(cfg):
    """Есть ли собранная прошивка для env проекта (firmware.bin)."""
    env = cfg.get("env", "")
    dist = os.path.join(os.path.dirname(cfg.get("profile", "")), "iotm", env, "400", "firmware.bin")
    build = os.path.join(cfg.get("cwd", ""), ".pio", "build", env, "firmware.bin")
    return any(os.path.isfile(c) for c in (dist, build))


@bp.route('/ota/candidates', methods=['GET'])
def api_ota_candidates():
    """Список онлайн-устройств с платформой и признаком совместимости с проектом."""
    cfg = _resolve_upload_config()
    if cfg is None:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400
    env = cfg.get("env", "")
    files = ota_start.__globals__.get('resolve_ota_files')
    from core.ota import resolve_ota_files
    files = resolve_ota_files(cfg)
    if not _has_built_firmware(cfg):
        return jsonify({"success": False,
                        "error": "Прошивка не собрана. Сначала выполните сборку (??)."}), 400

    payload = _build_devices_payload().get("devices", [])
    candidates = []
    for d in payload:
        if not d.get("online"):
            continue
        dev_env = device_platform(d["key"])
        compatible = bool(dev_env) and dev_env == env
        unknown = not dev_env
        candidates.append({
            "key": d["key"],
            "ip": d.get("ip", ""),
            "name": d.get("name") or d["key"],
            "platform": dev_env,
            "compatible": compatible,
            "unknown": unknown,
        })
    candidates.sort(key=lambda x: (not x["compatible"], x["name"].lower()))
    return jsonify({
        "success": True,
        "env": env,
        "files": files,
        "candidates": candidates,
    })


@bp.route('/ota/start', methods=['POST'])
def api_ota_start():
    """Запуск OTA в фоне."""
    data = request.json or {}
    device_key = (data.get('device_key') or '').strip()
    mode = data.get('mode', 'firmware')
    fs_method = data.get('fs_method', 'flash')

    if mode not in ("fs", "firmware", "full"):
        return jsonify({"success": False, "error": f"Неизвестный режим OTA: {mode}"}), 400
    if fs_method not in ("flash", "copy"):
        return jsonify({"success": False, "error": f"Неизвестный способ записи FS: {fs_method}"}), 400
    if not device_key:
        return jsonify({"success": False, "error": "Не указано устройство"}), 400
    if build_is_running():
        return jsonify({"success": False, "error": "Сборка выполняется — дождитесь её завершения"}), 409
    if flash_is_running():
        return jsonify({"success": False, "error": "Идёт прошивка по USB — дождитесь её завершения"}), 409
    if ota_is_running():
        return jsonify({"success": False, "error": "OTA уже выполняется"}), 409

    entry = get_device_folder(device_key)
    if not entry:
        return jsonify({"success": False, "error": "Устройство не найдено"}), 404
    ip = entry.get("ip")
    if not ip:
        return jsonify({"success": False, "error": "У устройства не указан IP"}), 400

    cfg = _resolve_upload_config()
    if cfg is None:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400
    from core.ota import resolve_ota_files
    files = resolve_ota_files(cfg)

    steps = build_steps(mode, fs_method, cfg.get("env", ""), cfg.get("ini", ""))
    # неподдерживаемая комбинация (например, LibreTiny + fs_method=flash):
    # не запускаем воркер, отвечаем сразу — шаг 'error' содержит текст отказа
    err_steps = [st for st in steps if st.get("kind") == "error"]
    if err_steps:
        return jsonify({"success": False, "error": err_steps[0]["label"]}), 400
    missing = []
    for st in steps:
        if st["kind"] == "pull":
            for f in st["files"]:
                if f not in files:
                    missing.append(f)
    if missing:
        return jsonify({"success": False,
                        "error": "Не собраны файлы для выбранного режима: " + ", ".join(missing) +
                                 ". Выполните сборку (??)."}), 400
    if any(st["kind"] == "copy" for st in steps):
        data_dir = cfg.get("data_dir", "")
        if not data_dir or not os.path.isdir(data_dir):
            return jsonify({"success": False,
                            "error": f"Не найден каталог данных data_svelte: {data_dir}"}), 400

    dev_env = device_platform(device_key)
    if dev_env and dev_env != cfg.get("env", ""):
        return jsonify({"success": False,
                        "error": ("Платформа устройства ({}) не совпадает с платформой проекта ({}). "
                                  "OTA отменена.").format(dev_env, cfg.get("env", ""))}), 409

    ota_cfg = {
        "mode": mode,
        "fs_method": fs_method,
        "ip": ip,
        "env": cfg.get("env", ""),
        "ini": cfg.get("ini", ""),
        "project_label": cfg.get("project_label", ""),
        "files": files,
        "data_dir": cfg.get("data_dir", ""),
        "pc_ip": _local_ip_for_device(ip),
        "timeout": 180,
    }
    if not ota_start(ota_cfg):
        return jsonify({"success": False,
                        "error": "Не удалось запустить OTA (вероятно, уже выполняется)"}), 409
    logger.info(f"OTA запущена: env={cfg.get('env')}, mode={mode}, fs_method={fs_method}, ip={ip}")
    return jsonify({"success": True})


@bp.route('/ota/stream')
def api_ota_stream():
    """SSE-поток событий OTA (лог, шаги, прогресс, финал)."""
    def gen():
        for chunk in ota_event_stream():
            yield chunk
    return Response(gen(), mimetype="text/event-stream",
                    headers={"Cache-Control": "no-cache", "X-Accel-Buffering": "no"})

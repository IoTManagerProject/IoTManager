import ipaddress
import json
import os
import time
import threading
import logging

import state.globals as globals_
from flask import Blueprint, request, jsonify, Response

from core.config import get_platformio_platforms, is_compatible
from core.devices import (
    get_device_folder,
    _host_pingable,
    _build_devices_payload,
    _devices_signature,
    _scan_snapshot,
    _walk_tree,
    _safe_path,
    _rmtree_with_retry,
    _rmtree_background,
    add_device_by_ip,
    get_network_interfaces,
    scan_network_worker,
    _fetch_progress,
    ws_client,
)
from core import wifi as wifi_core
from utils import projects

# Import locks and state from state.globals
_device_folders_lock = globals_._device_folders_lock
_device_folders = globals_._device_folders
_devices_lock = globals_._devices_lock
_devices = globals_._devices
_device_states_lock = globals_._device_states_lock
_device_states = globals_._device_states
_scan_lock = globals_._scan_lock
_scan_state = globals_._scan_state

logger = logging.getLogger(__name__)

devices_bp = Blueprint("devices", __name__)


# ==================== Device List Routes ====================


@devices_bp.route("/devices", methods=["GET"])
def api_devices():
    """List of discovered devices with online/offline status."""
    return jsonify(_build_devices_payload())


@devices_bp.route("/devices/list-all", methods=["GET"])
def api_devices_list_all():
    """Returns list of ALL device folders with RAM/FS file presence."""
    from core.devices import DEVICE_DIR_ROOT, _load_folder_meta
    devices = []
    if not os.path.isdir(DEVICE_DIR_ROOT):
        return jsonify({"success": True, "devices": []})
    for folder_name in sorted(os.listdir(DEVICE_DIR_ROOT)):
        folder = os.path.join(DEVICE_DIR_ROOT, folder_name)
        if not os.path.isdir(folder):
            continue
        meta = _load_folder_meta(folder) or {}
        devices.append({
            "key": folder_name,
            "name": meta.get("name") or folder_name,
            "ip": meta.get("ip") or "",
            # settings.json — для копирования настроек
            "ram_settings": os.path.isfile(os.path.join(folder, "RAM", "settings.json")),
            "fs_settings": os.path.isfile(os.path.join(folder, "FS", "settings.json")),
            # profile.json (RAM) / flashProfile.json (FS) — для копирования модулей
            "ram_profile": os.path.isfile(os.path.join(folder, "RAM", "profile.json")),
            "fs_profile": os.path.isfile(os.path.join(folder, "FS", "flashProfile.json")),
        })
    return jsonify({"success": True, "devices": devices})


@devices_bp.route("/devices/stream")
def api_devices_stream():
    """SSE stream of device list changes (online/offline, new/updated)."""
    def gen():
        last_sig = None
        while True:
            sig = _devices_signature()
            if sig != last_sig:
                last_sig = sig
                yield f"data: {json.dumps(_build_devices_payload(), ensure_ascii=False)}\n\n"
            time.sleep(1)

    return Response(gen(), mimetype="text/event-stream", headers={
        "Cache-Control": "no-cache",
        "X-Accel-Buffering": "no",
    })


# ==================== WiFi-антенна: AP-сети модулей (iotm*) ====================


@devices_bp.route("/devices/wifi", methods=["GET"])
def api_wifi_state():
    """Снимок WiFi-сканера: AP-сети iotm*, счётчик новых, статус подключения."""
    return jsonify(wifi_core.get_state())


@devices_bp.route("/devices/wifi/stream")
def api_wifi_stream():
    """SSE-поток состояния WiFi-сканера (кадры — только при изменениях)."""
    def gen():
        last_sig = None
        while True:
            sig = wifi_core.signature()
            if sig != last_sig:
                last_sig = sig
                yield f"data: {json.dumps(wifi_core.get_state(), ensure_ascii=False)}\n\n"
            time.sleep(2)

    return Response(gen(), mimetype="text/event-stream", headers={
        "Cache-Control": "no-cache",
        "X-Accel-Buffering": "no",
    })


@devices_bp.route("/devices/wifi/scan", methods=["POST"])
def api_wifi_scan():
    """Скан эфира по требованию, не дожидаясь очередного цикла потока.

    force=True — активный поиск WlanScan, а не чтение кэша драйвера.
    """
    try:
        wifi_core.scan_once(force=True)
    except Exception as e:
        logger.error(f"WiFi scan error: {e}", exc_info=True)
        return jsonify({"success": False, "error": str(e)}), 500
    return jsonify({"success": True, **wifi_core.get_state()})


@devices_bp.route("/devices/wifi/seen", methods=["POST"])
def api_wifi_seen():
    """Список AP-сетей показан пользователю — гасит счётчик «новых» (кнопку «!»)."""
    body = request.get_json(silent=True) or {}
    ssids = body.get("ssids")
    try:
        wifi_core.mark_seen(ssids if isinstance(ssids, list) and ssids else None)
    except Exception as e:
        logger.error(f"WiFi seen error: {e}", exc_info=True)
        return jsonify({"success": False, "error": str(e)}), 500
    return jsonify({"success": True, **wifi_core.get_state()})


@devices_bp.route("/devices/wifi/connect", methods=["POST"])
def api_wifi_connect():
    """Подключение панели к AP-сети модуля + поиск модуля (подсеть/ping/devlist).

    Тело: {ssid, password?}. Выполняется фоном; ход — в /devices/wifi (connect.stage).
    """
    body = request.get_json(silent=True) or {}
    ssid = str(body.get("ssid") or "").strip()
    password = str(body.get("password") or "")
    if not ssid:
        return jsonify({"success": False, "error": "Не указано имя сети (ssid)"}), 400
    if not wifi_core.start_connect(ssid, password):
        return jsonify({"success": False, "error": "Подключение уже выполняется",
                        "connect": wifi_core.connect_state()}), 409
    return jsonify({"success": True, "ssid": ssid,
                    "connect": wifi_core.connect_state()})


@devices_bp.route("/devices/wifi/return", methods=["POST"])
def api_wifi_return():
    """Возврат панели в «домашнюю» сеть (сохранённый профиль) после AP модуля.

    Домашней считается последняя обычная сеть панели (не AP с префиксом iotm):
    она запоминается при каждом WiFi-скане и перед подключением к AP.
    """
    home = wifi_core.home_network()
    if not home["ssid"]:
        return jsonify({"success": False,
                        "error": "Домашняя сеть неизвестна: панель ещё не работала "
                                 "в обычной WiFi-сети"}), 400
    if not wifi_core.start_return():
        return jsonify({"success": False, "error": "Смена WiFi-сети уже выполняется",
                        "connect": wifi_core.connect_state()}), 409
    return jsonify({"success": True, "ssid": home["ssid"],
                    "connect": wifi_core.connect_state()})


@devices_bp.route("/devices/wifi/passwords", methods=["GET"])
def api_wifi_passwords():
    """Пароли AP из проектов (myProfile.json) и устройств (settings.json) —
    список для подстановки в модалке ввода пароля (кнопка 📁)."""
    try:
        return jsonify(wifi_core.stored_passwords())
    except Exception as e:
        logger.error(f"WiFi passwords error: {e}", exc_info=True)
        return jsonify({"success": False, "error": str(e)}), 500


# ==================== Device Management Routes ====================


@devices_bp.route("/device/<device_key>", methods=["DELETE"])
def api_device_delete(device_key):
    """Delete device: folder on disk and all records about it."""
    try:
        with _device_folders_lock:
            entry = _device_folders.get(device_key)
        if not entry:
            return jsonify({"success": False, "error": "Device not found"}), 404
        folder = entry["folder"]
        ip = entry.get("ip")
        with _device_folders_lock:
            _device_folders.pop(device_key, None)
        if ip:
            with _devices_lock:
                _devices.pop(ip, None)
            with _device_states_lock:
                _device_states.pop(device_key, None)
        if _rmtree_with_retry(folder):
            logger.info(f"Device deleted: {device_key} ({folder})")
            return jsonify({"success": True})
        logger.warning(f"Folder {folder} is busy; deletion deferred to background")
        threading.Thread(target=_rmtree_background, args=(folder,), daemon=True,
                         name="device-folder-cleanup").start()
        return jsonify({"success": True, "warning": "Folder is busy, will be deleted later"})
    except Exception as e:
        logger.error(f"Delete device error: {e}", exc_info=True)
        return jsonify({"success": False, "error": str(e)}), 500


@devices_bp.route("/device/<device_key>/copy-to-project", methods=["POST"])
def api_device_copy_to_project(device_key):
    """Копирование устройства в проект: существующий или новый.

    Правила те же, что у кнопок «⤵ Из устройства» в панели проекта:
    - settings.json (RAM/FS) -> iotmSettings, все поля кроме id/ip/root;
    - profile.json (RAM) / flashProfile.json (FS) -> платформа из default_envs
      (несовместимые модули отключаются) и активность модулей по path;
    - проект назначения при отсутствии создаётся из корневого шаблона.
    """
    data = request.json or {}
    dst_cat = (data.get("dst_cat") or "").strip()
    dst_name = (data.get("dst_name") or "").strip()
    section = (data.get("section") or "ram").lower()
    for value, label in ((dst_cat, "категория"), (dst_name, "имя проекта")):
        if not value or value in (".", "..") or "/" in value or "\\" in value or ".." in value:
            return jsonify({"success": False, "error": f"Некорректное имя: {label}"}), 400
    if section not in ("ram", "fs"):
        return jsonify({"success": False, "error": "Unknown section"}), 400
    if not os.path.isdir(os.path.join(projects.PROJECTS_DIR, dst_cat)):
        return jsonify({"success": False, "error": "Категория назначения не найдена"}), 400

    settings = _device_section_json(device_key, section, "settings.json")
    profile = _device_section_json(device_key, section, "profile.json" if section == "ram" else "flashProfile.json")
    # _device_section_json возвращает (Response, status) при ошибке чтения — трактуем как отсутствие файла
    settings = {} if isinstance(settings, tuple) else (settings or {})
    profile = {} if isinstance(profile, tuple) else (profile or {})
    if not settings and not profile:
        return jsonify({"success": False, "error": f"Нет settings.json и profile.json в разделе {section.upper()}"}), 404

    default_envs = (profile.get("projectProp", {}).get("platformio", {}).get("default_envs")
                    or profile.get("default_envs"))
    if default_envs and default_envs not in get_platformio_platforms():
        return jsonify({"success": False, "error": f"Платформа не найдена: {default_envs}"}), 400

    dst_path = os.path.join(projects.PROJECTS_DIR, dst_cat, dst_name)
    created = False
    if os.path.exists(dst_path):
        if projects.is_platformio(dst_name):
            return jsonify({"success": False, "error": "Проект PlatformIO нельзя изменить"}), 400
        cfg = projects.load_project_config(dst_cat, dst_name)
        if cfg is None:
            return jsonify({"success": False, "error": "myProfile.json проекта не найден"}), 400
    else:
        ok, msg = projects.create_project(dst_cat, dst_name, "")
        if not ok:
            return jsonify({"success": False, "error": msg}), 400
        cfg = projects.load_project_config(dst_cat, dst_name)
        created = True

    # 1) Значения: все поля настроек кроме служебных (id, ip, root)
    copied_settings = 0
    if settings:
        target_settings = cfg.setdefault("iotmSettings", {})
        for key, val in settings.items():
            if key not in ("id", "ip", "root"):
                target_settings[key] = val
                copied_settings += 1

    # 2) Платформа из default_envs: несовместимые модули отключаются
    if default_envs:
        for mods in cfg.get("modules", {}).values():
            if not isinstance(mods, list):
                continue
            for m in mods:
                mod_name = m.get("path", "").split("/")[-1]
                libs = globals_.modinfo_cache.get(mod_name, {}).get("usedLibs", {})
                if m.get("active") and not is_compatible(default_envs, libs):
                    m["active"] = False

    # 3) Активность модулей по path из профиля устройства (у устройства активные win)
    copied_modules = 0
    modules_src = profile.get("modules", {})
    if isinstance(modules_src, dict):
        for sec, arr in modules_src.items():
            target_mods = cfg.get("modules", {}).get(sec)
            if not isinstance(target_mods, list) or not isinstance(arr, list):
                continue
            src_map = {m.get("path"): bool(m.get("active")) for m in arr if isinstance(m, dict)}
            for m in target_mods:
                if m.get("path") in src_map:
                    m["active"] = src_map[m.get("path")]
                    copied_modules += 1

    if default_envs:
        cfg.setdefault("projectProp", {}).setdefault("platformio", {})["default_envs"] = default_envs

    projects.save_project_config(dst_cat, dst_name, cfg)

    # Если целевой проект открыт в панели — обновляем её состояние
    with globals_._lock:
        cur = globals_.current_project
        if cur and cur.get("category") == dst_cat and cur.get("name") == dst_name:
            globals_.current_config = cfg
            if default_envs:
                globals_.current_platform = default_envs

    logger.info(f"Устройство скопировано в проект: {device_key} ({section.upper()}) -> "
                f"{dst_cat}/{dst_name}{' (проект создан)' if created else ''}")
    return jsonify({"success": True, "created": created, "category": dst_cat, "name": dst_name,
                    "settings": copied_settings, "modules": copied_modules, "platform": default_envs})


@devices_bp.route("/device/<device_key>/info", methods=["GET"])
def api_device_info(device_key):
    """Device info and path to its folder on disk."""
    entry = get_device_folder(device_key)
    if not entry:
        return jsonify({"success": False, "error": "Device folder not yet created"}), 404
    return jsonify({"success": True, **entry})


@devices_bp.route("/devices/interfaces", methods=["GET"])
def api_devices_interfaces():
    """Returns list of network interfaces for selection."""
    interfaces = get_network_interfaces()
    return jsonify({"success": True, "interfaces": interfaces})


# ==================== Network Scan Routes ====================


@devices_bp.route("/devices/scan", methods=["POST"])
def api_devices_scan():
    """Start background network scan."""
    data = request.json or {}
    subnet_label = data.get("subnet", "")
    with _scan_lock:
        if _scan_state["running"]:
            if globals_._scan_thread and globals_._scan_thread.is_alive():
                return jsonify({"success": True, "already": True})
            logger.warning("Reset stuck scan state (thread dead)")
            _scan_state["running"] = False
        _scan_state.update({
            "total": 0, "done": 0, "subnet": "",
            "alive": [], "found": [], "failed": [], "error": None,
        })
        _scan_state["running"] = True
    globals_._scan_thread = threading.Thread(
        target=scan_network_worker, daemon=True,
        args=(subnet_label if subnet_label else None,),
        name="scan-network")
    globals_._scan_thread.start()
    return jsonify({"success": True})


@devices_bp.route("/devices/scan/stream")
def api_devices_scan_stream():
    """SSE stream of network scan progress."""
    def gen():
        try:
            while True:
                st = _scan_snapshot()
                yield f"data: {json.dumps(st, ensure_ascii=False)}\n\n"
                if not st["running"]:
                    break
                time.sleep(0.15)
        except GeneratorExit:
            pass
    return Response(gen(), mimetype="text/event-stream", headers={
        "Cache-Control": "no-cache",
        "X-Accel-Buffering": "no",
    })


@devices_bp.route("/devices/add", methods=["POST"])
def api_devices_add():
    """Add device by IP."""
    data = request.json or {}
    return jsonify(add_device_by_ip(str(data.get("ip", ""))))


# ==================== Fetch Routes ====================


@devices_bp.route("/device/<device_key>/fetch/<section>", methods=["POST"])
def api_device_fetch(device_key, section):
    """Starts background download of RAM or FS section from device."""
    from core.devices import ws_client
    sec = section.lower()
    if sec not in ("ram", "fs"):
        return jsonify({"success": False, "error": "Unknown section"}), 400
    entry = get_device_folder(device_key)
    if not entry:
        return jsonify({"success": False, "error": "Device folder not yet created"}), 404
    dev_ip = entry.get("ip")
    if not dev_ip:
        return jsonify({"success": False, "error": "Device IP not specified"}), 400

    state = {"stage": "running", "done": 0, "total": 0,
             "name": "Connecting to device...", "files": [], "error": None,
             "ip": dev_ip}
    _fetch_progress[(device_key, sec)] = state

    def progress(fname, done, total):
        state["done"] = done
        state["total"] = total
        state["name"] = fname

    def worker():
        try:
            if sec == "ram":
                saved = ws_client.fetch_ram(dev_ip, entry["ram_dir"], progress=progress)
            else:
                saved = ws_client.fetch_fs(dev_ip, entry["fs_dir"], progress=progress)
            state["files"] = saved
            state["stage"] = "done"
        except Exception as e:
            logger.error(f"fetch {sec} {dev_ip}: {e}")
            state["stage"] = "error"
            state["error"] = str(e)

    threading.Thread(target=worker, daemon=True).start()
    return jsonify({"success": True})


@devices_bp.route("/device/<device_key>/fetch/<section>/status", methods=["GET"])
def api_device_fetch_status(device_key, section):
    """Returns current progress of background section download."""
    key = section.lower()
    state = _fetch_progress.get((device_key, key))
    if not state:
        return jsonify({"success": False, "error": "Download not started"}), 404
    if state["stage"] == "error":
        return jsonify({"success": False, "error": state.get("error") or "Download error"})
    return jsonify({
        "success": True,
        "stage": state["stage"],
        "done": state.get("done", 0),
        "total": state.get("total", 0),
        "name": state.get("name", ""),
        "files": state.get("files", []),
    })
# ==================== File Tree and File Routes ====================


@devices_bp.route("/device/<device_key>/fetch/<section>/file", methods=["POST"])
def api_device_fetch_one_file(device_key, section):
    """Downloads a single file (RAM or FS) from device into its folder.

    Body: {path: <relative path>}. For RAM the file must be one of the
    ws_client RAM files (config.json, items.json, ...); for FS the path is
    relative to the FS root (e.g. "index.html", "js/app.js").
    """
    sec = section.lower()
    if sec not in ("ram", "fs"):
        return jsonify({"success": False, "error": "Unknown section"}), 400
    entry = get_device_folder(device_key)
    if not entry:
        return jsonify({"success": False, "error": "Device folder not yet created"}), 404
    dev_ip = entry.get("ip")
    if not dev_ip:
        return jsonify({"success": False, "error": "Device IP not specified"}), 400
    data = request.json or {}
    rel = str(data.get("path", "")).strip()
    if not rel:
        return jsonify({"success": False, "error": "Path not specified"}), 400
    out_dir = entry[f"{sec}_dir"]
    try:
        if sec == "ram":
            # В RAM файлы плоские — используем только имя файла
            name = os.path.basename(rel)
            ws_client.fetch_ram_file(dev_ip, name, out_dir)
            return jsonify({"success": True, "path": name})
        # FS: путь проверяем на выход за пределы каталога устройства
        if not _safe_path(entry["fs_dir"], rel):
            return jsonify({"success": False, "error": "Invalid path"}), 400
        saved = ws_client.fetch_fs_file(dev_ip, rel, entry["fs_dir"])
        return jsonify({"success": True, "path": saved})
    except Exception as e:  # noqa: BLE001 — сеть/устройство, отдаём текст ошибки
        logger.error(f"fetch one {sec} {dev_ip}: {e}")
        return jsonify({"success": False, "error": str(e)}), 502





# ==================== File Tree and File Routes ====================


@devices_bp.route("/device/<device_key>/tree/<section>", methods=["GET"])
def api_device_tree(device_key, section):
    """File tree of RAM or FS section (local device folder)."""
    entry = get_device_folder(device_key)
    if not entry:
        return jsonify({"success": False, "error": "Device folder not yet created"}), 404
    key = section.lower()
    if key not in ("ram", "fs"):
        return jsonify({"success": False, "error": "Unknown section"}), 400
    root = entry[f"{key}_dir"]
    dirs, files = _walk_tree(root)
    return jsonify({"success": True, "root": root, "dirs": dirs, "files": files})


@devices_bp.route("/device/<device_key>/file/<section>", methods=["GET"])
def api_device_read_file(device_key, section):
    """File content in RAM or FS section."""
    entry = get_device_folder(device_key)
    if not entry:
        return jsonify({"success": False, "error": "Device folder not yet created"}), 404
    key = section.lower()
    if key not in ("ram", "fs"):
        return jsonify({"success": False, "error": "Unknown section"}), 400
    root = entry[f"{key}_dir"]
    rel = request.args.get("path", "")
    abs_path = _safe_path(root, rel)
    if not abs_path or not os.path.isfile(abs_path):
        return jsonify({"success": False, "error": "File not found"}), 404
    # Только метаданные (имя/размер) — редактор запрашивает их для бинарных файлов
    if request.args.get("meta") == "1":
        return jsonify({
            "success": True,
            "path": rel,
            "name": os.path.basename(rel),
            "size": os.path.getsize(abs_path),
        })
    try:
        with open(abs_path, "r", encoding="utf-8", errors="replace") as f:
            content = f.read()
    except Exception as e:
        return jsonify({"success": False, "error": str(e)}), 500
    return jsonify({
        "success": True,
        "path": rel,
        "name": os.path.basename(rel),
        "size": os.path.getsize(abs_path),
        "content": content,
    })


@devices_bp.route("/device/<device_key>/file/<section>", methods=["POST"])
def api_device_save_file(device_key, section):
    """Saves modified file locally to device folder."""
    entry = get_device_folder(device_key)
    if not entry:
        return jsonify({"success": False, "error": "Device folder not yet created"}), 404
    key = section.lower()
    if key not in ("ram", "fs"):
        return jsonify({"success": False, "error": "Unknown section"}), 400
    root = entry[f"{key}_dir"]
    data = request.json or {}
    rel = data.get("path", "")
    content = data.get("content", "")
    abs_path = _safe_path(root, rel)
    if not abs_path:
        return jsonify({"success": False, "error": "Invalid path"}), 400
    # Бинарные файлы (*.gz, favicon.ico) через редактор не записываем
    lower = str(rel).lower()
    if lower.endswith('.gz') or os.path.basename(lower) == 'favicon.ico':
        return jsonify({"success": False, "error": "Бинарный файл недоступен в редакторе"}), 400
    try:
        os.makedirs(os.path.dirname(abs_path), exist_ok=True)
        with open(abs_path, "w", encoding="utf-8") as f:
            f.write(content)
    except Exception as e:
        return jsonify({"success": False, "error": str(e)}), 500
    return jsonify({"success": True, "path": rel})


@devices_bp.route("/device/<device_key>/write/ram", methods=["POST"])
def api_device_write_ram(device_key):
    """Writes a file back to the device RAM via reverse WebSocket command.

    Body: {path: <relative path>, content: <text>}
    Only files supported by the firmware (ws_client.WRITE_HEADERS) can be written.
    On success the local RAM copy is updated as well.
    """
    entry = get_device_folder(device_key)
    if not entry:
        return jsonify({"success": False, "error": "Device folder not yet created"}), 404
    dev_ip = entry.get("ip")
    if not dev_ip:
        return jsonify({"success": False, "error": "Device IP not specified"}), 400

    data = request.json or {}
    rel = data.get("path", "")
    content = data.get("content")
    if content is None:
        content = ""

    ram_dir = entry.get("ram_dir")
    abs_path = _safe_path(ram_dir, rel)
    if not abs_path:
        return jsonify({"success": False, "error": "Invalid path"}), 400

    filename = os.path.basename(abs_path)
    if filename not in ws_client.WRITE_HEADERS:
        return jsonify({
            "success": False,
            "error": (f"Файл '{filename}' не поддерживается прошивкой для записи обратно "
                      f"на устройство. Доступны: {', '.join(sorted(ws_client.WRITE_HEADERS))}.")
        }), 400

    try:
        ws_client.write_file(dev_ip, filename, content)
    except Exception as e:
        logger.error(f"Write to device {dev_ip} ({filename}): {e}", exc_info=True)
        return jsonify({"success": False, "error": str(e)}), 500

    # Обновляем локальную копию, чтобы не было рассинхрона с устройством
    try:
        os.makedirs(os.path.dirname(abs_path), exist_ok=True)
        with open(abs_path, "w", encoding="utf-8") as f:
            f.write(content)
    except Exception as e:
        logger.warning(f"Could not update local copy {abs_path}: {e}")

    return jsonify({"success": True, "path": rel})


@devices_bp.route("/device/<device_key>/ping", methods=["POST"])
def api_device_ping(device_key):
    """Ручной пинг устройства перед получением файлов.

    Для устройств со статусом «не в сети»: факт доступности по ICMP,
    FSM-статус не меняется. Устройство без папки (orphan) ищем по IP
    из ключа — фронтенд передаёт activeDevice.key || activeDevice.ip.
    """
    entry = get_device_folder(device_key)
    ip = entry.get("ip") if entry else ""
    if not ip:
        try:
            ipaddress.ip_address(device_key)
            ip = device_key
        except ValueError:
            return jsonify({"success": False, "error": "Device not found"}), 404
    online = _host_pingable(ip)
    logger.info(f"Ping {ip} ({device_key}): {'ok' if online else 'no reply'}")
    return jsonify({"success": True, "online": bool(online)})


@devices_bp.route("/device/<device_key>/reboot", methods=["POST"])
def api_device_reboot(device_key):
    """Sends reboot command to the device via WebSocket (/reboot|)."""
    entry = get_device_folder(device_key)
    if not entry:
        return jsonify({"success": False, "error": "Device folder not yet created"}), 404
    dev_ip = entry.get("ip")
    if not dev_ip:
        return jsonify({"success": False, "error": "Device IP not specified"}), 400
    try:
        ok = ws_client.reboot(dev_ip)
    except Exception as e:
        logger.error(f"Reboot {dev_ip}: {e}", exc_info=True)
        return jsonify({"success": False, "error": str(e)}), 500
    if not ok:
        return jsonify({"success": False, "error": f"Не удалось отправить команду перезагрузки на {dev_ip}"}), 502
    logger.info(f"Reboot command sent to {dev_ip} ({device_key})")
    return jsonify({"success": True})


# ==================== Device Settings Routes ====================


def _device_section_json(device_key, section, filename):
    """Reads a JSON file from the RAM/FS folder of the device."""
    sec = (section or "").lower()
    if sec not in ("ram", "fs"):
        return jsonify({"success": False, "error": "Unknown section"}), 400
    entry = get_device_folder(device_key)
    if entry:
        base_dir = entry[f"{sec}_dir"]
    else:
        # Fallback: folder on disk (device not yet registered in memory)
        from core.devices import DEVICE_DIR_ROOT
        base_dir = os.path.join(DEVICE_DIR_ROOT, device_key, sec.upper())
    path = os.path.join(base_dir, filename)
    if not os.path.isfile(path):
        return jsonify({"success": False, "error": f"{filename} not found in {sec.upper()}"}), 404
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            return json.load(f)
    except Exception as e:
        logger.error(f"Read {path}: {e}", exc_info=True)
        return jsonify({"success": False, "error": str(e)}), 500


@devices_bp.route("/device/settings", methods=["GET"])
def api_device_settings():
    """Returns settings.json from the device RAM/FS folder."""
    key = request.args.get("key", "")
    section = request.args.get("section", "ram")
    data = _device_section_json(key, section, "settings.json")
    # _device_section_json returns (Response, status) on error
    if isinstance(data, tuple):
        return data
    return jsonify({"success": True, "settings": data})


@devices_bp.route("/device/profile", methods=["GET"])
def api_device_profile():
    """Returns modules and default_envs from profile.json (RAM) or flashProfile.json (FS)."""
    key = request.args.get("key", "")
    section = request.args.get("section", "ram")
    data = _device_section_json(key, section, "profile.json" if section == "ram" else "flashProfile.json")
    if isinstance(data, tuple):
        return data
    # default_envs: новый формат — projectProp.platformio.default_envs,
    # запасной — ключ default_envs верхнего уровня
    default_envs = (data.get("projectProp", {}).get("platformio", {}).get("default_envs")
                    or data.get("default_envs"))
    return jsonify({"success": True, "modules": data.get("modules", {}),
                    "default_envs": default_envs})

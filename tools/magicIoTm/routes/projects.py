"""
Проекты — маршруты Flask для управления проектами.
"""

import json
import os
import logging
import shutil

import state.globals as globals_
from flask import Blueprint, request, jsonify
from utils import projects, build, measure_run
from core.config import PROJECT_ROOT, ROOT_CONFIG_FILE, PLATFORMIO_INI_FILE, _project_dir, _find_data_svelte
from core.devices import _safe_path, _walk_tree

logger = logging.getLogger(__name__)

bp = Blueprint('projects', __name__)


# ==================== Вспомогательные функции ====================

def _dir_size(path):
    """Суммарный размер файлов в каталоге (байты)."""
    if not path or not os.path.isdir(path):
        return 0
    total = 0
    for root, _dirs, files in os.walk(path):
        for f in files:
            try:
                total += os.path.getsize(os.path.join(root, f))
            except OSError:
                pass
    return total


# ==================== Маршруты: проекты ====================

@bp.route('/projects', methods=['GET'])
def api_list_projects():
    return jsonify({
        "success": True,
        "tree": projects.list_projects(),
        "about": projects.get_all_abouts(),
    })


@bp.route('/projects/data-svelte', methods=['GET'])
def api_projects_with_data_svelte():
    """Проекты, у которых есть папка data_svelte (кроме PlatformIO).

    Используется для привязки прошивки к менеджеру устройства и копирования файлов.
    """
    result = []
    tree = projects.list_projects()
    for cat, projs in tree.items():
        for name in projs:
            if projects.is_platformio(name):
                continue
            dsv = _find_data_svelte(os.path.join(projects.PROJECTS_DIR, cat, name))
            if dsv:
                result.append({"category": cat, "name": name, "path": dsv})
    result.sort(key=lambda p: (p["category"], p["name"]))
    return jsonify({"success": True, "projects": result})


@bp.route('/projects/category', methods=['POST'])
def api_create_category():
    name = request.json.get('name', '').strip()
    if not name:
        return jsonify({"success": False, "error": "Имя не указано"}), 400
    ok, msg = projects.create_category(name)
    return jsonify({"success": ok, "error": msg if not ok else None})


@bp.route('/projects/category/<name>', methods=['DELETE'])
def api_delete_category(name):
    ok, msg = projects.delete_category(name)
    return jsonify({"success": ok, "error": msg if not ok else None})


@bp.route('/projects/category/<name>/rename', methods=['POST'])
def api_rename_category(name):
    new_name = request.json.get('name', '').strip()
    if not new_name:
        return jsonify({"success": False, "error": "Новое имя не указано"}), 400
    ok, msg = projects.rename_category(name, new_name)
    return jsonify({"success": ok, "error": msg if not ok else None})


@bp.route('/projects/create', methods=['POST'])
def api_create_project():
    data = request.json
    cat = data.get('category', '').strip()
    name = data.get('name', '').strip()
    desc = data.get('description', '')
    if not cat or not name:
        return jsonify({"success": False, "error": "Категория и имя обязательны"}), 400
    ok, msg = projects.create_project(cat, name, desc)
    return jsonify({"success": ok, "error": msg if not ok else None})


@bp.route('/projects/<category>/<name>', methods=['DELETE'])
def api_delete_project(category, name):
    if projects.is_platformio(name):
        return jsonify({"success": False, "error": "Проект PlatformIO нельзя удалить"}), 400
    ok, msg = projects.delete_project(category, name)
    return jsonify({"success": ok, "error": msg if not ok else None})


@bp.route('/projects/<category>/<name>/rename', methods=['POST'])
def api_rename_project(category, name):
    if projects.is_platformio(name):
        return jsonify({"success": False, "error": "Проект PlatformIO нельзя переименовать"}), 400
    new_name = request.json.get('name', '').strip()
    if not new_name:
        return jsonify({"success": False, "error": "Новое имя не указано"}), 400
    ok, msg = projects.rename_project(category, name, new_name)
    return jsonify({"success": ok, "error": msg if not ok else None})


@bp.route('/projects/copy', methods=['POST'])
def api_copy_project():
    data = request.json
    src_cat = data.get('src_cat', '')
    src_name = data.get('src_name', '')
    dst_cat = data.get('dst_cat', '')
    dst_name = data.get('dst_name', '')
    if projects.is_platformio(src_name):
        # Сохраняем исходное имя устройства проекта PlatformIO,
        # чтобы не изменять его в копии.
        src_dev_name = None
        if os.path.exists(ROOT_CONFIG_FILE):
            with open(ROOT_CONFIG_FILE, 'r', encoding='utf-8') as f:
                src_dev_name = json.load(f).get("iotmSettings", {}).get("name")
        # Копирование проекта PlatformIO:
        # создаём обычный проект из корневого myProfile.json как шаблона
        ok, msg = projects.create_project(dst_cat, dst_name, "")
        if ok:
            # Восстанавливаем исходное имя устройства (create_project перезаписывает его именем проекта)
            if src_dev_name:
                cfg = projects.load_project_config(dst_cat, dst_name)
                if cfg is not None:
                    cfg.setdefault("iotmSettings", {})["name"] = src_dev_name
                    projects.save_project_config(dst_cat, dst_name, cfg)
            # Копируем и platformio.ini из корня в новый проект
            dst_dir = os.path.join(projects.PROJECTS_DIR, dst_cat, dst_name)
            if os.path.exists(PLATFORMIO_INI_FILE):
                shutil.copy(PLATFORMIO_INI_FILE, os.path.join(dst_dir, 'platformio.ini'))
    else:
        ok, msg = projects.copy_project(src_cat, src_name, dst_cat, dst_name)
    return jsonify({"success": ok, "error": msg if not ok else None})


@bp.route('/projects/move', methods=['POST'])
def api_move_project():
    data = request.json
    if projects.is_platformio(data.get('src_name', '')):
        return jsonify({"success": False, "error": "Проект PlatformIO нельзя перенести"}), 400
    ok, msg = projects.move_project(
        data.get('src_cat', ''), data.get('src_name', ''),
        data.get('dst_cat', ''), data.get('dst_name', '')
    )
    return jsonify({"success": ok, "error": msg if not ok else None})


@bp.route('/projects/<category>/<name>/open', methods=['POST'])
def api_open_project(category, name):
    config = projects.load_project_config(category, name)
    if config is None:
        return jsonify({"success": False, "error": "Проект не найден"}), 404
    globals_.current_project = {"category": category, "name": name}
    globals_.current_config = config
    projects.save_history(category, name)
    about = projects.load_project_about(category, name)
    # Определяем платформу из конфига
    de = config.get("projectProp", {}).get("platformio", {}).get("default_envs", "")
    if de:
        globals_.current_platform = de
    logger.info(f"Открыт проект: {category}/{name}")
    return jsonify({"success": True, "config": config, "about": about, "platform": globals_.current_platform})


@bp.route('/platformio/open', methods=['POST'])
def api_open_platformio():
    """Открытие проекта PlatformIO.

    Данные берутся напрямую из корневого myProfile.json,
    список платформ — из platformio.ini.
    """
    if not os.path.exists(ROOT_CONFIG_FILE):
        return jsonify({"success": False, "error": "myProfile.json не найден"}), 404
    with open(ROOT_CONFIG_FILE, 'r', encoding='utf-8') as f:
        config = json.load(f)
    globals_.current_project = {"category": projects.PLATFORMIO_PROJECT, "name": projects.PLATFORMIO_PROJECT}
    globals_.current_config = config
    # Платформа по умолчанию из конфига
    de = config.get("projectProp", {}).get("platformio", {}).get("default_envs", "")
    if de:
        globals_.current_platform = de
    logger.info(f"Открыт проект: {projects.PLATFORMIO_PROJECT}")
    return jsonify({
        "success": True,
        "config": config,
        "about": "",
        "platform": globals_.current_platform,
        "protected": True,
    })


@bp.route('/projects/last', methods=['GET'])
def api_last_project():
    hist = projects.load_history()
    if not hist:
        return jsonify({"success": True, "project": None})
    # Проверяем, существует ли ещё
    path = os.path.join(projects.PROJECTS_DIR, hist.get("category", ""), hist.get("name", ""))
    if not os.path.exists(os.path.join(path, projects.CONFIG_FILENAME)):
        return jsonify({"success": True, "project": None})
    return jsonify({"success": True, "project": hist})


@bp.route('/projects/list-all', methods=['GET'])
def api_list_all():
    """Список всех проектов для копирования настроек"""
    tree = projects.list_projects()
    flat = []
    for cat, projs in tree.items():
        for p in projs:
            flat.append({"category": cat, "name": p})
    return jsonify({"success": True, "projects": flat})


@bp.route('/projects/<category>/<name>/settings', methods=['GET'])
def api_get_project_settings(category, name):
    """Получение сырых iotmSettings проекта (для копирования отдельных групп)"""
    config = projects.load_project_config(category, name)
    if config is None:
        return jsonify({"success": False, "error": "Проект не найден"}), 404
    return jsonify({"success": True, "settings": config.get("iotmSettings", {})})


@bp.route('/projects/copy-settings', methods=['POST'])
def api_copy_settings():
    """Копирование iotmSettings из другого проекта"""
    if not globals_.current_project:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400
    data = request.json
    src_cat = data.get('src_cat', '')
    src_name = data.get('src_name', '')
    src_config = projects.load_project_config(src_cat, src_name)
    if not src_config:
        return jsonify({"success": False, "error": "Исходный проект не найден"}), 404
    # Копируем iotmSettings, но НЕ трогаем name и apssid текущего устройства
    src_settings = dict(src_config.get("iotmSettings", {}))
    src_settings.pop("name", None)
    src_settings.pop("apssid", None)
    with globals_._lock:
        globals_.current_config.setdefault("iotmSettings", {}).update(src_settings)
        projects.save_project_config(globals_.current_project["category"], globals_.current_project["name"], globals_.current_config)
    return jsonify({"success": True, "settings": globals_.current_config["iotmSettings"]})


@bp.route('/projects/copy-modules', methods=['POST'])
def api_copy_modules():
    """Копирование modules из другого проекта"""
    if not globals_.current_project:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400
    data = request.json
    src_cat = data.get('src_cat', '')
    src_name = data.get('src_name', '')
    src_config = projects.load_project_config(src_cat, src_name)
    if not src_config:
        return jsonify({"success": False, "error": "Исходный проект не найден"}), 404
    with globals_._lock:
        globals_.current_config["modules"] = src_config.get("modules", {})
        projects.save_project_config(globals_.current_project["category"], globals_.current_project["name"], globals_.current_config)
    return jsonify({"success": True, "modules": globals_.current_config["modules"]})


@bp.route('/config/import-root', methods=['POST'])
def api_import_root():
    """Импорт myProfile.json из корня проекта как шаблона для нового проекта"""
    if not globals_.current_project:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400
    if not os.path.exists(ROOT_CONFIG_FILE):
        return jsonify({"success": False, "error": "myProfile.json не найден"}), 404
    with open(ROOT_CONFIG_FILE, 'r', encoding='utf-8') as f:
        base = json.load(f)
    # Сохраняем имя устройства из текущего проекта
    dev_name = globals_.current_config.get("iotmSettings", {}).get("name", globals_.current_project["name"])
    base["iotmSettings"]["name"] = dev_name
    with globals_._lock:
        globals_.current_config = base
        projects.save_project_config(globals_.current_project["category"], globals_.current_project["name"], base)
    logger.info(f"Импортирован базовый конфиг в {globals_.current_project['category']}/{globals_.current_project['name']}")
    return jsonify({"success": True, "config": base})


# ==================== Починка platformio.ini ====================

@bp.route('/projects/repair-data-dir', methods=['POST'])
def api_repair_data_dir():
    """Перезаписывает [platformio] data_dir проекта на <проект>/data_svelte.

    Нужно, когда путь к каталогу данных ФС устарел (проект перенесли в другую
    категорию, категорию/проект переименовали) — тогда `pio run -t uploadfs`
    падает на сборке образа littlefs с кодом 1. Меняется только строка data_dir:
    остальной platformio.ini (комментарии, секции env) не трогается.

    body: {category: 'Платы', name: 'esp32s2mini'}
    """
    data = request.json or {}
    category = (data.get('category') or '').strip()
    name = (data.get('name') or '').strip()
    if not category or not name:
        return jsonify({"success": False, "error": "Не указан проект"}), 400

    if projects.is_platformio(name):
        proj_dir = PROJECT_ROOT
    else:
        proj_dir = os.path.join(projects.PROJECTS_DIR, category, name)
    if not os.path.isdir(proj_dir):
        return jsonify({"success": False, "error": f"Каталог проекта не найден: {proj_dir}"}), 404

    ok, value = projects.fix_data_dir(proj_dir)
    if not ok:
        logger.warning(f"Не удалось исправить data_dir для {category}/{name}: {value}")
        return jsonify({"success": False, "error": value}), 400
    logger.info(f"Исправлен data_dir для {category}/{name}: {value}")
    return jsonify({"success": True, "data_dir": value})


# ==================== Файловая система FS проекта ====================

def _project_fs_dir(category, name):
    """Каталог FS проекта (data_svelte) или None, если его нет."""
    if projects.is_platformio(name):
        proj_dir = PROJECT_ROOT
    else:
        proj_dir = os.path.join(projects.PROJECTS_DIR, category, name)
    if not os.path.isdir(proj_dir):
        return None
    return _find_data_svelte(proj_dir)


@bp.route('/projects/<category>/<name>/tree/fs', methods=['GET'])
def api_project_fs_tree(category, name):
    """Дерево файлов FS проекта (data_svelte)."""
    root = _project_fs_dir(category, name)
    if not root:
        return jsonify({"success": False, "error": "Каталог data_svelte не найден"}), 404
    dirs, files = _walk_tree(root)
    # Бинарные файлы (*.gz, favicon.ico) отдаём — они видны в дереве,
    # но их содержимое в редактор не открывается (см. file/fs)
    return jsonify({"success": True, "root": root, "dirs": dirs, "files": files})


@bp.route('/projects/<category>/<name>/file/fs', methods=['GET'])
def api_project_fs_read_file(category, name):
    """Содержимое файла из data_svelte проекта."""
    root = _project_fs_dir(category, name)
    if not root:
        return jsonify({"success": False, "error": "Каталог data_svelte не найден"}), 404
    rel = request.args.get("path", "")
    abs_path = _safe_path(root, rel)
    if not abs_path or not os.path.isfile(abs_path):
        return jsonify({"success": False, "error": "File not found"}), 404
    # Бинарные файлы (*.gz, favicon.ico): содержимое в редактор не отдаём,
    # метаданные (имя/размер) доступны по ?meta=1
    lower = abs_path.lower()
    is_binary = lower.endswith('.gz') or os.path.basename(lower) == 'favicon.ico'
    if request.args.get("meta") == "1":
        return jsonify({
            "success": True,
            "path": rel,
            "name": os.path.basename(rel),
            "size": os.path.getsize(abs_path),
        })
    if is_binary:
        return jsonify({"success": False, "error": "Бинарный файл недоступен в редакторе"}), 400
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


@bp.route('/projects/<category>/<name>/file/fs', methods=['POST'])
def api_project_fs_save_file(category, name):
    """Сохраняет изменённый файл локально в data_svelte проекта."""
    root = _project_fs_dir(category, name)
    if not root:
        return jsonify({"success": False, "error": "Каталог data_svelte не найден"}), 404
    data = request.json or {}
    rel = data.get("path", "")
    content = data.get("content", "")
    abs_path = _safe_path(root, rel)
    if not abs_path:
        return jsonify({"success": False, "error": "Invalid path"}), 400
    # Бинарные файлы (*.gz, favicon.ico) через редактор не записываем
    lower = abs_path.lower()
    if lower.endswith('.gz') or os.path.basename(lower) == 'favicon.ico':
        return jsonify({"success": False, "error": "Бинарный файл недоступен в редакторе"}), 400
    try:
        os.makedirs(os.path.dirname(abs_path), exist_ok=True)
        with open(abs_path, "w", encoding="utf-8") as f:
            f.write(content)
    except Exception as e:
        return jsonify({"success": False, "error": str(e)}), 500
    return jsonify({"success": True, "path": rel})
# ==================== Получение файлов из устройства ====================

def _iter_device_folders():
    """Все папки устройств (как /devices/list-all), но с путями RAM/FS.

    Возвращает ключ, имя, IP и каталоги разделов каждого устройства.
    """
    from core.devices import DEVICE_DIR_ROOT, _load_folder_meta
    if not os.path.isdir(DEVICE_DIR_ROOT):
        return
    for folder_name in sorted(os.listdir(DEVICE_DIR_ROOT)):
        folder = os.path.join(DEVICE_DIR_ROOT, folder_name)
        if not os.path.isdir(folder):
            continue
        meta = _load_folder_meta(folder) or {}
        yield {
            "key": folder_name,
            "name": meta.get("name") or folder_name,
            "ip": meta.get("ip") or "",
            "ram_dir": os.path.join(folder, "RAM"),
            "fs_dir": os.path.join(folder, "FS"),
        }


def _dir_has_files(path):
    """Есть ли в каталоге (рекурсивно) хоть один файл."""
    if not path or not os.path.isdir(path):
        return False
    for _root, _dirs, files in os.walk(path):
        if files:
            return True
    return False


@bp.route('/projects/<category>/<name>/file/sources', methods=['GET'])
def api_project_fs_fetch_sources(category, name):
    """Доступные источники получения файлов из устройства для проекта.

    mode=file — доступность конкретного файла (path): сохранённая копия RAM
    (по имени файла) / FS (по тому же пути); mode=all — наличие хоть каких-то
    сохранённых файлов RAM/FS. ram_live/fs_live — устройство сейчас в сети
    (статус зелёный) и раздел можно тянуть напрямую: RAM — только если файл
    входит в набор RAM_FILES (прошивка отдаёт по WS лишь его; например
    flashProfile.json в RAM отсутствует и пункта «с устройства» не получает),
    FS — любой файл по HTTP. Пункты с недоступными источниками фронтенд в
    модалке не показывает: нет сохранённых файлов — «сохранённые» пункты не
    показываются, нет устройств в сети — «с устройства» пункты не показываются.
    """
    root = _project_fs_dir(category, name)
    if not root:
        return jsonify({"success": False, "error": "Каталог data_svelte не найден"}), 404
    mode = request.args.get("mode", "file")
    if mode not in ("file", "all"):
        return jsonify({"success": False, "error": "Unknown mode"}), 400
    rel = request.args.get("path", "").strip()
    if mode == "file" and not rel:
        return jsonify({"success": False, "error": "Не выбран файл"}), 400
    # RAM хранится плоско — совпадение по имени файла; FS — по тому же пути
    ram_name = os.path.basename(rel) if mode == "file" else ""
    fs_rel = rel if mode == "file" else ""
    # «с устройства» доступно только для устройств, которые сейчас в сети
    online_ips = _online_device_ips()
    # RAM по WS отдаёт только фиксированный набор файлов (RAM_FILES);
    # для остальных имён (например flashProfile.json) пункт «RAM — с устройства»
    # не предлагается — устройству нечего отдать
    from utils.ws_client import RAM_FILES
    ram_fetchable = frozenset(RAM_FILES.values())
    sources = []
    for dev in _iter_device_folders():
        if mode == "all":
            ram_saved = _dir_has_files(dev["ram_dir"])
            fs_saved = _dir_has_files(dev["fs_dir"])
        else:
            ram_saved = os.path.isfile(os.path.join(dev["ram_dir"], ram_name))
            src = _safe_path(dev["fs_dir"], fs_rel)
            fs_saved = bool(src) and os.path.isfile(src)
        live = bool(dev["ip"]) and dev["ip"] in online_ips
        sources.append({
            "key": dev["key"], "name": dev["name"], "ip": dev["ip"],
            "ram_saved": ram_saved, "fs_saved": fs_saved,
            "ram_live": live and (mode == "all" or ram_name in ram_fetchable),
            "fs_live": live,
        })
    return jsonify({"success": True, "sources": sources})


def _online_device_ips():
    """Множество IP устройств, которые сейчас в сети (статус зелёный).

    Только устройства с папкой и FSM-статусом STATE_GREEN считаются онлайн.
    Orphan-устройства (без папки, только IP в _devices) по таймауту multicast
    не дают пунктов «с устройства».
    """
    from core.devices import (
        STATE_GREEN, _device_folders, _device_folders_lock,
        _device_states, _device_states_lock,
    )
    with _device_folders_lock:
        folder_ips = {e.get("ip") for e in _device_folders.values()
                      if e.get("ip")}
    with _device_states_lock:
        green_keys = [k for k, st in _device_states.items()
                      if st.get("state") == STATE_GREEN]
    with _device_folders_lock:
        green_ips = {
            _device_folders[k].get("ip")
            for k in green_keys
            if k in _device_folders and _device_folders[k].get("ip")
        }
    return green_ips & folder_ips


@bp.route('/projects/<category>/<name>/file/from-device', methods=['POST'])
def api_project_fs_fetch_from_device(category, name):
    """Получает файл/файлы из устройства в data_svelte проекта.

    body: {device_key, section: 'ram'|'fs', live: bool, all: bool, path: <rel>}
    live=False — из сохранённой копии в папке устройства (RAM/FS);
    live=True — напрямую с устройства (RAM по WS, FS по HTTP);
    all=False — один файл (path), all=True — весь раздел.
    """
    root = _project_fs_dir(category, name)
    if not root:
        return jsonify({"success": False, "error": "Каталог data_svelte не найден"}), 404
    data = request.json or {}
    device_key = str(data.get("device_key", "")).strip()
    section = str(data.get("section", "")).lower()
    live = bool(data.get("live"))
    fetch_all = bool(data.get("all"))
    rel = str(data.get("path", "")).strip()
    if section not in ("ram", "fs"):
        return jsonify({"success": False, "error": "Unknown section"}), 400
    dev = None
    for d in _iter_device_folders():
        if d["key"] == device_key:
            dev = d
            break
    if not dev:
        return jsonify({"success": False, "error": "Папка устройства не найдена"}), 404
    from core.devices import ws_client
    saved = []
    try:
        if section == "ram":
            saved = _fetch_ram(dev, root, live, fetch_all, rel)
        else:  # fs
            saved = _fetch_fs(dev, root, live, fetch_all, rel)
    except Exception as e:  # noqa: BLE001 — сеть/устройство, отдаём текст ошибки
        logger.error(f"fetch from device {section} {dev['ip'] or '?'}: {e}")
        return jsonify({"success": False, "error": str(e)}), 502
    return jsonify({"success": True, "section": section, "live": live,
                    "all": fetch_all, "saved": saved})


def _fetch_ram(dev, root, live, fetch_all, rel):
    """Копирование/скачивание файлов раздела RAM в data_svelte проекта."""
    from core.devices import ws_client
    if fetch_all:
        if live:
            return ws_client.fetch_ram(dev["ip"], root)
        saved = []
        if os.path.isdir(dev["ram_dir"]):
            for fname in sorted(os.listdir(dev["ram_dir"])):
                src = os.path.join(dev["ram_dir"], fname)
                if os.path.isfile(src):
                    shutil.copy2(src, os.path.join(root, fname))
                    saved.append(fname)
        return saved
    # один файл: в RAM хранится плоско, совпадение по имени
    src_name = os.path.basename(rel)
    if src_name not in ws_client.RAM_FILES.values():
        raise ValueError(f"Файл {src_name} не относится к RAM устройства")
    if live:
        ws_client.fetch_ram_file(dev["ip"], src_name, root)
        # устройство отдаёт файл под своим именем; если в проекте путь другой — переносим
        if src_name != rel:
            abs_src = _safe_path(root, src_name)
            abs_dst = _safe_path(root, rel)
            if abs_src and abs_dst:
                os.makedirs(os.path.dirname(abs_dst), exist_ok=True)
                os.replace(abs_src, abs_dst)
    else:
        src = os.path.join(dev["ram_dir"], src_name)
        if not os.path.isfile(src):
            raise FileNotFoundError(f"Сохранённый файл не найден: {src_name}")
        abs_dst = _safe_path(root, rel)
        if not abs_dst:
            raise ValueError("Invalid path")
        os.makedirs(os.path.dirname(abs_dst), exist_ok=True)
        shutil.copy2(src, abs_dst)
    return [rel]


def _fetch_fs(dev, root, live, fetch_all, rel):
    """Копирование/скачивание файлов раздела FS в data_svelte проекта."""
    from core.devices import ws_client
    if fetch_all:
        if live:
            return ws_client.fetch_fs(dev["ip"], root)
        if not _dir_has_files(dev["fs_dir"]):
            raise FileNotFoundError("Сохранённые файлы FS не найдены")
        shutil.copytree(dev["fs_dir"], root, dirs_exist_ok=True)
        saved = []
        for r, _dirs, files in os.walk(dev["fs_dir"]):
            for f in files:
                full = os.path.join(r, f)
                saved.append(os.path.relpath(full, dev["fs_dir"]).replace("\\", "/"))
        return saved
    # один файл
    if not rel:
        raise ValueError("Не выбран файл")
    abs_dst = _safe_path(root, rel)
    if not abs_dst:
        raise ValueError("Invalid path")
    if live:
        ws_client.fetch_fs_file(dev["ip"], rel, root)
    else:
        src = _safe_path(dev["fs_dir"], rel)
        if not src or not os.path.isfile(src):
            raise FileNotFoundError(f"Сохранённый файл не найден: {rel}")
        os.makedirs(os.path.dirname(abs_dst), exist_ok=True)
        shutil.copy2(src, abs_dst)
    return [rel]


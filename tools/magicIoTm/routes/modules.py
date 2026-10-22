"""
Модули — маршруты Flask для управления модулями.
"""

import logging

import state.globals as globals_
from flask import Blueprint, request, jsonify
from utils import projects
from core.config import scan_modinfo, load_platforms, get_compat_map, calc_size, get_module_flash, get_module_ram, is_compatible, get_fs_usage

logger = logging.getLogger(__name__)

bp = Blueprint('modules', __name__)


@bp.route('/modules/toggle', methods=['POST'])
def api_toggle_module():
    if not globals_.current_project:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400
    data = request.json
    section = data.get('section', '')
    path = data.get('path', '')
    active = data.get('active', False)
    with globals_._lock:
        if section in globals_.current_config.get("modules", {}):
            for m in globals_.current_config["modules"][section]:
                if m.get("path") == path:
                    m["active"] = active
                    break
        projects.save_project_config(globals_.current_project["category"], globals_.current_project["name"], globals_.current_config)
    fp, fu, ft, rp, ru, rt = calc_size()
    sp, su, st = get_fs_usage()
    return jsonify({"success": True, "flash_pct": fp, "flash_used": fu, "flash_total": ft,
                     "ram_pct": rp, "ram_used": ru, "ram_total": rt,
                     "fs_pct": sp, "fs_used": su, "fs_total": st})


@bp.route('/modules/sync', methods=['POST'])
def api_sync_modules():
    """Пакетное обновление активных модулей.

    body: {modules: {section: {path: active}}}
    """
    if not globals_.current_project:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400
    data = request.json
    sync_map = data.get('modules', {})
    with globals_._lock:
        for section, paths in sync_map.items():
            if section in globals_.current_config.get("modules", {}):
                for m in globals_.current_config["modules"][section]:
                    if m.get("path") in paths:
                        m["active"] = bool(paths[m.get("path")])
        projects.save_project_config(globals_.current_project["category"], globals_.current_project["name"], globals_.current_config)
    fp, fu, ft, rp, ru, rt = calc_size()
    sp, su, st = get_fs_usage()
    return jsonify({"success": True, "flash_pct": fp, "flash_used": fu, "flash_total": ft,
                     "ram_pct": rp, "ram_used": ru, "ram_total": rt,
                     "fs_pct": sp, "fs_used": su, "fs_total": st})


@bp.route('/modules/compatibility', methods=['GET'])
def api_compat():
    return jsonify({"success": True, "compatibility": get_compat_map(), "platform": globals_.current_platform})


@bp.route('/modules/reload', methods=['POST'])
def api_modules_reload():
    """Перезагрузка кэшей modinfo и platforms с диска (после замера размеров).

    Платформы перечитываем здесь, а не только в SSE-потоке замера: клиент
    закрывает поток сразу после события done, и генератор может не дойти
    до перезагрузки кэшей. Вызывается клиентом до updateGauge().
    """
    scan_modinfo()
    load_platforms()
    return jsonify({"success": True})


@bp.route('/modules/info', methods=['POST'])
def api_module_info():
    path = request.json.get('path', '')
    if not path:
        return jsonify({"success": False, "error": "Путь не указан"}), 400
    name = path.split("/")[-1]
    info = globals_.modinfo_cache.get(name, {})
    if not info:
        return jsonify({"success": False, "error": "Информация не найдена"}), 404
    return jsonify({"success": True, "info": {"about": info.get("about", {}), "usedLibs": info.get("usedLibs", {}),
                                               "usedFLASH": info.get("usedFLASH", {}), "usedRAM": info.get("usedRAM", {})}})

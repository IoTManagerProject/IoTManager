import logging

import state.globals as globals_
from flask import Blueprint, request, jsonify
from core.config import (get_platformio_platforms, scan_modinfo, load_platforms,
                         calc_size, get_fs_usage, is_compatible,
                         read_firmware_version)
from utils import projects

logger = logging.getLogger(__name__)

bp = Blueprint('platforms', __name__)


@bp.route('/platforms', methods=['GET'])
def api_platforms():
    platforms = []
    for p in get_platformio_platforms():
        pl = globals_.platforms_cache.get(p, {})
        platforms.append({
            "name": p,
            "baseline_flash": pl.get("baseline_flash", 0),
            # Версия прошивки последнего замера (FIRMWARE_VERSION из platforms.json)
            "baseline_version": pl.get("FIRMWARE_VERSION", ""),
        })
    return jsonify({
        "success": True,
        "platforms": platforms,
        # Текущая версия прошивки из include/Const.h
        "firmware_version": read_firmware_version(),
    })


@bp.route('/platform/change', methods=['POST'])
def api_change_platform():
    if not globals_.current_project:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400
    new_plat = request.json.get('platform', '')
    names = get_platformio_platforms()
    if new_plat not in names:
        return jsonify({"success": False, "error": "Платформа не найдена"}), 400
    # Отключаем несовместимые
    disabled = 0
    with globals_._lock:
        for section, mods in globals_.current_config.get("modules", {}).items():
            if not isinstance(mods, list):
                continue
            for m in mods:
                name = m.get("path", "").split("/")[-1]
                libs = globals_.modinfo_cache.get(name, {}).get("usedLibs", {})
                if m.get("active") and not is_compatible(new_plat, libs):
                    m["active"] = False
                    disabled += 1
        globals_.current_platform = new_plat
        globals_.current_config.setdefault("projectProp", {}).setdefault("platformio", {})["default_envs"] = new_plat
        projects.save_project_config(globals_.current_project["category"], globals_.current_project["name"], globals_.current_config)
    fp, fu, ft, rp, ru, rt = calc_size()
    sp, su, st = get_fs_usage()
    logger.info(f"Платформа: {new_plat}, отключено: {disabled}")
    return jsonify({"success": True, "platform": new_plat, "flash_pct": fp, "flash_used": fu, "flash_total": ft,
                     "ram_pct": rp, "ram_used": ru, "ram_total": rt,
                     "fs_pct": sp, "fs_used": su, "fs_total": st, "disabled_count": disabled})


@bp.route('/size', methods=['GET'])
def api_size():
    fp, fu, ft, rp, ru, rt = calc_size()
    sp, su, st = get_fs_usage()
    return jsonify({"flash_pct": fp, "flash_used": fu, "flash_total": ft,
                     "ram_pct": rp, "ram_used": ru, "ram_total": rt,
                     "fs_pct": sp, "fs_used": su, "fs_total": st,
                     "platform": globals_.current_platform})

"""
Загрузка (USB-прошивка) — маршруты Flask для прошивки ESP-устройств.
"""

import json
import os
import logging
import time

import state.globals as globals_
from flask import Blueprint, request, jsonify, Response

from core.flasher import (
    MODES,
    start as flash_start,
    is_running as flash_is_running,
    ensure_installed,
    list_esp_ports,
    family_of_env,
    expected_flash_from_env,
    detect_device,
    list_raw_ports,
)
from core.builder import resolve_build_config, is_running as build_is_running, data_dir_report, data_dir_error_text
from core.config import PROJECT_ROOT, ROOT_CONFIG_FILE, PLATFORMIO_INI_FILE, BASE_DIR

logger = logging.getLogger(__name__)

bp = Blueprint('upload', __name__)


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


@bp.route('/upload/status', methods=['GET'])
def api_upload_status():
    """Готовность к загрузке: собрана ли прошивка и ожидаемое семейство чипа."""
    cfg = _resolve_upload_config()
    if cfg is None:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400
    env = cfg.get("env", "")
    return jsonify({
        "success": True,
        "has_firmware": _has_built_firmware(cfg),
        "env": env,
        "expected_family": family_of_env(env),
    })


@bp.route('/upload/detect', methods=['POST'])
def api_upload_detect():
    """Определяет подключённый ESP-чип и проверяет соответствие платформе проекта."""
    cfg = _resolve_upload_config()
    if cfg is None:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400

    ensure_installed()

    env = cfg.get("env", "")
    expected = family_of_env(env)
    expected_flash = expected_flash_from_env(env)
    ports = list_esp_ports()
    matched = [p for p in ports if p.get("family") == expected]
    raw_ports = list_raw_ports()

    return jsonify({
        "success": True,
        "ports": ports,
        "raw_ports": raw_ports,
        "matched": matched,
        "env": env,
        "expected_family": expected,
        "expected_flash": expected_flash,
        "has_firmware": _has_built_firmware(cfg),
    })


@bp.route('/upload/start', methods=['POST'])
def api_upload_start():
    """Запуск прошивки в фоне. body: {mode: 'fs'|'firmware'|'full', port: 'COMx'}."""
    data = request.json or {}
    mode = data.get('mode', 'firmware')
    port = (data.get('port') or '').strip()

    if mode not in MODES:
        return jsonify({"success": False, "error": f"Неизвестный режим прошивки: {mode}"}), 400
    if not port:
        return jsonify({"success": False, "error": "Не указан COM-порт"}), 400
    if build_is_running():
        return jsonify({"success": False, "error": "Сборка выполняется — дождитесь её завершения"}), 409
    if flash_is_running():
        return jsonify({"success": False, "error": "Прошивка уже выполняется"}), 409

    cfg = _resolve_upload_config()
    if cfg is None:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400
    if not _has_built_firmware(cfg):
        return jsonify({"success": False, "error": "Прошивка не собрана. Сначала выполните сборку (??)."}), 400

    # Предпроверка каталога данных ФС: pio -t uploadfs собирает образ littlefs
    # из [platformio] data_dir (mklittlefs -c $PROJECT_DATA_DIR) и при устаревшем
    # пути падает с неинформативным «can't read source directory … Error 1».
    # UI по code='data_dir' предлагает исправить platformio.ini и повторить.
    if mode in ("fs", "full"):
        report = data_dir_report(cfg)
        if not report["ok"]:
            proj = globals_.current_project or {}
            reason = data_dir_error_text(report)
            logger.warning(f"Прошивка остановлена: {reason}")
            return jsonify({
                "success": False,
                "code": "data_dir",
                "error": reason,
                "ini_value": report["ini_value"],
                "resolved": report["resolved"],
                "expected": report["expected"],
                "project": {"category": proj.get("category", ""), "name": proj.get("name", "")},
            }), 409

    ensure_installed()

    expected_flash = expected_flash_from_env(cfg.get("env", ""))
    dev_info = detect_device(port)
    if (dev_info and dev_info.get("flash_bytes") is not None
            and expected_flash and dev_info["flash_bytes"] < expected_flash):
        return jsonify({
            "success": False,
            "error": ("Флеш-память чипа {} ({}) меньше требуемой для платформы ({} МБ). "
                      "Прошивка отменена.").format(
                dev_info.get("model", "?"),
                dev_info.get("flash_label", "?"),
                expected_flash // 1048576,
            )
        }), 409

    cfg["mode"] = mode
    cfg["upload_port"] = port
    flash_start(cfg)
    logger.info(f"Прошивка запущена: env={cfg['env']}, mode={mode}, port={port}")
    return jsonify({"success": True})


@bp.route('/upload/stream')
def api_upload_stream():
    """SSE-поток событий прошивки (лог, шаги, финал)."""
    def gen():
        from core.flasher import event_stream
        for chunk in event_stream():
            yield chunk
    return Response(gen(), mimetype="text/event-stream",
                    headers={"Cache-Control": "no-cache", "X-Accel-Buffering": "no"})

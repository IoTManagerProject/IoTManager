import logging

import state.globals as globals_
from flask import Blueprint, request, jsonify, Response

from core.builder import get_platformio_path, resolve_build_config, is_running, event_stream, get_status, start
from core.measurer import is_running as measure_is_running
from core.config import PROJECT_ROOT, ROOT_CONFIG_FILE, PLATFORMIO_INI_FILE, BASE_DIR, save_platform_fs_total, _project_dir
from utils import projects

logger = logging.getLogger(__name__)

bp = Blueprint('build', __name__)


@bp.route('/build/start', methods=['POST'])
def api_build_start():
    """Запуск сборки в фоне. Перед сборкой сохраняем текущий конфиг."""
    if not globals_.current_project:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400
    if is_running():
        return jsonify({"success": False, "error": "Сборка уже выполняется"}), 409
    if measure_is_running():
        return jsonify({"success": False, "error": "Замер размера уже выполняется — дождитесь его завершения"}), 409
    if globals_.current_config:
        try:
            with globals_._lock:
                projects.save_project_config(globals_.current_project["category"], globals_.current_project["name"], globals_.current_config)
        except Exception as e:
            logger.error(f"Не удалось сохранить конфиг перед сборкой: {e}")
    cfg = resolve_build_config(globals_.current_project, globals_.current_config or {})
    if cfg is None:
        return jsonify({"success": False, "error": "Не удалось определить пути для сборки"}), 400
    start(cfg)
    logger.info(f"Сборка запущена: {cfg['project_label']}, env={cfg['env']}")
    return jsonify({"success": True})


@bp.route('/build/stream')
def api_build_stream():
    """SSE-поток событий сборки (лог, шаги, финал)."""
    def gen():
        for chunk in event_stream():
            yield chunk
        try:
            st = get_status()
            if st.get("success") and st.get("sizes") and st["sizes"].get("fs_total"):
                save_platform_fs_total(globals_.current_platform, st["sizes"]["fs_total"])
        except Exception as e:
            logger.error(f"Не удалось сохранить total_fs после сборки: {e}")
    return Response(gen(), mimetype="text/event-stream",
                    headers={"Cache-Control": "no-cache", "X-Accel-Buffering": "no"})

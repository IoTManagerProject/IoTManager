import logging

import state.globals as globals_
from flask import Blueprint, request, jsonify, Response

from core.builder import get_platformio_path, is_running as build_is_running
from core.measurer import start, is_running as measure_is_running, event_stream, stop
from core.config import (PROJECT_ROOT, scan_modinfo, load_platforms, _project_dir,
                         BASE_DIR, MEASURE_SCRIPT, get_platformio_platforms)

logger = logging.getLogger(__name__)

bp = Blueprint('measure', __name__)


@bp.route('/measure/start', methods=['POST'])
def api_measure_start():
    """Запуск замера размера модулей (measure.py) в фоне."""
    if not globals_.current_project:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400
    if build_is_running():
        return jsonify({"success": False, "error": "Сборка уже выполняется — дождитесь её завершения"}), 409
    if measure_is_running():
        return jsonify({"success": False, "error": "Замер размера уже выполняется"}), 409

    data = request.json or {}
    scope = data.get('scope', 'all')
    module_path = (data.get('module') or '').strip()
    platform = (data.get('platform') or '').strip() or globals_.current_platform
    # Область замера базы: 'current' — только выбранная платформа, 'all' — все
    plat_scope = data.get('platforms_scope') or 'current'

    env_list = [platform]
    if scope == 'baseline' and plat_scope == 'all':
        env_list = get_platformio_platforms() or [platform]

    args = ['--no-color', '--pio', get_platformio_path(), '--baseline', 'prev']
    for env in env_list:
        args += ['--env', env]
    label = f"{globals_.current_project.get('category', '')}/{globals_.current_project.get('name', '')}"

    if scope == 'module':
        if not module_path:
            return jsonify({"success": False, "error": "Модуль не указан"}), 400
        args += ['--module', module_path]
        label = f"{label} · модуль {module_path.split('/')[-1]}"
    elif scope == 'baseline':
        args += ['--baseline', 'build', '--baseline-only']
        if len(env_list) > 1:
            label = f"{label} · базовая прошивка ({len(env_list)} платформ)"
        else:
            label = f"{label} · базовая прошивка"
    elif scope == 'profile':
        args += ['--mode', '2']
    elif scope == 'without':
        args += ['--mode', '3']
    else:
        args += ['--mode', '1']

    args += ['--project-dir', _project_dir(globals_.current_project)]

    abort_file = PROJECT_ROOT + '/.measure_abort'
    cfg = {
        "script": MEASURE_SCRIPT,
        "args": args + ['--abort-file', abort_file],
        "cwd": PROJECT_ROOT,
        "label": label,
        "abort_file": abort_file,
    }
    if not start(cfg):
        return jsonify({"success": False, "error": "Не удалось запустить замер"}), 409
    logger.info(f"Замер запущен: scope={scope}, platform={platform}, module={module_path}, "
                f"platforms_scope={plat_scope if scope == 'baseline' else '-'}")
    return jsonify({"success": True})


@bp.route('/measure/stream')
def api_measure_stream():
    """SSE-поток событий замера (лог, финал)."""
    def gen():
        for chunk in event_stream():
            yield chunk
        try:
            scan_modinfo()
            load_platforms()
            logger.info("Кэши modinfo/platforms обновлены после замера")
        except Exception as e:
            logger.error(f"Не удалось обновить кэши после замера: {e}")
    return Response(gen(), mimetype="text/event-stream",
                    headers={"Cache-Control": "no-cache", "X-Accel-Buffering": "no"})


@bp.route('/measure/abort', methods=['POST'])
def api_measure_abort():
    """Мягкое прерывание текущего замера (measure.py восстановит состояние проекта)."""
    if not measure_is_running():
        return jsonify({"success": False, "error": "Замер не выполняется"}), 409
    if stop():
        logger.info("Запрос на прерывание замера отправлен")
        return jsonify({"success": True})
    return jsonify({"success": False, "error": "Не удалось прервать замер"}), 409

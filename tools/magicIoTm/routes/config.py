"""
Config routes for MagicIoTm

Все маршруты, связанные с конфигурацией проекта.
"""

import json
import logging
import time
from datetime import datetime

import state.globals as globals_
from flask import Blueprint, request, jsonify
from utils import projects
from core.config import calc_size, get_fs_usage

logger = logging.getLogger(__name__)

bp = Blueprint('config', __name__)


@bp.route('/config', methods=['GET'])
def api_get_config():
    return jsonify(globals_.current_config or {})


@bp.route('/config/save', methods=['POST'])
def api_save_config():
    if not globals_.current_project:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400
    data = request.json
    if "modules" not in data:
        return jsonify({"success": False, "error": "Неверный формат"}), 400
    with globals_._lock:
        globals_.current_config = data
        projects.save_project_config(globals_.current_project["category"], globals_.current_project["name"], data)
    return jsonify({"success": True})


@bp.route('/config/settings', methods=['POST'])
def api_save_settings():
    if not globals_.current_project:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400
    settings = request.json
    with globals_._lock:
        if "iotmSettings" not in globals_.current_config:
            globals_.current_config["iotmSettings"] = {}
        globals_.current_config["iotmSettings"].update(settings)
        projects.save_project_config(globals_.current_project["category"], globals_.current_project["name"], globals_.current_config)
    return jsonify({"success": True})


@bp.route('/config/about', methods=['POST'])
def api_save_about():
    if not globals_.current_project:
        return jsonify({"success": False, "error": "Проект не открыт"}), 400
    text = request.json.get('text', '')
    projects.save_project_about(globals_.current_project["category"], globals_.current_project["name"], text)
    return jsonify({"success": True})


@bp.route('/config/export', methods=['GET'])
def api_export():
    if not globals_.current_config:
        return jsonify({"success": False, "error": "Нет конфигурации"}), 400
    ts = datetime.now().strftime("%Y%m%d_%H%M%S")
    fname = f"myProfile_{ts}.json"
    return jsonify({"success": True, "data": json.dumps(globals_.current_config, ensure_ascii=False, indent=2), "filename": fname})

"""
Валидация — маршруты Flask.
"""

import logging

from flask import Blueprint, request, jsonify
from utils import projects

logger = logging.getLogger(__name__)

bp = Blueprint('validation', __name__)


@bp.route('/validate/name', methods=['GET'])
def api_validate_name():
    val = request.args.get('value', '')
    cur = request.args.get('project', '')
    ok, msg = projects.validate_name(val, cur if cur else None)
    return jsonify({"valid": ok, "message": msg})


@bp.route('/validate/apssid', methods=['GET'])
def api_validate_apssid():
    val = request.args.get('value', '')
    cur = request.args.get('project', '')
    ok, msg = projects.validate_apssid(val, cur if cur else None)
    return jsonify({"valid": ok, "message": msg})

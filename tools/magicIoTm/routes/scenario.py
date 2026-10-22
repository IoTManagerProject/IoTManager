"""
Сценарии — маршруты Flask для работы с scenario.txt.
"""

import json
import os
import glob
import logging

from flask import Blueprint, jsonify
from core.config import MODULES_SRC_DIR

logger = logging.getLogger(__name__)

bp = Blueprint('scenario', __name__)


@bp.route('/scenario/functions', methods=['GET'])
def api_scenario_functions():
    """Списки функций для проверки scenario.txt.

    - reserved: зарезервированные имена функций/переменных из src/modules/sceninfo.json
    - bySubtype: соответствие subtype элемента -> список встроенных функций модуля
      (собрано из about.funcInfo[].name всех modinfo.json по configItem[].subtype)
    """
    reserved = []
    sceninfo_path = os.path.join(MODULES_SRC_DIR, 'sceninfo.json')
    if os.path.isfile(sceninfo_path):
        try:
            with open(sceninfo_path, 'r', encoding='utf-8') as f:
                si = json.load(f)
            for fi in (si.get('about', {}).get('funcInfo', []) or []):
                if fi.get('name'):
                    reserved.append(str(fi['name']))
        except Exception as e:  # noqa: BLE001
            logger.error(f"sceninfo.json: {e}")

    by_subtype = {}
    pattern = os.path.join(MODULES_SRC_DIR, '**', 'modinfo.json')
    for fp in glob.glob(pattern, recursive=True):
        try:
            with open(fp, 'r', encoding='utf-8') as f:
                info = json.load(f)
            about = info.get('about', {}) or {}
            module_name = about.get('moduleName') or os.path.basename(os.path.dirname(fp))
            funcs = [str(fi['name']) for fi in (about.get('funcInfo', []) or []) if fi.get('name')]
            for ci in (info.get('configItem', []) or []):
                sub = ci.get('subtype')
                if not sub:
                    continue
                bucket = by_subtype.setdefault(str(sub), set())
                bucket.update(funcs)
        except Exception as e:  # noqa: BLE001
            logger.error(f"modinfo {fp}: {e}")

    return jsonify({
        "success": True,
        "reserved": sorted(set(reserved)),
        "bySubtype": {k: sorted(v) for k, v in by_subtype.items()},
        "module": "Scenario",
    })

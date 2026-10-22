"""
Builder — логика сборки прошивок.

Извлечено из app.py для разделения ответственности.
"""

import os
import logging
import shutil
import subprocess
import sys

from utils import projects, build
from state.globals import current_project, current_config
from core.config import (
    PROJECT_ROOT, ROOT_CONFIG_FILE, PLATFORMIO_INI_FILE,
    PLATFORMS_FILE, MEASURE_SCRIPT, BASE_DIR,
    get_project_platformio_path, _project_dir,
)

logger = logging.getLogger(__name__)


def get_platformio_path():
    """Путь к исполняемому файлу PlatformIO (pio/platformio).

    Сначала ищем команду в PATH, затем резервно — в стандартном каталоге установки.
    """
    # 1. Поиск в PATH (pio или platformio)
    for name in ('pio', 'platformio'):
        found = shutil.which(name)
        if found:
            return found
    # 2. Резерв: стандартный каталог установки PlatformIO
    if os.name == 'nt':
        exe_names = ['platformio.exe', 'pio.exe']
        base = os.path.join(os.environ['USERPROFILE'], '.platformio', 'penv', 'Scripts')
    else:
        exe_names = ['pio', 'platformio']
        base = os.path.join(os.environ.get('HOME', ''), '.platformio', 'penv', 'bin')
    for name in exe_names:
        candidate = os.path.join(base, name)
        if os.path.isfile(candidate):
            return candidate
    # 3. Резерв: окружение самой панели (pio, установленный через
    #    /api/tools/platformio/install) — sys.executable = <venv>/Scripts|bin/python
    exe_names = ['pio.exe', 'platformio.exe'] if os.name == 'nt' else ['pio', 'platformio']
    venv_dir = os.path.dirname(sys.executable)
    for name in exe_names:
        candidate = os.path.join(venv_dir, name)
        if os.path.isfile(candidate):
            return candidate
    # Если ни один не найден — возвращаем дефолт, чтобы вызвать понятную ошибку запуска
    return os.path.join(base, exe_names[0])


def resolve_build_config(proj, config):
    """Формирует cfg для build.start() по текущему проекту."""
    if projects.is_platformio(proj.get("name", "")):
        profile = ROOT_CONFIG_FILE
        ini = PLATFORMIO_INI_FILE
    else:
        proj_dir = os.path.join(projects.PROJECTS_DIR, proj.get("category", ""), proj.get("name", ""))
        profile = os.path.join(proj_dir, projects.CONFIG_FILENAME)
        ini = os.path.join(proj_dir, "platformio.ini")
    if not os.path.isfile(profile):
        logger.error(f"Профиль не найден: {profile}")
        return None
    env = ""
    try:
        env = config.get("projectProp", {}).get("platformio", {}).get("default_envs", "")
    except AttributeError:
        env = ""
    # Скрипт подготовки профиля лежит в utils/ рядом с build.py
    prepare_script = os.path.join(os.path.dirname(os.path.abspath(build.__file__)), "PrepareProject.py")
    return {
        "profile": profile,
        "ini": ini,
        "env": env,
        "pio": get_platformio_path(),
        "prepare": prepare_script,
        "cwd": PROJECT_ROOT,
        # data_svelte всегда лежит в корне проекта (рядом с myProfile.json)
        "data_dir": os.path.join(os.path.dirname(profile), "data_svelte"),
        "project_label": f"{proj.get('category','')}/{proj.get('name','')}",
    }


def data_dir_report(cfg):
    """Отчёт о состоянии каталога данных ФС текущего проекта.

    Тонкая обёртка над projects.data_dir_report(): берёт ini/profile/cwd из cfg
    (см. resolve_build_config) и ожидаемый каталог — <проект>/data_svelte.

    Returns:
        dict: ok, reason, ini, ini_value, resolved, expected
    """
    ini = cfg.get("ini", "") or ""
    profile = cfg.get("profile", "") or ""
    expected = cfg.get("data_dir") or os.path.join(os.path.dirname(profile), projects.DATA_DIR_NAME)
    return projects.data_dir_report(ini, expected, cfg.get("cwd") or PROJECT_ROOT)


data_dir_error_text = projects.data_dir_error_text


# Re-export from utils.build
start = build.start
is_running = build.is_running
event_stream = build.event_stream
get_status = build.get_status
